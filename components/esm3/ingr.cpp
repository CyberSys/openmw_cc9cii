#include "ingr.hpp"

//#ifdef NDEBUG
//#undef NDEBUG
//#endif

#include <cassert>

#include "common.hpp"
#include "reader.hpp"
#include "../esm/esmwriter.hpp"

namespace ESM3
{
    unsigned int Ingredient::sRecordId = REC_INGR;

    void Ingredient::load(Reader& reader, bool& isDeleted)
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
                case ESM3::SUB_IRDT:
                {
                    assert (subHdr.dataSize == 56 && "INGR data size mismatch");
                    assert (subHdr.dataSize == sizeof(mData) && "INGR data size mismatch");
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
            reader.fail("Missing IRDT subrecord");

        // horrible hack to fix broken data in records
        for (int i=0; i<4; ++i)
        {
            if (mData.mEffectID[i] != 85 &&
                mData.mEffectID[i] != 22 &&
                mData.mEffectID[i] != 17 &&
                mData.mEffectID[i] != 79 &&
                mData.mEffectID[i] != 74)
            {
                mData.mAttributes[i] = -1;
            }

            // is this relevant in cycle from 0 to 4?
            if (mData.mEffectID[i] != 89 &&
                mData.mEffectID[i] != 26 &&
                mData.mEffectID[i] != 21 &&
                mData.mEffectID[i] != 83 &&
                mData.mEffectID[i] != 78)
            {
                mData.mSkills[i] = -1;
            }
        }
    }

    void Ingredient::save(ESM::ESMWriter& esm, bool isDeleted) const
    {
        esm.writeHNCString("NAME", mId);

        if (isDeleted)
        {
            esm.writeHNString("DELE", "", 3);
            return;
        }

        esm.writeHNCString("MODL", mModel);
        esm.writeHNOCString("FNAM", mName);
        esm.writeHNT("IRDT", mData, 56);
        esm.writeHNOCString("SCRI", mScript);
        esm.writeHNOCString("ITEX", mIcon);
    }

    void Ingredient::blank()
    {
        mData.mWeight = 0;
        mData.mValue = 0;
        for (int i=0; i<4; ++i)
        {
            mData.mEffectID[i] = 0;
            mData.mSkills[i] = 0;
            mData.mAttributes[i] = 0;
        }

        mName.clear();
        mModel.clear();
        mIcon.clear();
        mScript.clear();
    }
}
