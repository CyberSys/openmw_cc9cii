#ifndef ESM3_CUSTOMMARKERSTATE_H
#define ESM3_CUSTOMMARKERSTATE_H

#include "cellid.hpp"

namespace ESM
{
    class ESMWriter;
}

namespace ESM3
{
    class Reader;

    // format 0, saved games only
    struct CustomMarker
    {
        float mWorldX;
        float mWorldY;

        ESM3::CellId mCell;

        std::string mNote;

        bool operator == (const CustomMarker& other) const
        {
            return mNote == other.mNote && mCell == other.mCell && mWorldX == other.mWorldX && mWorldY == other.mWorldY;
        }

        //void load (Reader& reader);
        void save (ESM::ESMWriter& writer) const;
    };
}

#endif
