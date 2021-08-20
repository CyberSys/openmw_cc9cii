#include "importscpt.hpp"

#include <cassert>

#include <components/esm3/reader.hpp>

namespace ESSImport
{
    void SCPT::load(ESM3::Reader& esm)
    {
        assert(esm.hdr().typeId == ESM3::REC_SCPT);

        esm.getSubRecordHeader(ESM3::SUB_SCHD);
        esm.get(mSCHD);

        esm.getSubRecordHeader(); // mSCRI expects this
        mSCRI.load(esm);

        mRefNum = -1;
        if (esm.getSubRecordHeader())
        {
            if (esm.subRecordHeader().typeId == ESM3::SUB_RNAM)
            {
                esm.get(mRefNum);
                mRunning = true;
            }
            else
                mRunning = false;
        }
    }
}
