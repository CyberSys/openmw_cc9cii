#include "transport.hpp"

//#ifdef NDEBUG
//#undef NDEBUG
//#endif

#include <cassert>

#include <components/debug/debuglog.hpp>

#include "common.hpp"
#include "reader.hpp"
#include "../esm/esmwriter.hpp"

namespace ESM3
{
    // NOTE: assumes the sub-record header was just read
    void Transport::add(Reader& reader)
    {
        const ESM3::SubRecordHeader& subHdr = reader.subRecordHeader();
        if (subHdr.typeId == ESM3::SUB_DODT)
        {
            Dest dodt;
            assert (subHdr.dataSize == 24 && "Transport pos size mismatch");
            reader.get(dodt.mPos);
            mList.push_back(dodt);
        }
        else if (subHdr.typeId == ESM3::SUB_DNAM)
        {
            std::string name;
            reader.getZString(name);
            if (mList.empty())
                Log(Debug::Warning) << "Encountered DNAM record without DODT record, skipped.";
            else
                mList.back().mCellName = name;
        }
    }

    void Transport::save(ESM::ESMWriter& esm) const
    {
        typedef std::vector<Dest>::const_iterator DestIter;
        for (DestIter it = mList.begin(); it != mList.end(); ++it)
        {
            esm.writeHNT("DODT", it->mPos, sizeof(it->mPos));
            esm.writeHNOCString("DNAM", it->mCellName);
        }
    }

}
