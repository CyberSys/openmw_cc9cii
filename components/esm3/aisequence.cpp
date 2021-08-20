#include "aisequence.hpp"

#include "reader.hpp"
#include "../esm/esmwriter.hpp"

#include <memory>

namespace ESM3
{
namespace AiSequence
{

    void AiWander::load(Reader& esm)
    {
        esm.getSubRecordHeader(ESM3::SUB_DATA);
        esm.get(mData);

        esm.getSubRecordHeader(ESM3::SUB_STAR);
        esm.get(mDurationData); // was mStartTime

        mStoredInitialActorPosition = false;
        if (esm.getNextSubRecordHeader(ESM3::SUB_POS_))
        {
            mStoredInitialActorPosition = true;
            esm.get(mInitialActorPosition);
        }
    }

    void AiWander::save(ESM::ESMWriter& esm) const
    {
        esm.writeHNT ("DATA", mData);
        esm.writeHNT ("STAR", mDurationData);
        if (mStoredInitialActorPosition)
            esm.writeHNT ("POS_", mInitialActorPosition);
    }

    void AiTravel::load(Reader& esm)
    {
        esm.getSubRecordHeader(ESM3::SUB_DATA);
        esm.get(mData);

        if (esm.getNextSubRecordHeader(ESM3::SUB_HIDD))
            esm.get(mHidden);
    }

    void AiTravel::save(ESM::ESMWriter& esm) const
    {
        esm.writeHNT ("DATA", mData);
        esm.writeHNT ("HIDD", mHidden);
    }

    void AiEscort::load(Reader& esm)
    {
        esm.getSubRecordHeader(ESM3::SUB_DATA);
        esm.get(mData);

        esm.getSubRecordHeader(ESM3::SUB_TARG);
        esm.getString(mTargetId); // NOTE: string not null terminated

        mTargetActorId = -1;
        if (esm.getNextSubRecordHeader(ESM3::SUB_TAID))
            esm.get(mTargetActorId);

        esm.getSubRecordHeader(ESM3::SUB_DURA);
        esm.get(mRemainingDuration);

        if (esm.getNextSubRecordHeader(ESM3::SUB_CELL))
            esm.getString(mCellId); // TODO: check string not null terminated
    }

    void AiEscort::save(ESM::ESMWriter& esm) const
    {
        esm.writeHNT ("DATA", mData);
        esm.writeHNString ("TARG", mTargetId);
        esm.writeHNT ("TAID", mTargetActorId);
        esm.writeHNT ("DURA", mRemainingDuration);
        if (!mCellId.empty())
            esm.writeHNString ("CELL", mCellId);
    }

    void AiFollow::load(Reader& esm)
    {
        esm.getSubRecordHeader(ESM3::SUB_DATA);
        esm.get(mData);

        esm.getSubRecordHeader(ESM3::SUB_TARG);
        esm.getString(mTargetId); // TODO: check string not null terminated

        mTargetActorId = -1;
        if (esm.getNextSubRecordHeader(ESM3::SUB_TAID))
            esm.get(mTargetActorId);

        esm.getSubRecordHeader(ESM3::SUB_DURA);
        esm.get(mRemainingDuration);

        if (esm.getNextSubRecordHeader(ESM3::SUB_CELL))
            esm.getString(mCellId); // TODO: check string not null terminated

        esm.getSubRecordHeader(ESM3::SUB_ALWY);
        esm.get(mAlwaysFollow);

        mCommanded = false;
        if (esm.getNextSubRecordHeader(ESM3::SUB_CMND))
            esm.get(mCommanded);

        mActive = false;
        if (esm.getNextSubRecordHeader(ESM3::SUB_ACTV))
            esm.get(mActive);
    }

    void AiFollow::save(ESM::ESMWriter& esm) const
    {
        esm.writeHNT ("DATA", mData);
        esm.writeHNString("TARG", mTargetId);
        esm.writeHNT ("TAID", mTargetActorId);
        esm.writeHNT ("DURA", mRemainingDuration);
        if (!mCellId.empty())
            esm.writeHNString ("CELL", mCellId);
        esm.writeHNT ("ALWY", mAlwaysFollow);
        esm.writeHNT ("CMND", mCommanded);
        if (mActive)
            esm.writeHNT("ACTV", mActive);
    }

    void AiActivate::load(Reader& esm)
    {
        esm.getSubRecordHeader(ESM3::SUB_TARG);
        esm.getString(mTargetId); // TODO: check string not null terminated
    }

    void AiActivate::save(ESM::ESMWriter& esm) const
    {
        esm.writeHNString("TARG", mTargetId);
    }

    void AiCombat::load(Reader& esm)
    {
        esm.getSubRecordHeader(ESM3::SUB_TARG);
        esm.get(mTargetActorId);
    }

    void AiCombat::save(ESM::ESMWriter& esm) const
    {
        esm.writeHNT ("TARG", mTargetActorId);
    }

    void AiPursue::load(Reader& esm)
    {
        esm.getSubRecordHeader(ESM3::SUB_TARG);
        esm.get(mTargetActorId);
    }

    void AiPursue::save(ESM::ESMWriter& esm) const
    {
        esm.writeHNT ("TARG", mTargetActorId);
    }

    AiSequence::~AiSequence()
    {
        for (std::vector<AiPackageContainer>::iterator it = mPackages.begin(); it != mPackages.end(); ++it)
            delete it->mPackage;
    }

    void AiSequence::load(Reader& esm)
    {

        while (esm.getNextSubRecordHeader(ESM3::SUB_AIPK))
        {
            int type;
            esm.get(type);

            mPackages.emplace_back();
            mPackages.back().mType = type;

            switch (type)
            {
            case Ai_Wander:
            {
                std::unique_ptr<AiWander> ptr = std::make_unique<AiWander>();
                ptr->load(esm);
                mPackages.back().mPackage = ptr.release();
                break;
            }
            case Ai_Travel:
            {
                std::unique_ptr<AiTravel> ptr = std::make_unique<AiTravel>();
                ptr->load(esm);
                mPackages.back().mPackage = ptr.release();
                break;
            }
            case Ai_Escort:
            {
                std::unique_ptr<AiEscort> ptr = std::make_unique<AiEscort>();
                ptr->load(esm);
                mPackages.back().mPackage = ptr.release();
                break;
            }
            case Ai_Follow:
            {
                std::unique_ptr<AiFollow> ptr = std::make_unique<AiFollow>();
                ptr->load(esm);
                mPackages.back().mPackage = ptr.release();
                break;
            }
            case Ai_Activate:
            {
                std::unique_ptr<AiActivate> ptr = std::make_unique<AiActivate>();
                ptr->load(esm);
                mPackages.back().mPackage = ptr.release();
                break;
            }
            case Ai_Combat:
            {
                std::unique_ptr<AiCombat> ptr = std::make_unique<AiCombat>();
                ptr->load(esm);
                mPackages.back().mPackage = ptr.release();
                break;
            }
            case Ai_Pursue:
            {
                std::unique_ptr<AiPursue> ptr = std::make_unique<AiPursue>();
                ptr->load(esm);
                mPackages.back().mPackage = ptr.release();
                break;
            }
            default:
                return;
            }
        }

        if (esm.getNextSubRecordHeader(ESM3::SUB_LAST))
            esm.get(mLastAiPackage);
    }

    void AiSequence::save(ESM::ESMWriter& esm) const
    {
        for (std::vector<AiPackageContainer>::const_iterator it = mPackages.begin(); it != mPackages.end(); ++it)
        {
            esm.writeHNT ("AIPK", it->mType);
            switch (it->mType)
            {
            case Ai_Wander:
                static_cast<const AiWander*>(it->mPackage)->save(esm);
                break;
            case Ai_Travel:
                static_cast<const AiTravel*>(it->mPackage)->save(esm);
                break;
            case Ai_Escort:
                static_cast<const AiEscort*>(it->mPackage)->save(esm);
                break;
            case Ai_Follow:
                static_cast<const AiFollow*>(it->mPackage)->save(esm);
                break;
            case Ai_Activate:
                static_cast<const AiActivate*>(it->mPackage)->save(esm);
                break;
            case Ai_Combat:
                static_cast<const AiCombat*>(it->mPackage)->save(esm);
                break;
            case Ai_Pursue:
                static_cast<const AiPursue*>(it->mPackage)->save(esm);
                break;

            default:
                break;
            }
        }

        esm.writeHNT ("LAST", mLastAiPackage);
    }
}
}
