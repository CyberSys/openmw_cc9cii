#include "race.hpp"

//#ifdef NDEBUG
//#undef NDEBUG
//#endif

#include <cassert>

#include "common.hpp"
#include "reader.hpp"
#include "../esm/esmwriter.hpp"

namespace ESM3
{
    unsigned int Race::sRecordId = REC_RACE;

    std::uint32_t Race::MaleFemale::getValue (bool male) const
    {
        return male ? mMale : mFemale;
    }

    float Race::MaleFemaleF::getValue (bool male) const
    {
        return male ? mMale : mFemale;
    }

    void Race::load(Reader& reader, bool& isDeleted)
    {
        isDeleted = false;

        mPowers.mList.clear();

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
                case ESM3::SUB_DESC: reader.getString(mDescription); break; // NOTE: not null terminated
                case ESM3::SUB_RADT:
                {
                    assert (subHdr.dataSize == 140 && "RACE incorrect data size");
                    assert (subHdr.dataSize == sizeof(mData) && "RACE incorrect data size");
                    reader.get(mData);
                    hasData = true;
                    break;
                }
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
            }
        }

        if (!hasName)
            reader.fail("Missing NAME subrecord");

        if (!hasData && !isDeleted)
            reader.fail("Missing RADT subrecord");
    }

    void Race::save(ESM::ESMWriter& esm, bool isDeleted) const
    {
        esm.writeHNCString("NAME", mId);

        if (isDeleted)
        {
            esm.writeHNString("DELE", "", 3);
            return;
        }

        esm.writeHNOCString("FNAM", mName);
        esm.writeHNT("RADT", mData, 140);
        mPowers.save(esm);
        esm.writeHNOString("DESC", mDescription);
    }

    void Race::blank()
    {
        mName.clear();
        mDescription.clear();

        mPowers.mList.clear();

        for (int i=0; i<7; ++i)
        {
            mData.mBonus[i].mSkill = -1;
            mData.mBonus[i].mBonus = 0;
        }

        for (int i=0; i<8; ++i)
            mData.mAttributeValues[i].mMale = mData.mAttributeValues[i].mFemale = 1;

        mData.mHeight.mMale = mData.mHeight.mFemale = 1;
        mData.mWeight.mMale = mData.mWeight.mFemale = 1;

        mData.mFlags = 0;
    }
}
