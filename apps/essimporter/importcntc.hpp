#ifndef OPENMW_ESSIMPORT_IMPORTCNTC_H
#define OPENMW_ESSIMPORT_IMPORTCNTC_H

#include "importinventory.hpp"

namespace ESM3
{
    class Reader;
}

namespace ESSImport
{

    /// Changed container contents
    struct CNTC
    {
        int mIndex;

        Inventory mInventory;

        void load(ESM3::Reader& esm);
    };

}
#endif
