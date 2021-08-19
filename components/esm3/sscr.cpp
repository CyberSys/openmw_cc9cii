#include "sscr.hpp"

#include <cassert>

#include "common.hpp"
#include "reader.hpp"
#include "../esm/esmwriter.hpp"

namespace ESM3
{
    unsigned int StartScript::sRecordId = REC_SSCR;

    void StartScript::load(Reader& reader, bool& isDeleted)
    {
        isDeleted = false;

        bool hasData = false;
        bool hasName = false;
        while (reader.getSubRecordHeader())
        {
            const ESM3::SubRecordHeader& subHdr = reader.subRecordHeader();
            switch (subHdr.typeId)
            {
                case ESM3::SUB_NAME:
                {
                    reader.getString(mId); // NOTE: string not null terminated
                    assert(subHdr.dataSize == mId.size() && "SSCR id string length incorrect");
                    hasName = true;
                    break;
                }
                case ESM3::SUB_DATA:
                {
                    reader.getString(mData); // NOTE: string not null terminated
                    assert(subHdr.dataSize == mData.size() && "SSCR data string length incorrect");
                    hasData = true;
                    break;
                }
                case ESM3::SUB_DELE:
                {
                    reader.skipSubRecordData();
                    isDeleted = true;
                    break;
                }
                default:
                    reader.fail("Unknown subrecord");
                    break;
            }
        }

        if (!hasName)
            reader.fail("Missing NAME");

        if (!hasData && !isDeleted)
            reader.fail("Missing DATA");
    }

    void StartScript::save(ESM::ESMWriter& esm, bool isDeleted) const
    {
        esm.writeHNCString("NAME", mId);
        if (isDeleted)
        {
            esm.writeHNString("DELE", "", 3);
        }
        else
        {
            esm.writeHNString("DATA", mData);
        }
    }

    void StartScript::blank()
    {
        mData.clear();
    }
}
