#include "importques.hpp"

#ifdef NDEBUG
#undef NDEBUG
#endif

#include <cassert>

#include <components/esm3/reader.hpp>

namespace ESSImport
{

    void QUES::load(ESM3::Reader& esm)
    {
        assert(esm.hdr().typeId == ESM3::REC_QUES);

        while (esm.getSubRecordHeader())
        {
            const ESM3::SubRecordHeader& subHdr = esm.subRecordHeader();
            switch (subHdr.typeId)
            {
                case ESM3::SUB_DATA:
                {
                    std::string info;
                    esm.getZString(info);
                    mInfo.push_back(info);
                    break;
                }
                default:
                    esm.skipSubRecordData();
                    break;
            }
        }
    }

}
