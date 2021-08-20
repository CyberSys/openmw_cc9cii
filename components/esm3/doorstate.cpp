#include "doorstate.hpp"

#include "reader.hpp"
#include "../esm/esmwriter.hpp"

#include <components/debug/debuglog.hpp>

namespace ESM3
{
    void DoorState::load(Reader& esm)
    {
        ObjectState::load(esm);

        mDoorState = 0;
        if (esm.getNextSubRecordHeader(ESM3::SUB_ANIM))
            esm.get(mDoorState);

        if (mDoorState < 0 || mDoorState > 2)
            Log(Debug::Warning) << "Dropping invalid door state (" << mDoorState << ") for door \"" << mRef.mRefID << "\"";
    }

    void DoorState::save(ESM::ESMWriter& esm, bool inInventory) const
    {
        ObjectState::save(esm, inInventory);

        if (mDoorState < 0 || mDoorState > 2)
        {
            Log(Debug::Warning) << "Dropping invalid door state (" << mDoorState << ") for door \"" << mRef.mRefID << "\"";
            return;
        }

        if (mDoorState != 0)
            esm.writeHNT ("ANIM", mDoorState);
    }
}
