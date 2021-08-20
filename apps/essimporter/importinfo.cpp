#include "importinfo.hpp"

#include <cassert>

#include <components/esm3/reader.hpp>

namespace ESSImport
{
    void INFO::load(ESM3::Reader& esm)
    {
        assert(esm.hdr().typeId == ESM3::REC_INFO);

        if (esm.getNextSubRecordHeader(ESM3::SUB_INAM))
            esm.getZString(mInfo);

        if (esm.getNextSubRecordHeader(ESM3::SUB_ACDT))
            esm.getZString(mActorRefId);
    }
}
