#include "importcellref.hpp"

//#ifdef NDEBUG
//#undef NDEBUG
//#endif

#include <cassert>
//#include <iostream>

#include <components/esm3/reader.hpp>

namespace ESSImport
{
    // called from ConvertCell::read()
    void CellRef::load(ESM3::Reader& esm)
    {
        blank();

        // (FRMR subrecord name is already read by the loop in ConvertCell)
        assert(esm.subRecordHeader().typeId == ESM3::SUB_FRMR);
        esm.get(mRefNum.mIndex); // FRMR

        // this is required since openmw supports more than 255 content files
        int pluginIndex = (mRefNum.mIndex & 0xff000000) >> 24;
        mRefNum.mContentFile = pluginIndex-1;
        mRefNum.mIndex &= 0x00ffffff;

        mEnabled = true;
        mDeleted = 0;

        bool finished = false;
        while (!finished && esm.getSubRecordHeader())
        {
            const ESM3::SubRecordHeader& subHdr = esm.subRecordHeader();
            switch (subHdr.typeId)
            {
                case ESM3::SUB_NAME:
                {
                    esm.getZString(mIndexedRefId);

                    ActorData::load(esm); // TODO: does this always happen here?
                    break;
                }
                case ESM3::SUB_LVCR:
                {
                    // occurs on levelled creature spawner references
                    // probably some identifier for the creature that has been spawned?
                    unsigned char lvcr;
                    esm.get(lvcr);
                    //std::cout << "LVCR: " << (int)lvcr << std::endl;
                    break;
                }
                case ESM3::SUB_ZNAM:
                {
                    esm.get(mEnabled);
                    break;
                }
                case ESM3::SUB_DATA:
                {
                    assert(subHdr.dataSize == 24 && "CellRef incorrect DATA size");
                    // DATA should occur for all references, except levelled creature spawners
                    // I've seen DATA *twice* on a creature record, and with the exact same content too! weird
                    // alarmvoi0000.ess
                    esm.get(mPos);
                    break;
                }
                case ESM3::SUB_DELE:
                {
                    unsigned int deleted;
                    esm.get(deleted);
                    mDeleted = ((deleted >> 24) & 0x2) != 0; // the other 3 bytes seem to be uninitialized garbage
                    break;
                }
                case ESM3::SUB_MVRF:
                {
                    esm.skipSubRecordData(); // skip MVRF
                    esm.getSubRecordHeader();
                    esm.skipSubRecordData(); // skip CNDT
                    break;
                }
                default:
                    esm.cacheSubRecordHeader(); // prepare for continuing in ConvertCell::read()
                    finished = true;
                    break;
            }
        }
    }
}
