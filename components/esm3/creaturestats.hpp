#ifndef ESM_CREATURESTATS_H
#define ESM_CREATURESTATS_H

#include <string>
#include <vector>
#include <map>

#include "statstate.hpp"

#include "../esm/defs.hpp"

#include "../esm/attr.hpp"
#include "spellstate.hpp"
#include "activespells.hpp"
#include "magiceffects.hpp"
#include "aisequence.hpp"

namespace ESM
{
    class ESMWriter;
}

namespace ESM3
{
    class Reader;

    // format 0, saved games only
    struct CreatureStats
    {
        struct CorprusStats
        {
            int mWorsenings[ESM::Attribute::Length];
            ESM::TimeStamp mNextWorsening;
        };

        StatState<float> mAttributes[ESM::Attribute::Length];
        StatState<float> mDynamic[3];

        ESM3::MagicEffects mMagicEffects;

        ESM3::AiSequence::AiSequence mAiSequence;

        bool mHasAiSettings;
        StatState<int> mAiSettings[4];

        std::map<ESM3::SummonKey, int> mSummonedCreatureMap;
        std::vector<int> mSummonGraveyard;

        ESM::TimeStamp mTradeTime;
        int mGoldPool;
        int mActorId;
        //int mHitAttemptActorId;

        enum Flags
        {
            Dead                   = 0x0001,
            DeathAnimationFinished = 0x0002,
            Died                   = 0x0004,
            Murdered               = 0x0008,
            TalkedTo               = 0x0010,
            Alarmed                = 0x0020,
            Attacked               = 0x0040,
            Knockdown              = 0x0080,
            KnockdownOneFrame      = 0x0100,
            KnockdownOverOneFrame  = 0x0200,
            HitRecovery            = 0x0400,
            Block                  = 0x0800,
            RecalcDynamicStats     = 0x1000
        };
        bool mDead;
        bool mDeathAnimationFinished;
        bool mDied;
        bool mMurdered;
        bool mTalkedTo;
        bool mAlarmed;
        bool mAttacked;
        bool mKnockdown;
        bool mKnockdownOneFrame;
        bool mKnockdownOverOneFrame;
        bool mHitRecovery;
        bool mBlock;
        unsigned int mMovementFlags;
        float mFallHeight;
        std::string mLastHitObject;
        std::string mLastHitAttemptObject;
        bool mRecalcDynamicStats;
        int mDrawState;
        signed char mDeathAnimation;
        ESM::TimeStamp mTimeOfDeath;
        int mLevel;

        std::map<std::string, CorprusStats> mCorprusSpells;
        SpellState mSpells;
        ESM3::ActiveSpells mActiveSpells;

        /// Initialize to default state
        void blank();

        void load (Reader& esm);
        void save (ESM::ESMWriter& esm) const;
    };
}

#endif
