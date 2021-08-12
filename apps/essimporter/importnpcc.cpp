#include "importnpcc.hpp"

#ifdef NDEBUG
#undef NDEBUG
#endif

#include <cassert>

#include <components/esm3/reader.hpp>

namespace ESSImport
{

    void NPCC::load(ESM3::Reader& esm)
    {
        bool subHdrRead = false;
        while (subHdrRead || esm.getSubRecordHeader())
        {
            subHdrRead = false;
            const ESM3::SubRecordHeader& subHdr = esm.subRecordHeader();
            switch (subHdr.typeId)
            {
                case ESM3::SUB_NPDT:
                {
                    assert(subHdr.dataSize == sizeof(mNPDT) && "NPDT data size mismatch");
                    esm.get(mNPDT);
                    break;
                }
                case ESM3::SUB_AI_A: // Activate
                case ESM3::SUB_AI_E: // Escort
                case ESM3::SUB_AI_F: // Follow
                case ESM3::SUB_AI_T: // Travel
                case ESM3::SUB_AI_W: // Wander
                case ESM3::SUB_CNDT: // Cell (does this ever occur in a savefile?)
                {
                    mAiPackage.add(esm);
                    break;
                }
                case ESM3::SUB_NPCO:
                {
                    mInventory.load(esm);
                    break;
                }
                case ESM3::SUB_XIDX: // separate stack; optional, always follows SUB_NPCO
                {
                    unsigned int idx;
                    esm.get(idx);
                    mInventory.mItems.back().mCount = 1;
                    break;
                }
                case ESM3::SUB_SCRI: // item script; optional, always comes after SUB_XIDX
                {
                    subHdrRead = mInventory.mItems.back().mSCRI.load(esm);
                    break;
                }
//#if 0
                case ESM3::SUB_XCHG: // enchantment charge; optional, always comes after SUB_XIDX
                {
                    esm.get(mInventory.mItems.back().mEnchantmentCharge);
                    break;
                }
                case ESM3::SUB_XSOL: // soul; optional, always comes after SUB_XIDX
                {
                    esm.get(mInventory.mItems.back().mSoul);
                    break;
                }
//#endif
                case ESM3::SUB_XHLT: // optional, always comes after SUB_XIDX
                {
                    esm.get(mInventory.mItems.back().mChargeInt);
                    break;
                }
                case ESM3::SUB_WIDX: // Equipped items
                {
                    // note: same item can be equipped 2 items (e.g. 2 rings)
                    // and will be *stacked* in the NPCO list, unlike openmw!
                    // this is currently not handled properly.

                    unsigned int itemIndex; // index of the item in the NPCO list
                    esm.get(itemIndex);

                    if (itemIndex >= (unsigned int)mInventory.mItems.size())
                        esm.fail("equipment item index out of range");

                    // appears to be a relative index for only the *possible* slots this item can be equipped in,
                    // i.e. 0 most of the time
                    int slotIndex;
                    esm.get(slotIndex);
                    mInventory.mItems[itemIndex].mRelativeEquipmentSlot = slotIndex;

                    break;
                }
                case ESM3::SUB_MODL: // (do these below ever occur in a savefile?)
                case ESM3::SUB_FNAM: // Name
                case ESM3::SUB_RNAM: // Race
                case ESM3::SUB_CNAM: // Class
                case ESM3::SUB_ANAM: // Faction
                case ESM3::SUB_BNAM: // Head
                case ESM3::SUB_KNAM: // Hair
              //case ESM3::SUB_SCRI: // Script (FIXME: is this the same as the one for the inventory item?
                case ESM3::SUB_FLAG: // ignored in a savefile
                case ESM3::SUB_NPCS: // Spells
                case ESM3::SUB_AIDT: // AI data
                case ESM3::SUB_DODT: // Transport
                case ESM3::SUB_DNAM: // Transport
              //case ESM3::SUB_DELE: // (FIXME: not sure about this one)
                {
                    esm.skipSubRecordData();
                    break;
                }
                default: // XCHG, XSOL
                    esm.fail("Unknown subrecord");
                    // FIXME: can't call CellRef::loadData() because it expects the sub-record header
                    //        has not been read
                    //bool isDeleted = false;
                    //subRead = mInventory.mItems.back().ESM3::CellRef::loadData(esm, isDeleted);
                    //break;
            }
        }
    }
}
