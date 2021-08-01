#include "npc_.hpp"

//#ifdef NDEBUG
//#undef NDEBUG
//#endif

#include <cassert>

#include "common.hpp"
#include "reader.hpp"
#include "../esm/esmwriter.hpp"

namespace ESM3
{
    unsigned int NPC::sRecordId = REC_NPC_;

    void NPC::load(Reader& reader, bool& isDeleted)
    {
        isDeleted = false;
        mRecordFlags = reader.getRecordFlags();

        mSpells.mList.clear();
        mInventory.mList.clear();
        mTransport.mList.clear();
        mAiPackage.mList.clear();
        mAiData.blank();
        mAiData.mHello = mAiData.mFight = mAiData.mFlee = 30;

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
                case ESM3::SUB_RNAM: reader.getZString(mRace); break;
                case ESM3::SUB_CNAM: reader.getZString(mClass); break;
                case ESM3::SUB_ANAM: reader.getZString(mFaction); break;
                case ESM3::SUB_BNAM: reader.getZString(mHead); break;
                case ESM3::SUB_KNAM: reader.getZString(mHair); break;
                case ESM3::SUB_SCRI: reader.getZString(mScript); break;
                case ESM3::SUB_NPDT:
                {
                    hasNpdt = true;
                    if (subHdr.dataSize == 52)
                    {
                        mNpdtType = NPC_DEFAULT;
                        assert (subHdr.dataSize == sizeof(mNpdt) && "NPC_ data size mismatch");
                        reader.get(mNpdt);
                    }
                    else if (subHdr.dataSize == 12)
                    {
                        //Reading into temporary NPDTstruct12 object
                        NPDTstruct12 npdt12;
                        mNpdtType = NPC_WITH_AUTOCALCULATED_STATS;
                        assert (subHdr.dataSize == sizeof(npdt12) && "NPC_ data size mismatch");
                        reader.get(npdt12);

                        //Clearing the mNdpt struct to initialize all values
                        blankNpdt();
                        //Swiching to an internal representation
                        mNpdt.mLevel = npdt12.mLevel;
                        mNpdt.mDisposition = npdt12.mDisposition;
                        mNpdt.mReputation = npdt12.mReputation;
                        mNpdt.mRank = npdt12.mRank;
                        mNpdt.mGold = npdt12.mGold;
                    }
                    else
                        reader.fail("NPC_NPDT must be 12 or 52 bytes long");
                    break;
                }
                case ESM3::SUB_FLAG:
                {
                    int flags;
                    assert (subHdr.dataSize == 4 && "NPC_ flag size mismatch");
                    reader.get(flags);
                    mFlags = flags & 0xFF;
                    mBloodType = ((flags >> 8) & 0xFF) >> 2;
                    hasFlags = true;
                    break;
                }
                case ESM3::SUB_NPCO: mInventory.add(reader); break;
                case ESM3::SUB_NPCS: mSpells.add(reader); break;
                case ESM3::SUB_AIDT:
                {
                    assert (subHdr.dataSize == sizeof(mAiData) && "NPC_ AiData size mismatch");
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

    void NPC::save(ESM::ESMWriter& esm, bool isDeleted) const
    {
        esm.writeHNCString("NAME", mId);

        if (isDeleted)
        {
            esm.writeHNString("DELE", "", 3);
            return;
        }

        esm.writeHNOCString("MODL", mModel);
        esm.writeHNOCString("FNAM", mName);
        esm.writeHNCString("RNAM", mRace);
        esm.writeHNCString("CNAM", mClass);
        esm.writeHNCString("ANAM", mFaction);
        esm.writeHNCString("BNAM", mHead);
        esm.writeHNCString("KNAM", mHair);
        esm.writeHNOCString("SCRI", mScript);

        if (mNpdtType == NPC_DEFAULT)
        {
            esm.writeHNT("NPDT", mNpdt, 52);
        }
        else if (mNpdtType == NPC_WITH_AUTOCALCULATED_STATS)
        {
            NPDTstruct12 npdt12;
            npdt12.mLevel = mNpdt.mLevel;
            npdt12.mDisposition = mNpdt.mDisposition;
            npdt12.mReputation = mNpdt.mReputation;
            npdt12.mRank = mNpdt.mRank;
            npdt12.mGold = mNpdt.mGold;
            esm.writeHNT("NPDT", npdt12, 12);
        }

        esm.writeHNT("FLAG", ((mBloodType << 10) + mFlags));

        mInventory.save(esm);
        mSpells.save(esm);
        esm.writeHNT("AIDT", mAiData, sizeof(mAiData));

        mTransport.save(esm);

        mAiPackage.save(esm);
    }

    bool NPC::isMale() const {
        return (mFlags & Female) == 0;
    }

    void NPC::setIsMale(bool value) {
        mFlags |= Female;
        if (value) {
            mFlags ^= Female;
        }
    }

    void NPC::blank()
    {
        mNpdtType = NPC_DEFAULT;
        blankNpdt();
        mBloodType = 0;
        mFlags = 0;
        mInventory.mList.clear();
        mSpells.mList.clear();
        mAiData.blank();
        mAiData.mHello = mAiData.mFight = mAiData.mFlee = 30;
        mTransport.mList.clear();
        mAiPackage.mList.clear();
        mName.clear();
        mModel.clear();
        mRace.clear();
        mClass.clear();
        mFaction.clear();
        mScript.clear();
        mHair.clear();
        mHead.clear();
    }

    void NPC::blankNpdt()
    {
        mNpdt.mLevel = 0;
        mNpdt.mStrength = mNpdt.mIntelligence = mNpdt.mWillpower = mNpdt.mAgility =
            mNpdt.mSpeed = mNpdt.mEndurance = mNpdt.mPersonality = mNpdt.mLuck = 0;
        for (int i=0; i< Skill::Length; ++i) mNpdt.mSkills[i] = 0;
        mNpdt.mReputation = 0;
        mNpdt.mHealth = mNpdt.mMana = mNpdt.mFatigue = 0;
        mNpdt.mDisposition = 0;
        mNpdt.mUnknown1 = 0;
        mNpdt.mRank = 0;
        mNpdt.mUnknown2 = 0;
        mNpdt.mGold = 0;
    }

    int NPC::getFactionRank() const
    {
        if (mFaction.empty())
            return -1;
        else
            return mNpdt.mRank;
    }

    const std::vector<Transport::Dest>& NPC::getTransport() const
    {
        return mTransport.mList;
    }
}
