#include "stat.hpp"

#include "common.hpp"
#include "reader.hpp"
#include "../esm/esmwriter.hpp"

namespace ESM3
{
    unsigned int Static::sRecordId = REC_STAT;

    void Static::load(Reader& reader, bool& isDeleted)
    {
        isDeleted = false;
        mRecordFlags = reader.getRecordFlags();
        //bool isBlocked = (mRecordFlags & ESM::FLAG_Blocked) != 0;
        //bool isPersistent = (mRecordFlags & ESM::FLAG_Persistent) != 0;

        bool hasName = false;
        while (reader.getSubRecordHeader())
        {
            const ESM3::SubRecordHeader& subHdr = reader.subRecordHeader();
            switch (subHdr.typeId)
            {
                case ESM3::SUB_NAME:
                {
                    reader.getZString(mId);
                    hasName = true;
                    break;
                }
                case ESM3::SUB_MODL: reader.getZString(mModel); break;
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
    }

    void Static::save(ESM::ESMWriter& esm, bool isDeleted) const
    {
        esm.writeHNCString("NAME", mId);
        if (isDeleted)
        {
            esm.writeHNString("DELE", "", 3);
        }
        else
        {
            esm.writeHNCString("MODL", mModel);
        }
    }

    void Static::blank()
    {
        mModel.clear();
    }
}
