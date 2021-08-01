#include "crea.hpp"

//#ifdef NDEBUG
//#undef NDEBUG
//#endif

#include <cassert>

#include <components/debug/debuglog.hpp>

#include "common.hpp"
#include "reader.hpp"
#include "../esm/esmwriter.hpp"

namespace ESM3
{
    unsigned int Creature::sRecordId = REC_CREA;

    void Creature::load(Reader& reader, bool& isDeleted)
    {
        isDeleted = false;
        mRecordFlags = reader.getRecordFlags();

        mAiPackage.mList.clear();
        mInventory.mList.clear();
        mSpells.mList.clear();
        mTransport.mList.clear();

        mScale = 1.f;
        mAiData.blank();
        mAiData.mFight = 90;
        mAiData.mFlee = 20;

        bool hasName = false;
        bool hasNpdt = false;
        bool hasFlags = false;
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
                case ESM3::SUB_FNAM: reader.getZString(mName); break;
                case ESM3::SUB_SCRI: reader.getZString(mScript); break;
                case ESM3::SUB_CNAM: reader.getZString(mOriginal); break;
                case ESM3::SUB_NPDT:
                {
                    assert (subHdr.dataSize == 96 && "CREA data size mismatch");
                    assert (subHdr.dataSize == sizeof(mData) && "CREA data size mismatch");
                    reader.get(mData);
                    hasNpdt = true;
                    break;
                }
                case ESM3::SUB_FLAG:
                {
                    int flags;
                    assert (subHdr.dataSize == 4 && "CREA flag size mismatch");
                    reader.get(flags);
                    mFlags = flags & 0xFF;
                    mBloodType = ((flags >> 8) & 0xFF) >> 2;
                    hasFlags = true;
                    break;
                }
                case ESM3::SUB_XSCL: reader.get(mScale); break;
                case ESM3::SUB_NPCO: mInventory.add(reader); break;
                case ESM3::SUB_NPCS: mSpells.add(reader); break;
                case ESM3::SUB_AIDT:
                {
                    assert (subHdr.dataSize == sizeof(mAiData) && "CREA AiData size mismatch");
                    reader.get(mAiData, sizeof(mAiData));
                    break;
                }
                case ESM3::SUB_DODT:
                case ESM3::SUB_DNAM:
                {
                    mTransport.add(reader);
                    break;
                }
                case ESM3::SUB_AI_A: // Activate
                case ESM3::SUB_AI_E: // Escort
                case ESM3::SUB_AI_F: // Follow
                case ESM3::SUB_AI_T: // Travel
                case ESM3::SUB_AI_W: // Wander
                case ESM3::SUB_CNDT: // Cell
                {
                    mAiPackage.add(reader);
                    break;
                }
                case ESM3::SUB_INDX:
                {
                    // seems to occur only in .ESS files, unsure of purpose
                    int index;
                    reader.get(index);
                    Log(Debug::Warning) << "Creature::load: Unhandled INDX " << index;
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

        if (!hasNpdt && !isDeleted)
            reader.fail("Missing NPDT subrecord");

        if (!hasFlags && !isDeleted)
            reader.fail("Missing FLAG subrecord");
    }

    void Creature::save(ESM::ESMWriter& esm, bool isDeleted) const
    {
        esm.writeHNCString("NAME", mId);

        if (isDeleted)
        {
            esm.writeHNString("DELE", "", 3);
            return;
        }

        esm.writeHNCString("MODL", mModel);
        esm.writeHNOCString("CNAM", mOriginal);
        esm.writeHNOCString("FNAM", mName);
        esm.writeHNOCString("SCRI", mScript);
        esm.writeHNT("NPDT", mData, 96);
        esm.writeHNT("FLAG", ((mBloodType << 10) + mFlags));
        if (mScale != 1.0) {
            esm.writeHNT("XSCL", mScale);
        }

        mInventory.save(esm);
        mSpells.save(esm);
        esm.writeHNT("AIDT", mAiData, sizeof(mAiData));
        mTransport.save(esm);
        mAiPackage.save(esm);
    }

    void Creature::blank()
    {
        mData.mType = 0;
        mData.mLevel = 0;
        mData.mStrength = mData.mIntelligence = mData.mWillpower = mData.mAgility =
            mData.mSpeed = mData.mEndurance = mData.mPersonality = mData.mLuck = 0;
        mData.mHealth = mData.mMana = mData.mFatigue = 0;
        mData.mSoul = 0;
        mData.mCombat = mData.mMagic = mData.mStealth = 0;
        for (int i=0; i<6; ++i) mData.mAttack[i] = 0;
        mData.mGold = 0;
        mBloodType = 0;
        mFlags = 0;
        mScale = 1.f;
        mModel.clear();
        mName.clear();
        mScript.clear();
        mOriginal.clear();
        mInventory.mList.clear();
        mSpells.mList.clear();
        mAiData.blank();
        mAiData.mFight = 90;
        mAiData.mFlee = 20;
        mAiPackage.mList.clear();
        mTransport.mList.clear();
    }

    const std::vector<Transport::Dest>& Creature::getTransport() const
    {
        return mTransport.mList;
    }
}
