#include "npcstate.hpp"

void ESM3::NpcState::load (Reader& esm)
{
    ObjectState::load (esm);

    if (mHasCustomState)
    {
        mInventory.load (esm);

        mNpcStats.load (esm);

        mCreatureStats.load (esm);
    }
}

void ESM3::NpcState::save (ESM::ESMWriter& esm, bool inInventory) const
{
    ObjectState::save (esm, inInventory);

    if (mHasCustomState)
    {
        mInventory.save (esm);

        mNpcStats.save (esm);

        mCreatureStats.save (esm);
    }
}

void ESM3::NpcState::blank()
{
    ObjectState::blank();
    mNpcStats.blank();
    mCreatureStats.blank();
    mHasCustomState = true;
}
