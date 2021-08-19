#include "ench.hpp"

#include <cassert>

#include "common.hpp"
#include "reader.hpp"
#include "../esm/esmwriter.hpp"

namespace ESM3
{
    unsigned int Enchantment::sRecordId = REC_ENCH;

    void Enchantment::load(Reader& reader, bool& isDeleted)
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
                case ESM3::SUB_ENDT:
                {
                    assert (subHdr.dataSize == 16 && "ENCH incorrect data size");
                    assert (subHdr.dataSize == sizeof(mData) && "ENCH incorrect data size");
                    reader.get(mData);
                    hasData = true;
                    break;
                }
                case ESM3::SUB_ENAM:
                {
                    //mEffects.add(reader);
                    ENAMstruct s;
                    assert (subHdr.dataSize == 24 && "ENCH effect size mismatch");
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
            reader.fail("Missing ENDT subrecord");
    }

    void Enchantment::save(ESM::ESMWriter& esm, bool isDeleted) const
    {
        esm.writeHNCString("NAME", mId);

        if (isDeleted)
        {
            esm.writeHNString("DELE", "", 3);
            return;
        }

        esm.writeHNT("ENDT", mData, 16);
        mEffects.save(esm);
    }

    void Enchantment::blank()
    {
        mData.mType = 0;
        mData.mCost = 0;
        mData.mCharge = 0;
        mData.mFlags = 0;

        mEffects.mList.clear();
    }
}
