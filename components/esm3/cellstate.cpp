#include "cellstate.hpp"

#include "reader.hpp"
#include "../esm/esmwriter.hpp"

void ESM3::CellState::load (Reader& esm)
{
    mWaterLevel = 0;
    esm.getHNOT (mWaterLevel, "WLVL");

    mHasFogOfWar = false;
    esm.getHNOT (mHasFogOfWar, "HFOW");

    mLastRespawn.mDay = 0;
    mLastRespawn.mHour = 0;
    esm.getHNOT (mLastRespawn, "RESP");
}

void ESM3::CellState::save (ESM::ESMWriter& esm) const
{
    if (!mId.mPaged)
        esm.writeHNT ("WLVL", mWaterLevel);

    esm.writeHNT ("HFOW", mHasFogOfWar);

    esm.writeHNT ("RESP", mLastRespawn);
}
