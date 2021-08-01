#include "misc.hpp"

//#ifdef NDEBUG
//#undef NDEBUG
//#endif

#include <cassert>

#include "common.hpp"
#include "reader.hpp"
#include "../esm/esmwriter.hpp"

namespace ESM3
{
    unsigned int Miscellaneous::sRecordId = REC_MISC;

    void Miscellaneous::load(Reader& reader, bool& isDeleted)
    {
        isDeleted = false;
        mRecordFlags = reader.getRecordFlags();

        bool hasName = false;
        bool hasData = false;
        while (reader.getSubRecordHeader())
        {
            const ESM3::SubRecordHeader& subHdr = reader.subRecordHeader();
            switch (subHdr.typeId)
            {
                case ESM3::SUB_NAME:
                {
                    reader.getZString(mId); // FIXME: fixed size?
                    hasName = true;
                    break;
                }
                case ESM3::SUB_MODL: reader.getZString(mModel); break;
                case ESM3::SUB_FNAM: reader.getZString(mName); break;
                case ESM3::SUB_SCRI: reader.getZString(mScript); break;
                case ESM3::SUB_ITEX: reader.getZString(mIcon); break;
                case ESM3::SUB_MCDT:
                {
                    assert (subHdr.dataSize == 12 && "MISC data size mismatch");
                    assert (subHdr.dataSize == sizeof(mData) && "MISC data size mismatch");
                    reader.get(mData);
                    hasData = true;
                    break;
                }
                case ESM3::SUB_DELE:
                {
                    reader.skipSubRecordData();
                    isDeleted = true;
                    break;
                }
                default:
                    reader.fail("Unknown subrecord");
                    break;
            }
        }

        if (!hasName)
            reader.fail("Missing NAME subrecord");

        if (!hasData && !isDeleted)
            reader.fail("Missing MCDT subrecord");
    }

    void Miscellaneous::save(ESM::ESMWriter& esm, bool isDeleted) const
    {
        esm.writeHNCString("NAME", mId);

        if (isDeleted)
        {
            esm.writeHNString("DELE", "", 3);
            return;
        }

        esm.writeHNCString("MODL", mModel);
        esm.writeHNOCString("FNAM", mName);
        esm.writeHNT("MCDT", mData, 12);
        esm.writeHNOCString("SCRI", mScript);
        esm.writeHNOCString("ITEX", mIcon);
    }

    void Miscellaneous::blank()
    {
        mData.mWeight = 0;
        mData.mValue = 0;
        mData.mIsKey = 0;
        mName.clear();
        mModel.clear();
        mIcon.clear();
        mScript.clear();
    }
}
