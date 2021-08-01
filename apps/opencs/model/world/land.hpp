#ifndef CSM_WORLD_LAND_H
#define CSM_WORLD_LAND_H

#include <string>

#include <components/esm3/land.hpp>

namespace CSMWorld
{
    /// \brief Wrapper for Land record. Encodes X and Y cell index in the ID.
    ///
    /// \todo Add worldspace support to the Land record.
    struct Land : public ESM3::Land
    {
        /// Loads the metadata and ID
        void load (ESM::Reader& esm, bool &isDeleted);

        static std::string createUniqueRecordId(int x, int y);
        static void parseUniqueRecordId(const std::string& id, int& x, int& y);
    };
}

#endif
