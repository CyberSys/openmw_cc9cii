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
        bool subDataRemaining = false;
        while (subDataRemaining || esm.getSubRecordHeader())
        {
            subDataRemaining = false;
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
                    // NOTE: it is assumed that all inventory related sub-records are grouped
                    //       so we off-load the loading here (i.e. XIDX, SCRI, XHLT occur for
                    //       that inventory item, and WIDX for equipped items)
                    subDataRemaining = mInventory.load(esm);
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
                default:
                    esm.fail("Unknown subrecord");
            }
        }
    }
}
