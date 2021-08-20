#ifndef ESM3_CREATURESTATE_H
#define ESM3_CREATURESTATE_H

#include "objectstate.hpp"
#include "inventorystate.hpp"
#include "creaturestats.hpp"

namespace ESM3
{
    // format 0, saved games only

    struct CreatureState final : public ObjectState
    {
        InventoryState mInventory;
        CreatureStats mCreatureStats;

        /// Initialize to default state
        void blank() override;

        void load (Reader& esm) override;
        void save (ESM::ESMWriter& esm, bool inInventory = false) const override;

        CreatureState& asCreatureState() override
        {
            return *this;
        }

        const CreatureState& asCreatureState() const override
        {
            return *this;
        }
    };
}

#endif
