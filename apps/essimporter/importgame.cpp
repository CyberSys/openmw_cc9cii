#include "importgame.hpp"

//#ifdef NDEBUG
//#undef NDEBUG
//#endif

#include <cassert>

#include <components/esm3/reader.hpp>

namespace ESSImport
{
    void GAME::load(ESM3::Reader& esm)
    {
        esm.getSubRecordHeader();
        const ESM3::SubRecordHeader& subHdr = esm.subRecordHeader();

        assert(sizeof(mGMDT) == 96 && "GMME.GMDT data size mismatch");
        if (subHdr.typeId == ESM3::SUB_GMDT)
        {
            if (subHdr.dataSize == 92)
            {
                esm.get(mGMDT, 92);
                mGMDT.mSecundaPhase = 0;
            }
            else if (subHdr.dataSize == 96)
            {
                esm.get(mGMDT);
            }
            else
                esm.fail("unexpected subrecord size for GAME.GMDT");
        }

        mGMDT.mWeatherTransition &= (0x000000ff);
        mGMDT.mSecundaPhase &= (0x000000ff);
        mGMDT.mMasserPhase &= (0x000000ff);
    }
}
