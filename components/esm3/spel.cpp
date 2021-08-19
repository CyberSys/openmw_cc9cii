#include "spel.hpp"

#include <cassert>

#include "common.hpp"
#include "reader.hpp"
#include "../esm/esmwriter.hpp"

namespace ESM3
{
    unsigned int Spell::sRecordId = REC_SPEL;

    void Spell::load(Reader& reader, bool& isDeleted)
    {
        isDeleted = false;

        mEffects.mList.clear();

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
                case ESM3::SUB_FNAM: reader.getZString(mName); break;
                case ESM3::SUB_SPDT:
                {
                    assert (subHdr.dataSize == 12 && "SPEL incorrect data size");
                    assert (subHdr.dataSize == sizeof(mData) && "SPEL incorrect data size");
                    reader.get(mData);
                    hasData = true;
                    break;
                }
                case ESM3::SUB_ENAM:
                {
                    assert (subHdr.dataSize == 24 && "SPEL incorrect effect size");
                    assert (subHdr.dataSize == sizeof(ENAMstruct) && "SPEL incorrect effect size");
                    ENAMstruct s;
                    reader.get(s);
                    mEffects.mList.push_back(s);
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
            reader.fail("Missing SPDT subrecord");
    }

    void Spell::save(ESM::ESMWriter& esm, bool isDeleted) const
    {
        esm.writeHNCString("NAME", mId);

        if (isDeleted)
        {
            esm.writeHNString("DELE", "", 3);
            return;
        }

        esm.writeHNOCString("FNAM", mName);
        esm.writeHNT("SPDT", mData, 12);
        mEffects.save(esm);
    }

    void Spell::blank()
    {
        mData.mType = 0;
        mData.mCost = 0;
        mData.mFlags = 0;

        mName.clear();
        mEffects.mList.clear();
    }
}
