#ifndef CSM_WOLRD_PATHGRID_H
#define CSM_WOLRD_PATHGRID_H

#include <vector>
#include <string>

#include <components/esm3/pgrd.hpp>

namespace CSMWorld
{
    struct Cell;
    template<typename T, typename AT>
    class IdCollection;

    /// \brief Wrapper for Pathgrid record
    ///
    /// \attention The mData.mX and mData.mY fields of the ESM::Pathgrid struct are not used.
    /// Exterior cell coordinates are encoded in the pathgrid ID.
    struct Pathgrid : public ESM3::Pathgrid
    {
        std::string mId;

        void load (ESM::Reader& reader, bool& isDeleted, const IdCollection<Cell>& cells);
        void load (ESM::Reader& reader, bool& isDeleted);
    };
}

#endif
