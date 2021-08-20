#include "importinventory.hpp"

#include <cassert>
#include <stdexcept>
#include <cmath>

#include <components/esm3/reader.hpp>

namespace ESSImport
{
    // assumes that the NPCO sub-record header was just read
    void Inventory::load(ESM3::Reader& esm)
    {
        assert(esm.subRecordHeader().typeId == ESM3::SUB_NPCO);
        bool skipOnce = true;

        int baseIndex = -1;
        int currentIndex = -1; // -1 means uninitialised
        bool doOnce = false;
        while (skipOnce || esm.getSubRecordHeader())
        {
            skipOnce = false;

            const ESM3::SubRecordHeader& subHdr = esm.subRecordHeader();
            switch (subHdr.typeId)
            {
                case ESM3::SUB_NPCO:
                {
                    assert(subHdr.dataSize == 4 + 32);

                    ContItem contItem;
                    esm.get(contItem);

                    InventoryItem item;
                    item.mId = contItem.mItem.toString();
                    // NOTE: mCount can be negative (possibly restockable?)
                    item.mCount = contItem.mCount;
                    item.mRelativeEquipmentSlot = -1;
                    item.mLockLevel = 0;
                    item.mRefNum.unset();
                    item.mChargeInt = -1;

                    // At this point we don't know if the item is stackable
                    mItems.push_back(item);

                    baseIndex = -1;
                    currentIndex = -1;
                    doOnce = true; // load any ESM3::CellRef sub-records

                    break;
                }
                case ESM3::SUB_XIDX: // separate stack; optional
                {
                    if (baseIndex < 0) // do this only once for each NPCO sub-record
                    {
                        // The last item is not stackable
                        InventoryItem& lastItem = mItems.back();
                        unsigned int numItems = std::abs(lastItem.mCount);

                        // Remember the base index
                        baseIndex = mItems.size() - 1;

                        lastItem.mCount = lastItem.mCount / std::abs(lastItem.mCount); // preserve sign
                        for (unsigned int i = 0; i < numItems - 1; ++i)
                        {
                            InventoryItem newItem = lastItem; //make a copy
                            mItems.push_back(newItem);
                        }
                    }

                    doOnce = true;

                    unsigned int idx;
                    esm.get(idx);
                    currentIndex = baseIndex + idx;

                    break;
                }
                case ESM3::SUB_SCRI: // item script; optional
                {
                    if (currentIndex > 0) // separate stack
                        mItems[currentIndex].mSCRI.load(esm);
                    else
                        mItems.back().mSCRI.load(esm);
                    break;
                }
                case ESM3::SUB_XHLT: // optional
                {
                    if (currentIndex > 0) // separate stack
                        esm.get(mItems[currentIndex].mChargeInt);
                    else
                        esm.get(mItems.back().mChargeInt);
                    break;
                }
                case ESM3::SUB_WIDX: // Equipped items
                {
                    // note: same item can be equipped 2 items (e.g. 2 rings)
                    // and will be *stacked* in the NPCO list, unlike openmw!
                    // this is currently not handled properly.

                    unsigned int itemIndex; // index of the item in the NPCO list
                    esm.get(itemIndex);

                    if (itemIndex >= (unsigned int)mItems.size())
                        esm.fail("equipment item index out of range");

                    // appears to be a relative index for only the *possible* slots this item
                    // can be equipped in, i.e. 0 most of the time
                    int slotIndex;
                    esm.get(slotIndex);
                    mItems[itemIndex].mRelativeEquipmentSlot = slotIndex;

                    break;
                }
                default:
                {
                    if (!doOnce)// && currentIndex > 0)
                    {
                        esm.cacheSubRecordHeader();
                        return;
                    }
                    else
                        doOnce = false;

                    // Not keen to modify CellRef::loadData() so we do this hack to "unread"
                    // the sub-record header.
                    esm.cacheSubRecordHeader(); // prepare for loading ESM3::CellRef::loadData()
                    bool isDeleted = false;
                    // for XSOL and XCHG seen so far, but probably others too
                    if (currentIndex > 0)
                        mItems[currentIndex].ESM3::CellRef::loadData(esm, isDeleted);
                    else
                        mItems.back().ESM3::CellRef::loadData(esm, isDeleted);

                    break;
                }
            }
        }
    }
}
