#include "body.hpp"

#include <cassert>

#include "common.hpp"
#include "reader.hpp"
#include "../esm/esmwriter.hpp"

namespace ESM3
{
    unsigned int BodyPart::sRecordId = REC_BODY;

    void BodyPart::load(Reader& reader, bool& isDeleted)
    {
        isDeleted = false;

        bool hasName = false;
        bool hasData = false;
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
                case ESM3::SUB_FNAM: reader.getZString(mRace);  break;
                case ESM3::SUB_BYDT:
                {
                    assert (sizeof(mData) == 4);
                    assert (subHdr.dataSize == sizeof(mData));
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
            reader.fail("Missing BYDT subrecord");
    }

    void BodyPart::save(ESM::ESMWriter& esm, bool isDeleted) const
    {
        esm.writeHNCString("NAME", mId);

        if (isDeleted)
        {
            esm.writeHNString("DELE", "", 3);
            return;
        }

        esm.writeHNCString("MODL", mModel);
        esm.writeHNOCString("FNAM", mRace);
        esm.writeHNT("BYDT", mData, 4);
    }

    void BodyPart::blank()
    {
        mData.mPart = 0;
        mData.mVampire = 0;
        mData.mFlags = 0;
        mData.mType = 0;

        mModel.clear();
        mRace.clear();
    }
}
