#ifndef OPENMW_ESSIMPORT_NPCC_H
#define OPENMW_ESSIMPORT_NPCC_H

#include <components/esm3/cont.hpp>

#include <components/esm3/aipackage.hpp>

#include "importinventory.hpp"

namespace ESM3
{
    class Reader;
}

namespace ESSImport
{

    struct NPCC
    {
        struct NPDT
        {
            unsigned char mDisposition;
            unsigned char unknown;
            unsigned char mReputation;
            unsigned char unknown2;
            int mIndex;
        } mNPDT;

        Inventory mInventory;
        ESM3::AIPackageList mAiPackage;

        void load(ESM3::Reader& esm);
    };

}

#endif
