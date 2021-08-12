#include "importinventory.hpp"

#ifdef NDEBUG
#undef NDEBUG
#endif

#include <cassert>
#include <stdexcept>

#include <components/esm3/reader.hpp>

namespace ESSImport
{
    void Inventory::load(ESM3::Reader& esm)
    {
        //assert(esm.subRecordHeader().typeId == ESM3::SUB_NPCO); // only if coming from NPCC::load()?
        {
            assert(esm.subRecordHeader().dataSize == 4 + 32);
            ContItem contItem;
            esm.get(contItem);

            InventoryItem item;
            item.mId = contItem.mItem.toString();
            item.mCount = contItem.mCount;
            item.mRelativeEquipmentSlot = -1;
            item.mLockLevel = 0;
            item.mRefNum.unset();
            item.mChargeInt = -1;
#if 0
            bool separateStacks = false;
            for (unsigned int i = 0; i < item.mCount ; ++i)
            {
                bool newStack = esm.isNextSub("XIDX");
                if (newStack)
                {
                    unsigned int idx;
                    esm.get(idx);
                    separateStacks = true;
                    item.mCount = 1;
                }

                item.mSCRI.load(esm);

                // for XSOL and XCHG seen so far, but probably others too
                bool isDeleted = false;
                item.ESM3::CellRef::loadData(esm, isDeleted);

                int charge=-1;
                esm.getHNOT(charge, "XHLT");
                item.mChargeInt = charge;

                if (newStack)
                    mItems.push_back(item);
            }

            if (!separateStacks)
#endif
                mItems.push_back(item);
        }
#if 0
        // equipped items
        while (esm.isNextSub("WIDX"))
        {
            // note: same item can be equipped 2 items (e.g. 2 rings)
            // and will be *stacked* in the NPCO list, unlike openmw!
            // this is currently not handled properly.

            esm.getSubHeader();
            int itemIndex; // index of the item in the NPCO list
            esm.getT(itemIndex);

            if (itemIndex < 0 || itemIndex >= int(mItems.size()))
                esm.fail("equipment item index out of range");

            // appears to be a relative index for only the *possible* slots this item can be equipped in,
            // i.e. 0 most of the time
            int slotIndex;
            esm.getT(slotIndex);

            mItems[itemIndex].mRelativeEquipmentSlot = slotIndex;
        }
#endif
    }

}
