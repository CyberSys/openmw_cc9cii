#include "convertcntc.hpp"

#include "convertinventory.hpp"

namespace ESSImport
{

    void convertCNTC(const CNTC &cntc, ESM3::ContainerState &state)
    {
        convertInventory(cntc.mInventory, state.mInventory);
    }

}
