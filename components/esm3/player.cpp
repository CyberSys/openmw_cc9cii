#include "player.hpp"

#include "reader.hpp"
#include "../esm/esmwriter.hpp"

void ESM3::Player::load (Reader& esm)
{
    esm.getSubRecordHeader();
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
                if (subHdr.dataSize != sizeof(mLastKnownExteriorPosition) || subHdr.dataSize != 12)
                    esm.fail("LKEP incorrect data size");
                esm.get(mLastKnownExteriorPosition);
                break;
            }
            case ESM3::SUB_MARK:
            {
                if (subHdr.dataSize != sizeof(mMarkedPosition) || subHdr.dataSize != 24)
                    esm.fail("MARK incorrect data size");
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
                esm.getString(mBirthsign); // NOTE: string not null terminated
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
                esm.cacheSubRecordHeader(); // :sadcat:
                checkPrevItems = true;
                break;
        }
    }

    while (checkPrevItems)
    {
        std::string boundItemId;
        std::string prevItemId;

        if (esm.getNextSubRecordHeader(ESM3::SUB_BOUN))
            esm.getString(boundItemId); // TODO: check string not null terminated

        if (esm.getNextSubRecordHeader(ESM3::SUB_PREV))
             esm.getString(prevItemId); // TODO: check string not null terminated

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
