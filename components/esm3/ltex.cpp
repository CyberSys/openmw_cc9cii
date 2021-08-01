#include "ltex.hpp"

#include "common.hpp"
#include "reader.hpp"
#include "../esm/esmwriter.hpp"

namespace ESM3
{
    unsigned int LandTexture::sRecordId = REC_LTEX;

    void LandTexture::load(Reader& reader, bool& isDeleted)
    {
        isDeleted = false;

        bool hasName = false;
        bool hasIndex = false;
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
                case ESM3::SUB_DATA: reader.getZString(mTexture); break;
                case ESM3::SUB_INTV:
                    reader.get(mIndex);
                    hasIndex = true;
                    break;
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

        if (!hasIndex)
            reader.fail("Missing INTV subrecord");
    }

    void LandTexture::save(ESM::ESMWriter& esm, bool isDeleted) const
    {
        esm.writeHNCString("NAME", mId);
        esm.writeHNT("INTV", mIndex);
        esm.writeHNCString("DATA", mTexture);

        if (isDeleted)
        {
            esm.writeHNString("DELE", "", 3);
        }
    }

    void LandTexture::blank()
    {
        mId.clear();
        mTexture.clear();
    }
}
