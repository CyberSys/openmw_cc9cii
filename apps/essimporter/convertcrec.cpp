#include "convertcrec.hpp"

#include "convertinventory.hpp"

namespace ESSImport
{

    void convertCREC(const CREC &crec, ESM3::CreatureState &state)
    {
        convertInventory(crec.mInventory, state.mInventory);
    }

}
