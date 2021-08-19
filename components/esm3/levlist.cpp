#include "levlist.hpp"

#include "reader.hpp"
#include "../esm/esmwriter.hpp"

namespace ESM3
{
    void LevelledListBase::load(Reader& reader, bool& isDeleted)
    {
        isDeleted = false;
        mRecordFlags = reader.getRecordFlags();

        std::uint32_t length = 0;
        bool hasName = false;
        bool hasList = false;
        while (reader.getSubRecordHeader())
        {
            const ESM3::SubRecordHeader& subHdr = reader.subRecordHeader();
            switch (subHdr.typeId)
            {
                case ESM3::SUB_NAME:
                {
                    reader.getZString(mId); // FIXME: fixed size for item only?
                    hasName = true;
                    break;
                }
                case ESM3::SUB_DATA: reader.get(mFlags); break;
                case ESM3::SUB_NNAM: reader.get(mChanceNone); break;
                case ESM3::SUB_INDX:
                {
                    reader.get(length);

                    mList.resize(length);

                    // If this levelled list was already loaded by a previous content file,
                    // we overwrite the list. Merging lists should probably be left to external tools,
                    // with the limited amount of information there is in the records, all merging methods
                    // will be flawed in some way. For a proper fix the ESM format would have to be changed
                    // to actually track list changes instead of including the whole list for every file
                    // that does something with that list.
                    for (size_t i = 0; i < mList.size(); i++)
                    {
                        LevelItem &li = mList[i];

                        reader.getSubRecordHeader(mRecName);
                        reader.getZString(li.mId); // FIXME: check if null terminated

                        reader.getSubRecordHeader(ESM3::SUB_INTV);
                        reader.get(li.mLevel);
                    }

                    hasList = true;
                    break;
                }
                case ESM3::SUB_INAM:
                case ESM3::SUB_CNAM:
                {
                    LevelItem li;
                    reader.getZString(li.mId);
                    mList.push_back(li);
                    break;
                }
                case ESM3::SUB_INTV:
                {
                    reader.get(mList.back().mLevel); // assumes this sub-record follows INAM/CNAM
                    break;
                }
                case ESM3::SUB_DELE:
                {
                    reader.skipSubRecordData();
                    isDeleted = true;
                    break;
                }
                default:
                {
                    if (!hasList)
                    {
                        // Original engine ignores rest of the record, even if there are items following
                        mList.clear();
                        reader.skipRecordData();
                    }
                    else
                    {
                        reader.fail("Unknown subrecord");
                    }
                    break;
                }
            }
        }

        if (!hasName)
            reader.fail("Missing NAME subrecord");

        if (length != mList.size())
            reader.fail("Item count incorrect");
    }

    void LevelledListBase::save(ESM::ESMWriter& esm, bool isDeleted) const
    {
        esm.writeHNCString("NAME", mId);

        if (isDeleted)
        {
            esm.writeHNString("DELE", "", 3);
            return;
        }

        esm.writeHNT("DATA", mFlags);
        esm.writeHNT("NNAM", mChanceNone);
        esm.writeHNT<int>("INDX", static_cast<unsigned int>(mList.size())); // FIXME: warning C4267

        for (std::vector<LevelItem>::const_iterator it = mList.begin(); it != mList.end(); ++it)
        {
            esm.writeHNCString(ESM::printName(mRecName), it->mId);
            esm.writeHNT("INTV", it->mLevel);
        }
    }

    void LevelledListBase::blank()
    {
        mFlags = 0;
        mChanceNone = 0;
        mList.clear();
    }

    unsigned int CreatureLevList::sRecordId = REC_LEVC;

    unsigned int ItemLevList::sRecordId = REC_LEVI;
}
