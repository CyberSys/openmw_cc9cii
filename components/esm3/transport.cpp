#include "transport.hpp"

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
            if (subHdr.dataSize != sizeof(dodt.mPos) || subHdr.dataSize != 24)
                reader.fail("Transport pos incorrect data size");
            reader.get(dodt.mPos);
            mList.push_back(dodt);
        }
        else if (subHdr.typeId == ESM3::SUB_DNAM)
        {
            std::string name;
            reader.getString(name); // TODO: check string not null terminated
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
