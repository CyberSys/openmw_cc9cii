#ifndef OPENMW_ESSIMPORT_IMPORTINVENTORY_H
#define OPENMW_ESSIMPORT_IMPORTINVENTORY_H

#include <vector>
#include <string>
#include <cstdint>

#include <components/esm3/cellref.hpp>
#include <components/esm/esmcommon.hpp>

#include "importscri.hpp"

namespace ESM3
{
    class Reader;
}

namespace ESSImport
{

    struct ContItem
    {
        std::uint32_t mCount;
        ESM::NAME32 mItem;
    };

    struct Inventory
    {
        struct InventoryItem : public ESM3::CellRef
        {
            std::string mId;
            unsigned int mCount;
            int mRelativeEquipmentSlot;
            SCRI mSCRI;
        };
        std::vector<InventoryItem> mItems;

        void load(ESM3::Reader& esm);
    };

}

#endif
