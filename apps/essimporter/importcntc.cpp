#include "importcntc.hpp"

#ifdef NDEBUG
#undef NDEBUG
#endif

#include <cassert>

#include <components/esm3/reader.hpp>

namespace ESSImport
{

    void CNTC::load(ESM3::Reader& esm)
    {
        mIndex = 0;
        assert(esm.hdr().typeId == ESM3::REC_CNTC);

        esm.getSubRecordHeader();
        assert(esm.subRecordHeader().typeId == ESM3::SUB_INDX);
        esm.get(mIndex);

        esm.getSubRecordHeader();
        if (esm.subRecordHeader().typeId == ESM3::SUB_NPCO)
            mInventory.load(esm);

        // FIXME: XIDX, SCRI, SCHG, XHLT
        esm.skipRecordData();
    }

}
