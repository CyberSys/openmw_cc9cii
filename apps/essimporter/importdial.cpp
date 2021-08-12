#include "importdial.hpp"

#ifdef NDEBUG
#undef NDEBUG
#endif

#include <cassert>

#include <components/esm3/reader.hpp>

namespace ESSImport
{
    void DIAL::load(ESM3::Reader &esm)
    {
        assert(esm.hdr().typeId == ESM3::REC_DIAL);

        while (esm.getSubRecordHeader())
        {
            const ESM3::SubRecordHeader& subHdr = esm.subRecordHeader();
            switch (subHdr.typeId)
            {
                case ESM3::SUB_DATA:
                {
                    // See ESM::Dialogue::Type enum, not sure why we would need this here though
                    int type = 0;
                    esm.get(type);
                    break;
                }
                case ESM3::SUB_DELE:
                {
                    // Deleted dialogue in a savefile. No clue what this means...
                    int deleted = 0;
                    esm.get(deleted);
                    break;
                }
                case ESM3::SUB_XIDX:
                {
                    mIndex = 0;
                    // *should* always occur except when the dialogue is deleted, but leaving it optional just in case...
                    esm.get(mIndex);
                    break;
                }
                default:
                    esm.skipSubRecordData();
                    break;
            }
        }
    }
}
