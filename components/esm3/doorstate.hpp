#ifndef ESM3_DOORSTATE_H
#define ESM3_DOORSTATE_H

#include "objectstate.hpp"

namespace ESM3
{
    // format 0, saved games only

    struct DoorState final : public ObjectState
    {
        int mDoorState = 0;

        void load (Reader& esm) override;
        void save (ESM::ESMWriter& esm, bool inInventory = false) const override;

        DoorState& asDoorState() override
        {
            return *this;
        }

        const DoorState& asDoorState() const override
        {
            return *this;
        }
    };
}

#endif
