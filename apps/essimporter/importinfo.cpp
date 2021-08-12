#include "importinfo.hpp"

#ifdef NDEBUG
#undef NDEBUG
#endif

#include <cassert>

#include <components/esm3/reader.hpp>

namespace ESSImport
{
    void INFO::load(ESM3::Reader& esm)
    {
        assert(esm.hdr().typeId == ESM3::REC_INFO);
        esm.getSubRecordHeader();

        if (esm.subRecordHeader().typeId == ESM3::SUB_INAM)
        {
            esm.getZString(mInfo);
            esm.getSubRecordHeader();
        }

        if (esm.subRecordHeader().typeId == ESM3::SUB_ACDT)
            esm.getZString(mActorRefId);
    }
}
