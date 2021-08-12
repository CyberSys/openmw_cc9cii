#ifndef OPENMW_ESSIMPORT_IMPORTSCRI_H
#define OPENMW_ESSIMPORT_IMPORTSCRI_H

#include <string>
#include <vector>

namespace ESM3
{
    class Reader;
}

namespace ESSImport
{

    /// Local variable assignments for a running script
    struct SCRI
    {
        std::string mScript;

        std::vector<short> mShorts;
        std::vector<int> mLongs;
        std::vector<float> mFloats;

        // returns true if a sub-record header was read (i.e. the caller of
        // this methods needs to deal with reading/skipping the data)
        bool load(ESM3::Reader& esm);
    };

}

#endif
