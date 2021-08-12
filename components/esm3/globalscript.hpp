#ifndef ESM3_GLOBALSCRIPT_H
#define ESM3_GLOBALSCRIPT_H

#include "locals.hpp"
#include "cellref.hpp"

namespace ESM
{
    class ESMWriter;
}

namespace ESM3
{
    class Reader;

    /// \brief Storage structure for global script state (only used in saved games)

    struct GlobalScript
    {
        std::string mId; /// \note must be lowercase
        Locals mLocals;
        int mRunning;
        std::string mTargetId; // for targeted scripts
        RefNum mTargetRef;

        //void load (Reader& esm);
        void save (ESM::ESMWriter& esm) const;
    };
}

#endif
