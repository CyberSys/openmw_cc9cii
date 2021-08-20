#ifndef OPENMW_ESSIMPORT_IMPORTSCPT_H
#define OPENMW_ESSIMPORT_IMPORTSCPT_H

#include "importscri.hpp"

#include <components/esm3/scpt.hpp>
#include <components/esm/esmcommon.hpp> // NAME32

namespace ESM3
{
    class Reader;
}

namespace ESSImport
{

    struct SCHD
    {
        ESM::NAME32              mName;
        ESM3::Script::SCHDstruct mData;
    };

    // A running global script
    struct SCPT
    {
        SCHD mSCHD;

        // values of local variables
        SCRI mSCRI;

        bool mRunning;
        int mRefNum; // Targeted reference, -1: no reference

        void load(ESM3::Reader& esm);
    };

}

#endif
