#include "importproj.h"

//#ifdef NDEBUG
//#undef NDEBUG
//#endif

#include <cassert>

#include <components/esm3/reader.hpp>

namespace ESSImport
{
    void ESSImport::PROJ::load(ESM3::Reader& esm)
    {
        assert(esm.hdr().typeId == ESM3::REC_PROJ);

        while (esm.getSubRecordHeader())
        {
            const ESM3::SubRecordHeader& subHdr = esm.subRecordHeader();
            switch (subHdr.typeId)
            {
                case ESM3::SUB_PNAM:
                {
                    PNAM pnam;
                    esm.get(pnam);
                    mProjectiles.push_back(pnam);
                    break;
                }
                default:
                    esm.skipSubRecordData();
                    break;
            }
        }
    }
}
