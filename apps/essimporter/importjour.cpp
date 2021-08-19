#include "importjour.hpp"

//#ifdef NDEBUG
//#undef NDEBUG
//#endif

#include <cassert>

#include <components/esm3/reader.hpp>

namespace ESSImport
{
    void JOUR::load(ESM3::Reader& esm)
    {
        assert(esm.hdr().typeId == ESM3::REC_JOUR);

        esm.getSubRecordHeader();
        esm.getZString(mText);
        esm.skipRecordData(); // may have trailing nulls; TODO: test
    }
}
