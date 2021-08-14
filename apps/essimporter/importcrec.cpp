#include "importcrec.hpp"

#include <components/esm3/reader.hpp>

namespace ESSImport
{
    void CREC::load(ESM3::Reader& esm)
    {
        mIndex = 0;
        assert(esm.hdr().typeId == ESM3::REC_CREC);

        bool subHdrRead = false;
        while (subHdrRead|| esm.getSubRecordHeader())
        {
            const ESM3::SubRecordHeader& subHdr = esm.subRecordHeader();
            switch (subHdr.typeId)
            {
                case ESM3::SUB_INDX:
                {
                    esm.get(mIndex);
                    break;
                }
                case ESM3::SUB_XSCL:
                {
                    // equivalent of ESM::Creature XSCL? probably don't have to convert this,
                    // since the value can't be changed
                    float scale;
                    esm.get(scale);
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
                    subHdrRead = mInventory.load(esm);
                    break;
                }
                default:
                    esm.skipSubRecordData();
                    break;
            }
        }
    }
}
