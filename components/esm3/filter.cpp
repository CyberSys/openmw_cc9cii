#include "filter.hpp"

#include "common.hpp"
#include "reader.hpp"
#include "../esm/esmwriter.hpp"

unsigned int ESM3::Filter::sRecordId = REC_FILT;

void ESM3::Filter::load (Reader& reader, bool& isDeleted)
{
    isDeleted = false;

    while (reader.getSubRecordHeader())
    {
        const ESM3::SubRecordHeader& subHdr = reader.subRecordHeader();
        switch (subHdr.typeId)
        {
            case ESM3::SUB_NAME: reader.getZString(mId); break;
            case ESM3::SUB_FILT: reader.getZString(mFilter); break;
            case ESM3::SUB_DESC: reader.getZString(mDescription); break;
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
}

void ESM3::Filter::save (ESM::ESMWriter& esm, bool isDeleted) const
{
    esm.writeHNCString ("NAME", mId);

    if (isDeleted)
    {
        esm.writeHNString("DELE", "", 3);
        return;
    }

    esm.writeHNCString ("FILT", mFilter);
    esm.writeHNCString ("DESC", mDescription);
}

void ESM3::Filter::blank()
{
    mFilter.clear();
    mDescription.clear();
}
