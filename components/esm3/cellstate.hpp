#ifndef ESM3_CELLSTATE_H
#define ESM3_CELLSTATE_H

#include "cellid.hpp"

#include "../esm/defs.hpp"

namespace ESM
{
    class ESMWriter;
}

namespace ESM3
{
    class Reader;

    // format 0, saved games only

    /// \note Does not include references
    struct CellState
    {
        CellId mId;

        float mWaterLevel;

        int mHasFogOfWar; // Do we have fog of war state (0 or 1)? (see fogstate.hpp)

        ESM::TimeStamp mLastRespawn;

        void load (Reader& esm);
        void save (ESM::ESMWriter& esm) const;
    };
}

#endif
