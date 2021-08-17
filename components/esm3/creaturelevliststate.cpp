#include "creaturelevliststate.hpp"

#include "reader.hpp"
#include "../esm/esmwriter.hpp"

namespace ESM3
{
    void CreatureLevListState::load(Reader& esm)
    {
        ObjectState::load(esm);

        mSpawnActorId = -1;
        if (esm.getNextSubRecordType() == ESM3::SUB_SPAW && esm.getSubRecordHeader())
            esm.get(mSpawnActorId);

        mSpawn = false;
        if (esm.getNextSubRecordType() == ESM3::SUB_RESP && esm.getSubRecordHeader())
            esm.get(mSpawn);
    }

    void CreatureLevListState::save(ESM::ESMWriter& esm, bool inInventory) const
    {
        ObjectState::save(esm, inInventory);

        if (mSpawnActorId != -1)
            esm.writeHNT ("SPAW", mSpawnActorId);

        if (mSpawn)
            esm.writeHNT ("RESP", mSpawn);
    }
}
