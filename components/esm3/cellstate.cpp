#include "cellstate.hpp"

#include "reader.hpp"
#include "../esm/esmwriter.hpp"

void ESM3::CellState::load (Reader& esm)
{
    mWaterLevel = 0;
    if (esm.getNextSubRecordHeader(ESM3::SUB_WLVL))
        esm.get(mWaterLevel);

    mHasFogOfWar = false;
    if (esm.getNextSubRecordHeader(ESM3::SUB_HFOW))
        esm.get(mHasFogOfWar);

    mLastRespawn.mDay = 0;
    mLastRespawn.mHour = 0;
    if (esm.getNextSubRecordHeader(ESM3::SUB_RESP))
        esm.get(mLastRespawn);
}

void ESM3::CellState::save (ESM::ESMWriter& esm) const
{
    if (!mId.mPaged)
        esm.writeHNT ("WLVL", mWaterLevel);

    esm.writeHNT ("HFOW", mHasFogOfWar);

    esm.writeHNT ("RESP", mLastRespawn);
}
