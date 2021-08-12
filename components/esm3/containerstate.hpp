#ifndef ESM3_CONTAINERSTATE_H
#define ESM3_CONTAINERSTATE_H

#include "objectstate.hpp"
#include "inventorystate.hpp"

namespace ESM3
{
    // format 0, saved games only

    struct ContainerState final : public ObjectState
    {
        InventoryState mInventory;

        void load (Reader &esm) override;
        void save (ESM::ESMWriter &esm, bool inInventory = false) const override;

        ContainerState& asContainerState() override
        {
            return *this;
        }
        const ContainerState& asContainerState() const override
        {
            return *this;
        }
    };
}

#endif
