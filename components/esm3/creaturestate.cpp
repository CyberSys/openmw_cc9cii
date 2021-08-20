#include "creaturestate.hpp"

void ESM3::CreatureState::load (Reader& esm)
{
    ObjectState::load (esm);

    if (mHasCustomState)
    {
        mInventory.load (esm);

        mCreatureStats.load (esm);
    }
}

void ESM3::CreatureState::save (ESM::ESMWriter& esm, bool inInventory) const
{
    ObjectState::save (esm, inInventory);

    if (mHasCustomState)
    {
        mInventory.save (esm);

        mCreatureStats.save (esm);
    }
}

void ESM3::CreatureState::blank()
{
    ObjectState::blank();
    mCreatureStats.blank();
}
