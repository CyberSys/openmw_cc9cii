#ifndef ESM3_PLAYER_H
#define ESM3_PLAYER_H

#include <string>

#include "npcstate.hpp"
#include "cellid.hpp"
#include "../esm/defs.hpp"

#include "skil.hpp"
#include "../esm/attr.hpp"

namespace ESM
{
    class ESMWriter;
}

namespace ESM3
{
    class Reader;

    // format 0, saved games only

    struct Player
    {
        NpcState mObject;
        CellId mCellId;
        float mLastKnownExteriorPosition[3];
        unsigned char mHasMark;
        ESM::Position mMarkedPosition;
        CellId mMarkedCell;
        std::string mBirthsign;

        int mCurrentCrimeId;
        int mPaidCrimeId;

        StatState<float> mSaveAttributes[ESM::Attribute::Length];
        StatState<float> mSaveSkills[ESM3::Skill::Length];

        typedef std::map<std::string, std::string> PreviousItems; // previous equipped items, needed for bound spells
        PreviousItems mPreviousItems;

        void load (Reader& esm);
        void save (ESM::ESMWriter& esm) const;
    };
}

#endif
