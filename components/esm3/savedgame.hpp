#ifndef ESM3_SAVEDGAME_H
#define ESM3_SAVEDGAME_H

#include <vector>
#include <string>

#include "../esm/defs.hpp"

namespace ESM
{
    class ESMWriter;
}

namespace ESM3
{
    class Reader;

    // format 0, saved games only

    struct SavedGame
    {
        static unsigned int sRecordId;

        static int sCurrentFormat;

        std::vector<std::string> mContentFiles;
        std::string mPlayerName;
        int mPlayerLevel;

        // ID of class
        std::string mPlayerClassId;
        // Name of the class. When using a custom class, the ID is not really meaningful prior
        // to loading the savegame, so the name is stored separately.
        std::string mPlayerClassName;

        std::string mPlayerCell;
        ESM::EpochTimeStamp mInGameTime;
        double mTimePlayed;
        std::string mDescription;
        std::vector<char> mScreenshot; // raw jpg-encoded data

        void load (Reader& esm);
        void save (ESM::ESMWriter& esm) const;
    };
}

#endif
