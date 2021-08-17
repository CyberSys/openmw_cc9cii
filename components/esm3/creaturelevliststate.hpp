#ifndef ESM3_CREATURELEVLISTSTATE_H
#define ESM3_CREATURELEVLISTSTATE_H

#include "objectstate.hpp"

namespace ESM3
{
    // format 0, saved games only

    struct CreatureLevListState final : public ObjectState
    {
        int mSpawnActorId;
        bool mSpawn;

        void load (Reader& esm) override;
        void save (ESM::ESMWriter& esm, bool inInventory = false) const override;

        CreatureLevListState& asCreatureLevListState() override
        {
            return *this;
        }

        const CreatureLevListState& asCreatureLevListState() const override
        {
            return *this;
        }
    };
}

#endif
