#include "containerstate.hpp"

void ESM3::ContainerState::load (Reader& esm)
{
    ObjectState::load (esm);

    mInventory.load (esm);
}

void ESM3::ContainerState::save (ESM::ESMWriter& esm, bool inInventory) const
{
    ObjectState::save (esm, inInventory);

    mInventory.save (esm);
}
