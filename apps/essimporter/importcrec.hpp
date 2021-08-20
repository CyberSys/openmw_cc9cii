#ifndef OPENMW_ESSIMPORT_CREC_H
#define OPENMW_ESSIMPORT_CREC_H

#include "importinventory.hpp"
#include <components/esm3/aipackage.hpp>

namespace ESM3
{
    class Reader;
}

namespace ESSImport
{

    /// Creature changes
    struct CREC
    {
        int mIndex;

        Inventory mInventory;
        ESM3::AIPackageList mAiPackage;

        void load(ESM3::Reader& esm);
    };

}

#endif
