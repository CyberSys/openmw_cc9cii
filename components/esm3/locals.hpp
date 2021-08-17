#ifndef ESM3_LOCALS_H
#define ESM3_LOCALS_H

#include <vector>
#include <string>

#include "variant.hpp"

namespace ESM
{
    class ESMWriter;
}

namespace ESM3
{
    class Reader;

    /// \brief Storage structure for local variables (only used in saved games)
    ///
    /// \note This is not a top-level record.

    struct Locals
    {
        std::vector<std::pair<std::string, Variant> > mVariables;

        bool load (Reader& esm);
        void save (ESM::ESMWriter& esm) const;
    };
}

#endif
