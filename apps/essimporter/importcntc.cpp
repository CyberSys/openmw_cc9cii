#include "importcntc.hpp"

#include <cassert>

#include <components/esm3/reader.hpp>

namespace ESSImport
{

    void CNTC::load(ESM3::Reader& esm)
    {
        assert(esm.hdr().typeId == ESM3::REC_CNTC);

        mIndex = 0;
        while (esm.getSubRecordHeader())
        {
            const ESM3::SubRecordHeader& subHdr = esm.subRecordHeader();
            switch (subHdr.typeId)
            {
                case ESM3::SUB_INDX:
                {
                    esm.get(mIndex);
                    break;
                }
                case ESM3::SUB_NPCO:
                {
                    mInventory.load(esm);
                    break;
                }
                default:
                    esm.fail("CNTC: Unknown subrecord");
            }
        }
    }

}
