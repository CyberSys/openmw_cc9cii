#include "creaturestats.hpp"

#include "reader.hpp"
#include "../esm/esmwriter.hpp"

void ESM3::CreatureStats::load (Reader& esm)
{
    int format = esm.getFormat();

    bool intFallback = esm.getFormat() < 11;
    for (int i=0; i<8; ++i)
        mAttributes[i].load (esm, intFallback);

    for (int i=0; i<3; ++i)
        mDynamic[i].load (esm);

    mGoldPool = 0;
    mTradeTime.mDay = 0;
    mTradeTime.mHour = 0;
    mMovementFlags = 0;
    mFallHeight = 0;
    mDrawState = 0;
    mLevel = 1;
    mActorId = -1;
    mDeathAnimation = -1;
    mTimeOfDeath.mDay = 0;
    mTimeOfDeath.mHour = 0;
    mHasAiSettings = false;

    int flags = 0;
    mDead = false;
    mDeathAnimationFinished = false;
    mDied = false;
    mMurdered = false;
    mTalkedTo = false;
    mAlarmed = false;
    mAttacked = false;
    mKnockdown = false;
    mKnockdownOneFrame = false;
    mKnockdownOverOneFrame = false;
    mHitRecovery = false;
    mBlock = false;
    mRecalcDynamicStats = false;

    bool finished = false;
    while (!finished && esm.getSubRecordHeader())
    {
        const ESM3::SubRecordHeader& subHdr = esm.subRecordHeader();
        switch (subHdr.typeId)
        {
            case ESM3::SUB_GOLD: esm.get(mGoldPool); break;
            case ESM3::SUB_TIME: esm.get(mTradeTime); break;
            case ESM3::SUB_AFLG:
            {
                assert(format >= 8);
                esm.get(flags);
                mDead = flags & Dead;
                mDeathAnimationFinished = flags & DeathAnimationFinished;
                mDied = flags & Died;
                mMurdered = flags & Murdered;
                mTalkedTo = flags & TalkedTo;
                mAlarmed = flags & Alarmed;
                mAttacked = flags & Attacked;
                mKnockdown = flags & Knockdown;
                mKnockdownOneFrame = flags & KnockdownOneFrame;
                mKnockdownOverOneFrame = flags & KnockdownOverOneFrame;
                mHitRecovery = flags & HitRecovery;
                mBlock = flags & Block;
                mRecalcDynamicStats = flags & RecalcDynamicStats;
                break;
            }
            case ESM3::SUB_DEAD:
            {
                assert(format < 8);
                esm.get(mDead);
                if (format < 3 && mDead)
                    mDeathAnimationFinished = true;
                break;
            }
            case ESM3::SUB_DFNT: assert(format < 8); esm.get(mDeathAnimationFinished); break;
            case ESM3::SUB_DIED: assert(format < 8); esm.get(mDied); break;
            case ESM3::SUB_MURD: assert(format < 8); esm.get(mMurdered); break;
            case ESM3::SUB_FRHT: assert(format < 8); esm.skipSubRecordData(); break; // Friendly hits, no longer used
            case ESM3::SUB_TALK: assert(format < 8); esm.get(mTalkedTo); break;
            case ESM3::SUB_ALRM: assert(format < 8); esm.get(mAlarmed); break;
            case ESM3::SUB_ATKD: assert(format < 8); esm.get(mAttacked); break;
            case ESM3::SUB_HOST: assert(format < 8); esm.skipSubRecordData(); break; // Hostile, no longer used
            case ESM3::SUB_ATCK: assert(format < 8); esm.skipSubRecordData(); break; // AttackingOrSpell, no longer used
            case ESM3::SUB_KNCK: assert(format < 8); esm.get(mKnockdown); break;
            case ESM3::SUB_KNC1: assert(format < 8); esm.get(mKnockdownOneFrame); break;
            case ESM3::SUB_KNCO: assert(format < 8); esm.get(mKnockdownOverOneFrame); break;
            case ESM3::SUB_HITR: assert(format < 8); esm.get(mHitRecovery); break;
            case ESM3::SUB_BLCK: assert(format < 8); esm.get(mBlock); break;
            case ESM3::SUB_MOVE: esm.get(mMovementFlags); break;
            case ESM3::SUB_ASTR: esm.skipSubRecordData(); break; // attackStrength, no longer used
            case ESM3::SUB_FALL: esm.get(mFallHeight); break;
            case ESM3::SUB_LHIT: esm.get(mLastHitObject); break;
            case ESM3::SUB_LHAT: esm.get(mLastHitAttemptObject); break;
            case ESM3::SUB_CALC: assert(format < 8); esm.get(mRecalcDynamicStats); break;
            case ESM3::SUB_DRAW: esm.get(mDrawState); break;
            case ESM3::SUB_LEVL: esm.get(mLevel); break;
            case ESM3::SUB_ACID: esm.get(mActorId); break;
            case ESM3::SUB_DANM: esm.get(mDeathAnimation); break;
            case ESM3::SUB_DTIM: esm.get(mTimeOfDeath); break;
            case ESM3::SUB_SPEL:
            {
                esm.cacheSubRecordHeader();
                mSpells.load(esm);
                break;
            }
            case ESM3::SUB_ID__:
            {
                esm.cacheSubRecordHeader();
                mActiveSpells.load(esm);
                break;
            }
            case ESM3::SUB_AIPK:
            {
                esm.cacheSubRecordHeader();
                mAiSequence.load(esm);
                break;
            }
            case ESM3::SUB_LAST: // FIXME: this should not occur here, see Arwen converted save
            {
                esm.skipSubRecordData();
                break;
            }
            case ESM3::SUB_EFID:
            {
                esm.cacheSubRecordHeader();
                mMagicEffects.load(esm);
                break;
            }
            case ESM3::SUB_SUMM:
            {
                int magicEffect;
                esm.get(magicEffect);

                std::string source = "";
                int effectIndex = -1;
                int actorId;
                esm.getSubRecordHeader();
                if (esm.subRecordHeader().typeId == ESM3::SUB_SOUR)
                {
                    esm.getZString(source);
                    esm.getSubRecordHeader();
                }

                if (esm.subRecordHeader().typeId == ESM3::SUB_EIND)
                {
                    esm.get(effectIndex);
                    esm.getSubRecordHeader();
                }

                assert(esm.subRecordHeader().typeId == ESM3::SUB_ACID);
                esm.get(actorId);

                mSummonedCreatureMap[SummonKey(magicEffect, source, effectIndex)] = actorId;
                break;
            }
            case ESM3::SUB_GRAV:
            {
                int actorId;
                esm.get(actorId);
                mSummonGraveyard.push_back(actorId);
                break;
            }
            case ESM3::SUB_AISE: esm.get(mHasAiSettings); break;
            case ESM3::SUB_CORP:
            {
                std::string id;
                esm.getZString(id);

                CorprusStats stats;
                esm.getSubRecordHeader();
                assert(esm.subRecordHeader().typeId == ESM3::SUB_WORS);
                esm.get(stats.mWorsenings);
                esm.getSubRecordHeader();
                assert(esm.subRecordHeader().typeId == ESM3::SUB_TIME);
                esm.get(stats.mNextWorsening);

                mCorprusSpells[id] = stats;
                break;
            }
            default:
                finished = true;
                 esm.cacheSubRecordHeader();
                 break;
        }
    }

    if (mHasAiSettings)
    {
        for (int i=0; i<4; ++i)
            mAiSettings[i].load(esm);
    }
}

void ESM3::CreatureStats::save (ESM::ESMWriter& esm) const
{
    for (int i=0; i<8; ++i)
        mAttributes[i].save (esm);

    for (int i=0; i<3; ++i)
        mDynamic[i].save (esm);

    if (mGoldPool)
        esm.writeHNT ("GOLD", mGoldPool);

    if (mTradeTime.mDay != 0 || mTradeTime.mHour != 0)
        esm.writeHNT ("TIME", mTradeTime);

    int flags = 0;
    if (mDead) flags |= Dead;
    if (mDeathAnimationFinished) flags |= DeathAnimationFinished;
    if (mDied) flags |= Died;
    if (mMurdered) flags |= Murdered;
    if (mTalkedTo) flags |= TalkedTo;
    if (mAlarmed) flags |= Alarmed;
    if (mAttacked) flags |= Attacked;
    if (mKnockdown) flags |= Knockdown;
    if (mKnockdownOneFrame) flags |= KnockdownOneFrame;
    if (mKnockdownOverOneFrame) flags |= KnockdownOverOneFrame;
    if (mHitRecovery) flags |= HitRecovery;
    if (mBlock) flags |= Block;
    if (mRecalcDynamicStats) flags |= RecalcDynamicStats;

    if (flags)
        esm.writeHNT ("AFLG", flags);

    if (mMovementFlags)
        esm.writeHNT ("MOVE", mMovementFlags);

    if (mFallHeight)
        esm.writeHNT ("FALL", mFallHeight);

    if (!mLastHitObject.empty())
        esm.writeHNString ("LHIT", mLastHitObject);

    if (!mLastHitAttemptObject.empty())
        esm.writeHNString ("LHAT", mLastHitAttemptObject);

    if (mDrawState)
        esm.writeHNT ("DRAW", mDrawState);

    if (mLevel != 1)
        esm.writeHNT ("LEVL", mLevel);

    if (mActorId != -1)
        esm.writeHNT ("ACID", mActorId);

    if (mDeathAnimation != -1)
        esm.writeHNT ("DANM", mDeathAnimation);

    if (mTimeOfDeath.mHour != 0 || mTimeOfDeath.mDay != 0)
        esm.writeHNT ("DTIM", mTimeOfDeath);

    mSpells.save(esm);
    mActiveSpells.save(esm);
    mAiSequence.save(esm);
    mMagicEffects.save(esm);

    for (const auto& summon : mSummonedCreatureMap)
    {
        esm.writeHNT ("SUMM", summon.first.mEffectId);
        esm.writeHNString ("SOUR", summon.first.mSourceId);
        int effectIndex = summon.first.mEffectIndex;
        if (effectIndex != -1)
            esm.writeHNT ("EIND", effectIndex);
        esm.writeHNT ("ACID", summon.second);
    }

    for (int key : mSummonGraveyard)
    {
        esm.writeHNT ("GRAV", key);
    }

    esm.writeHNT("AISE", mHasAiSettings);
    if (mHasAiSettings)
    {
        for (int i=0; i<4; ++i)
            mAiSettings[i].save(esm);
    }

    for (const auto& corprusSpell : mCorprusSpells)
    {
        esm.writeHNString("CORP", corprusSpell.first);

        const CorprusStats & stats = corprusSpell.second;
        esm.writeHNT("WORS", stats.mWorsenings);
        esm.writeHNT("TIME", stats.mNextWorsening);
    }
}

void ESM3::CreatureStats::blank()
{
    mTradeTime.mHour = 0;
    mTradeTime.mDay = 0;
    mGoldPool = 0;
    mActorId = -1;
    mHasAiSettings = false;
    mDead = false;
    mDeathAnimationFinished = false;
    mDied = false;
    mMurdered = false;
    mTalkedTo = false;
    mAlarmed = false;
    mAttacked = false;
    mKnockdown = false;
    mKnockdownOneFrame = false;
    mKnockdownOverOneFrame = false;
    mHitRecovery = false;
    mBlock = false;
    mMovementFlags = 0;
    mFallHeight = 0.f;
    mRecalcDynamicStats = false;
    mDrawState = 0;
    mDeathAnimation = -1;
    mLevel = 1;
    mCorprusSpells.clear();
}
