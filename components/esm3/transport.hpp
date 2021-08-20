#ifndef ESM3_TRANSPORT_H
#define ESM3_TRANSPORT_H

#include <string>
#include <vector>

#include "../esm/defs.hpp" // Position

namespace ESM
{
    class ESMWriter;
}

namespace ESM3
{
    class Reader;

    /// List of travel service destination. Shared by CREA and NPC_ records.
    struct Transport
    {

        struct Dest
        {
            ESM::Position    mPos;
            std::string mCellName;
        };

        std::vector<Dest> mList;

        /// Load one destination, assumes the subrecord name was already read
        void add(Reader& esm);

        void save(ESM::ESMWriter& esm) const;

    };

}

#endif
