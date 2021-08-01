#include "dial.hpp"

#include <components/debug/debuglog.hpp>

#include "common.hpp"
#include "reader.hpp"
#include "../esm/esmwriter.hpp"

namespace ESM3
{
    unsigned int Dialogue::sRecordId = REC_DIAL;

    void Dialogue::load(Reader& reader, bool& isDeleted)
    {
        loadId(reader);
        loadData(reader, isDeleted);
    }

    void Dialogue::loadId(Reader& reader)
    {
        reader.getSubRecordHeader();
        const ESM3::SubRecordHeader& subHdr = reader.subRecordHeader();
        if (subHdr.typeId != ESM3::SUB_NAME)
            reader.fail("Unexpected subrecord");

        reader.getZString(mId);
    }

    void Dialogue::loadData(Reader& reader, bool& isDeleted)
    {
        isDeleted = false;

        while (reader.getSubRecordHeader())
        {
            const ESM3::SubRecordHeader& subHdr = reader.subRecordHeader();
            switch (subHdr.typeId)
            {
                case ESM3::SUB_DATA:
                {
                    if (subHdr.dataSize == 1)
                    {
                        reader.get(mType);
                    }
                    else
                    {
                        reader.skipSubRecordData();
                    }
                    break;
                }
                case ESM3::SUB_DELE:
                {
                    reader.skipSubRecordData();
                    mType = Unknown;
                    isDeleted = true;
                    break;
                }
                default:
                    reader.fail("Unknown subrecord");
                    break;
            }
        }
    }

    void Dialogue::save(ESM::ESMWriter& esm, bool isDeleted) const
    {
        esm.writeHNCString("NAME", mId);
        if (isDeleted)
        {
            esm.writeHNString("DELE", "", 3);
        }
        else
        {
            esm.writeHNT("DATA", mType);
        }
    }

    void Dialogue::blank()
    {
        mInfo.clear();
    }

    void Dialogue::readInfo(Reader& reader, bool merge)
    {
        ESM3::DialInfo info;
        bool isDeleted = false;
        info.load(reader, isDeleted);

        if (!merge || mInfo.empty())
        {
            mLookup[info.mId] = std::make_pair(mInfo.insert(mInfo.end(), info), isDeleted);
            return;
        }

        InfoContainer::iterator it = mInfo.end();

        LookupMap::iterator lookup;
        lookup = mLookup.find(info.mId);

        if (lookup != mLookup.end())
        {
            it = lookup->second.first;
            // Since the new version of this record may have changed the next/prev linked list connection, we need to re-insert the record
            mInfo.erase(it);
            mLookup.erase(lookup);
        }

        if (info.mNext.empty())
        {
            mLookup[info.mId] = std::make_pair(mInfo.insert(mInfo.end(), info), isDeleted);
            return;
        }
        if (info.mPrev.empty())
        {
            mLookup[info.mId] = std::make_pair(mInfo.insert(mInfo.begin(), info), isDeleted);
            return;
        }

        lookup = mLookup.find(info.mPrev);
        if (lookup != mLookup.end())
        {
            it = lookup->second.first;

            mLookup[info.mId] = std::make_pair(mInfo.insert(++it, info), isDeleted);
            return;
        }

        lookup = mLookup.find(info.mNext);
        if (lookup != mLookup.end())
        {
            it = lookup->second.first;

            mLookup[info.mId] = std::make_pair(mInfo.insert(it, info), isDeleted);
            return;
        }

        Log(Debug::Warning) << "Warning: Failed to insert info " << info.mId;
    }

    void Dialogue::clearDeletedInfos()
    {
        LookupMap::const_iterator current = mLookup.begin();
        LookupMap::const_iterator end = mLookup.end();
        for (; current != end; ++current)
        {
            if (current->second.second)
            {
                mInfo.erase(current->second.first);
            }
        }
        mLookup.clear();
    }
}
