#include "player.hpp"

#include "reader.hpp"
#include "../esm/esmwriter.hpp"

void ESM3::Player::load (Reader& esm)
{
    mObject.mRef.loadId(esm, true);
    mObject.load (esm);

    mCellId.load (esm);

    mHasMark = false;
    bool checkPrevItems = false;

    while (!checkPrevItems && esm.getSubRecordHeader())
    {
        const ESM3::SubRecordHeader& subHdr = esm.subRecordHeader();
        switch (subHdr.typeId)
        {
            case ESM3::SUB_LKEP:
            {
                assert(subHdr.dataSize == 12 && "LKEP incorrect data size");
                esm.get(mLastKnownExteriorPosition);
                break;
            }
            case ESM3::SUB_MARK:
            {
                assert(subHdr.dataSize == 24 && "MARK incorrect data size");
                esm.get(mMarkedPosition);
                mMarkedCell.load(esm);
                break;
            }
            case ESM3::SUB_AMOV:// Automove, no longer used.
            {
                esm.skipSubRecordData();
                break;
            }
            case ESM3::SUB_SIGN:
            {
                esm.getZString(mBirthsign);
                break;
            }
            case ESM3::SUB_CURD:
            {
                esm.get(mCurrentCrimeId);
                break;
            }
            case ESM3::SUB_PAYD:
            {
                esm.get(mPaidCrimeId);
                break;
            }
            default:
                checkPrevItems = true;
                break;
        }
    }

    while (checkPrevItems)
    {
        std::string boundItemId;
        std::string prevItemId;

        esm.getSubRecordHeader();
        if (esm.subRecordHeader().typeId == ESM3::SUB_BOUN)
        {
            esm.getZString(boundItemId);
            esm.subRecordHeader();
        }

        if (esm.subRecordHeader().typeId == ESM3::SUB_PREV)
            esm.getZString(prevItemId);

        if (!boundItemId.empty())
            mPreviousItems[boundItemId] = prevItemId;
        else
            checkPrevItems = false;
    }

    bool intFallback = esm.getFormat() < 11;
    if (esm.hasMoreSubs())
    {
        for (int i = 0; i < ESM::Attribute::Length; ++i)
            mSaveAttributes[i].load(esm, intFallback);
        for (int i = 0; i < ESM3::Skill::Length; ++i)
            mSaveSkills[i].load(esm, intFallback);
    }
}

void ESM3::Player::save (ESM::ESMWriter& esm) const
{
    mObject.save (esm);

    mCellId.save (esm);

    esm.writeHNT ("LKEP", mLastKnownExteriorPosition);

    if (mHasMark)
    {
        esm.writeHNT ("MARK", mMarkedPosition, 24);
        mMarkedCell.save (esm);
    }

    esm.writeHNString ("SIGN", mBirthsign);

    esm.writeHNT ("CURD", mCurrentCrimeId);
    esm.writeHNT ("PAYD", mPaidCrimeId);

    for (PreviousItems::const_iterator it=mPreviousItems.begin(); it != mPreviousItems.end(); ++it)
    {
        esm.writeHNString ("BOUN", it->first);
        esm.writeHNString ("PREV", it->second);
    }

    for (int i = 0; i < ESM::Attribute::Length; ++i)
        mSaveAttributes[i].save(esm);
    for (int i = 0; i < ESM3::Skill::Length; ++i)
        mSaveSkills[i].save(esm);
}
