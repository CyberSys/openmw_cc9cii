#include "bsgn.hpp"

#include "common.hpp"
#include "reader.hpp"
#include "../esm/esmwriter.hpp"

namespace ESM3
{
    unsigned int BirthSign::sRecordId = REC_BSGN;

    void BirthSign::load(Reader& reader, bool& isDeleted)
    {
        isDeleted = false;

        mPowers.mList.clear();

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
                case ESM3::SUB_FNAM: reader.getZString(mName); break;
                case ESM3::SUB_TNAM: reader.getZString(mTexture); break;
                case ESM3::SUB_DESC: reader.getZString(mDescription); break;
                case ESM3::SUB_NPCS:
                {
                    mPowers.add(reader);
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
    }

    void BirthSign::save(ESM::ESMWriter& esm, bool isDeleted) const
    {
        esm.writeHNCString("NAME", mId);

        if (isDeleted)
        {
            esm.writeHNString("DELE", "", 3);
            return;
        }
        esm.writeHNOCString("FNAM", mName);
        esm.writeHNOCString("TNAM", mTexture);
        esm.writeHNOCString("DESC", mDescription);

        mPowers.save(esm);
    }

    void BirthSign::blank()
    {
        mName.clear();
        mDescription.clear();
        mTexture.clear();
        mPowers.mList.clear();
    }

}
