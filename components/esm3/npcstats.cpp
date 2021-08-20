#include "npcstats.hpp"

#include <cassert>

#include "reader.hpp"
#include "../esm/esmwriter.hpp"

ESM3::NpcStats::Faction::Faction() : mExpelled (false), mRank (-1), mReputation (0) {}

void ESM3::NpcStats::load (Reader& esm)
{
    while (esm.getNextSubRecordHeader(ESM3::SUB_FACT))
    {
        std::string id;
        esm.getString(id); // FIXME: check if string is null terminated

        Faction faction;

        int expelled = 0;
        if (esm.getNextSubRecordHeader(ESM3::SUB_FAEX))
            esm.get(expelled);

        if (expelled)
            faction.mExpelled = true;

        if (esm.getNextSubRecordHeader(ESM3::SUB_FARA))
            esm.get(faction.mRank);

        if (esm.getNextSubRecordHeader(ESM3::SUB_FARE))
            esm.get(faction.mReputation);

        mFactions.insert (std::make_pair (id, faction));
    }

    mDisposition = 0;
    if (esm.getNextSubRecordHeader(ESM3::SUB_DISP))
        esm.get(mDisposition);

    bool intFallback = esm.getFormat() < 11;
    for (int i=0; i<27; ++i)
        mSkills[i].load (esm, intFallback);

    mWerewolfDeprecatedData = false;
    if (esm.getFormat() < 8 && esm.getNextSubRecordType() == ESM3::SUB_STBA)
    {
        // we have deprecated werewolf skills, stored interleaved
        // Load into one big vector, then remove every 2nd value
        mWerewolfDeprecatedData = true;
        std::vector<ESM3::StatState<float> > skills(mSkills, mSkills + sizeof(mSkills)/sizeof(mSkills[0]));

        for (int i=0; i<27; ++i)
        {
            ESM3::StatState<float> skill;
            skill.load(esm, intFallback);
            skills.push_back(skill);
        }

        int i=0;
        for (std::vector<ESM3::StatState<float> >::iterator it = skills.begin(); it != skills.end(); ++i)
        {
            if (i%2 == 1)
                it = skills.erase(it);
            else
                ++it;
        }
        assert(skills.size() == 27);
        std::copy(skills.begin(), skills.end(), mSkills);
    }

    // No longer used
    bool hasWerewolfAttributes = false;
    if (esm.getNextSubRecordHeader(ESM3::SUB_HWAT))
        esm.get(hasWerewolfAttributes);

    if (hasWerewolfAttributes)
    {
        ESM3::StatState<int> dummy;
        for (int i=0; i<8; ++i)
            dummy.load(esm, intFallback);
        mWerewolfDeprecatedData = true;
    }

    mIsWerewolf = false;
    if (esm.getNextSubRecordHeader(ESM3::SUB_WOLF))
        esm.get(mIsWerewolf);

    mBounty = 0;
    if (esm.getNextSubRecordHeader(ESM3::SUB_BOUN))
        esm.get(mBounty);

    mReputation = 0;
    if (esm.getNextSubRecordHeader(ESM3::SUB_REPU))
        esm.get(mReputation);

    mWerewolfKills = 0;
    if (esm.getNextSubRecordHeader(ESM3::SUB_WKIL))
        esm.get(mWerewolfKills);

    // No longer used
    if (esm.getNextSubRecordHeader(ESM3::SUB_PROF))
        esm.skipSubRecordData(); // int profit

    // No longer used
    if (esm.getNextSubRecordHeader(ESM3::SUB_ASTR))
        esm.skipSubRecordData(); // attackStrength

    mLevelProgress = 0;
    if (esm.getNextSubRecordHeader(ESM3::SUB_LPRO))
        esm.get(mLevelProgress);

    for (int i = 0; i < 8; ++i)
        mSkillIncrease[i] = 0;
    if (esm.getNextSubRecordHeader(ESM3::SUB_INCR))
        esm.get(mSkillIncrease);

    for (int i=0; i<3; ++i)
        mSpecIncreases[i] = 0;
    if (esm.getNextSubRecordHeader(ESM3::SUB_SPEC))
        esm.get(mSpecIncreases);

    while (esm.getNextSubRecordHeader(ESM3::SUB_USED))
    {
        std::string used;
        esm.getString(used); // FIXME: check if string null terminated
        mUsedIds.push_back(used);
    }

    mTimeToStartDrowning = 0;
    if (esm.getNextSubRecordHeader(ESM3::SUB_DRTI))
        esm.get(mTimeToStartDrowning);

    // No longer used
    float lastDrowningHit = 0;
    if (esm.getNextSubRecordHeader(ESM3::SUB_DRLH))
        esm.get(lastDrowningHit);

    // No longer used
    float levelHealthBonus = 0;
    if (esm.getNextSubRecordHeader(ESM3::SUB_LVLH))
        esm.get(levelHealthBonus);

    mCrimeId = -1;
    if (esm.getNextSubRecordHeader(ESM3::SUB_CRID))
        esm.get(mCrimeId);
}

void ESM3::NpcStats::save (ESM::ESMWriter& esm) const
{
    for (std::map<std::string, Faction>::const_iterator iter (mFactions.begin());
        iter!=mFactions.end(); ++iter)
    {
        esm.writeHNString ("FACT", iter->first);

        if (iter->second.mExpelled)
        {
            int expelled = 1;
            esm.writeHNT ("FAEX", expelled);
        }

        if (iter->second.mRank >= 0)
            esm.writeHNT ("FARA", iter->second.mRank);

        if (iter->second.mReputation)
            esm.writeHNT ("FARE", iter->second.mReputation);
    }

    if (mDisposition)
        esm.writeHNT ("DISP", mDisposition);

    for (int i=0; i<27; ++i)
        mSkills[i].save (esm);

    if (mIsWerewolf)
        esm.writeHNT ("WOLF", mIsWerewolf);

    if (mBounty)
        esm.writeHNT ("BOUN", mBounty);

    if (mReputation)
        esm.writeHNT ("REPU", mReputation);

    if (mWerewolfKills)
        esm.writeHNT ("WKIL", mWerewolfKills);

    if (mLevelProgress)
        esm.writeHNT ("LPRO", mLevelProgress);

    bool saveSkillIncreases = false;
    for (int i = 0; i < 8; ++i)
    {
        if (mSkillIncrease[i] != 0)
        {
            saveSkillIncreases = true;
            break;
        }
    }
    if (saveSkillIncreases)
        esm.writeHNT ("INCR", mSkillIncrease);

    if (mSpecIncreases[0] != 0 ||
        mSpecIncreases[1] != 0 ||
        mSpecIncreases[2] != 0)
    esm.writeHNT ("SPEC", mSpecIncreases);

    for (std::vector<std::string>::const_iterator iter (mUsedIds.begin()); iter!=mUsedIds.end();
        ++iter)
        esm.writeHNString ("USED", *iter);

    if (mTimeToStartDrowning)
        esm.writeHNT ("DRTI", mTimeToStartDrowning);

    if (mCrimeId != -1)
        esm.writeHNT ("CRID", mCrimeId);
}

void ESM3::NpcStats::blank()
{
    mWerewolfDeprecatedData = false;
    mIsWerewolf = false;
    mDisposition = 0;
    mBounty = 0;
    mReputation = 0;
    mWerewolfKills = 0;
    mLevelProgress = 0;
    for (int i=0; i<8; ++i)
        mSkillIncrease[i] = 0;
    for (int i=0; i<3; ++i)
        mSpecIncreases[i] = 0;
    mTimeToStartDrowning = 20;
    mCrimeId = -1;
}
