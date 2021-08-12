#include "importklst.hpp"

#ifdef NDEBUG
#undef NDEBUG
#endif

#include <cassert>

#include <components/esm3/reader.hpp>

namespace ESSImport
{
    void KLST::load(ESM3::Reader& esm)
    {
        assert(esm.hdr().typeId == ESM3::REC_KLST);
        mWerewolfKills = 0;

        while (esm.getSubRecordHeader())
        {
            const ESM3::SubRecordHeader& subHdr = esm.subRecordHeader();
            switch (subHdr.typeId)
            {
                case ESM3::SUB_KNAM:
                {
                    std::string refId;
                    esm.getZString(refId);

                    int count;
                    esm.getSubRecordHeader(); // assume CNAM always follows KNAM
                    assert(esm.subRecordHeader().typeId == ESM3::SUB_CNAM);
                    esm.get(count);

                    mKillCounter[refId] = count;
                    break;
                }
                case ESM3::SUB_INTV:
                {
                    esm.get(mWerewolfKills);
                    break;
                }
                default:
                    esm.skipSubRecordData();
                    break;
            }
        }
    }
}
