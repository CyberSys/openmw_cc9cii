#include "importacdt.hpp"

//#ifdef NDEBUG
//#undef NDEBUG
//#endif

#include <cassert>

#include <components/esm3/reader.hpp>

#include <components/esm3/cellref.hpp>

namespace ESSImport
{
    // NOTE: assumes this method was called immediately after "NAME" sub-record
    //       without reading the next sub-record header
    void ActorData::load(ESM3::Reader& esm)
    {
        assert(esm.subRecordHeader().typeId == ESM3::SUB_NAME);

        mHasACDT = false;
        mHasACSC = false;
        mHasANIS = false;

        bool subDataRemaining = false;
        bool finished = false;
        bool doOnce = false;
        while (!finished && (subDataRemaining || esm.getSubRecordHeader()))
        {
            subDataRemaining = false;
            const ESM3::SubRecordHeader& subHdr = esm.subRecordHeader();
            switch (subHdr.typeId)
            {
                case ESM3::SUB_ACTN:
                {
                    /*
                    Activation flags:
                    ActivationFlag_UseEnabled  = 1
                    ActivationFlag_OnActivate  = 2
                    ActivationFlag_OnDeath     = 10h
                    ActivationFlag_OnKnockout  = 20h
                    ActivationFlag_OnMurder    = 40h
                    ActivationFlag_DoorOpening = 100h
                    ActivationFlag_DoorClosing = 200h
                    ActivationFlag_DoorJammedOpening  = 400h
                    ActivationFlag_DoorJammedClosing  = 800h
                    */
                    esm.skipSubRecordData();
                    break;
                }
                case ESM3::SUB_STPR:
                case ESM3::SUB_MNAM:
                {
                    esm.skipSubRecordData();
                    break;
                }
                case ESM3::SUB_ACDT:
                {
                    mHasACDT = true;
                    esm.get(mACDT);
                    break;
                }
                case ESM3::SUB_ACSC:
                {
                    mHasACSC = true;
                    esm.get(mACSC);
                    break;
                }
                case ESM3::SUB_ACSL:
                {
                    assert(esm.subRecordHeader().dataSize == 112 && "ACSL incorrect data size");
                    esm.skipSubRecordData();
                    break;
                }
                case ESM3::SUB_CSTN:
                case ESM3::SUB_LSTN:
                case ESM3::SUB_CSHN: // unsure at which point between LSTN and TGTN
                case ESM3::SUB_LSHN: // unsure if before or after CSTN/LSTN
                case ESM3::SUB_TGTN:
                {
                    esm.skipSubRecordData(); // "PlayerSaveGame", link to some object?
                    break;
                }
                case ESM3::SUB_AADT: // unsure at which point between TGTN and CRED
                {
                    esm.skipSubRecordData(); // occurred when a creature was in the middle of its attack, 44 bytes
                    break;
                }
                case ESM3::SUB_FGTN: // fight target?
                case ESM3::SUB_PWPC: // unsure at which point between FGTN and CHRD
                case ESM3::SUB_PWPS:
                case ESM3::SUB_ND3D:
                case ESM3::SUB_YNAM: // 4 byte, 0
                {
                    esm.skipSubRecordData();
                    break;
                }
                case ESM3::SUB_WNAM:
                {
                    esm.getZString(mSelectedSpell);
                    break;
                }
                case ESM3::SUB_XNAM:
                {
                    esm.getZString(mSelectedEnchantItem);
                    mSelectedSpell.clear(); // NOTE: seems very strange logic here (don't blame me, I only copied it)
                    break;
                }
                case ESM3::SUB_APUD:
                {
                    // used power
                    char tmp[32];
                    esm.get(tmp[0], 32);
                    std::string id(&tmp[0]);
                    (void)id;
                    // timestamp can't be used: this is the total hours passed, calculated by
                    // timestamp = 24 * (365 * year + cumulativeDays[month] + day)
                    // unfortunately cumulativeDays[month] is not clearly defined,
                    // in the (non-MCP) vanilla version the first month was missing, but MCP added it.
                    double timestamp;
                    esm.get(timestamp);
                    break;
                }
                // FIXME: not all actors have this, add flag
                case ESM3::SUB_CHRD: // npc only
                {
                    assert(subHdr.dataSize == 27 * 2 * sizeof(int) && "CHRD incorrect data size");
                    esm.get(mSkills, 27 * 2 * sizeof(int));
                    break;
                }
                case ESM3::SUB_CRED: // creature only
                {
                    assert(subHdr.dataSize == 3 * 2 * sizeof(int) && "CRED incorrect data size");
                    esm.get(mCombatStats, 3 * 2 * sizeof(int));
                    break;
                }
                case ESM3::SUB_SCRI:
                {
                    subDataRemaining = mSCRI.load(esm);
                    break;
                }
                case ESM3::SUB_ANIS:
                {
                    mHasANIS = true;
                    esm.get(mANIS);
                    break;
                }
                default:
                {
                    // Unknown sub-records are either in ESM3::CellRef (i.e. parent class) or
                    // in ESSImport::CellRef (i.e. child class).  Ensure that we check the parent
                    // class only once.
                    if (doOnce)
                    {
                        esm.cacheSubRecordHeader(); // prepare for continuing in CellRef::load()
                        finished = true;
                        break;
                    }
                    else
                        doOnce = true;

                    // Not keen to modify CellRef::loadData() so we do this hack to "unread"
                    // the sub-record header.
                    esm.cacheSubRecordHeader(); // prepare for loading ESM3::CellRef::loadData()
                    bool isDeleted = false;
                    subDataRemaining = ESM3::CellRef::loadData(esm, isDeleted);

                    break;
                }
            }
        }
    }
}
