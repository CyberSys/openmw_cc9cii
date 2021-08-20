#include "debugprofile.hpp"

#include "common.hpp"
#include "reader.hpp"
#include "../esm/esmwriter.hpp"

unsigned int ESM3::DebugProfile::sRecordId = REC_DBGP;

void ESM3::DebugProfile::load (Reader& reader, bool& isDeleted)
{
    isDeleted = false;

    while (reader.getSubRecordHeader())
    {
        const ESM3::SubRecordHeader& subHdr = reader.subRecordHeader();
        switch (subHdr.typeId)
        {
            case ESM3::SUB_NAME: reader.getString(mId); break; // TODO: check string not null terminated
            case ESM3::SUB_DESC: reader.getString(mDescription); break; // TODO: check string not null terminated
            case ESM3::SUB_SCRP: reader.getString(mScriptText); break; // TODO: check string not null terminated
            case ESM3::SUB_FLAG: reader.get(mFlags); break;
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

void ESM3::DebugProfile::save (ESM::ESMWriter& esm, bool isDeleted) const
{
    esm.writeHNCString ("NAME", mId);

    if (isDeleted)
    {
        esm.writeHNString("DELE", "", 3);
        return;
    }

    esm.writeHNCString ("DESC", mDescription);
    esm.writeHNCString ("SCRP", mScriptText);
    esm.writeHNT ("FLAG", mFlags);
}

void ESM3::DebugProfile::blank()
{
    mDescription.clear();
    mScriptText.clear();
    mFlags = 0;
}
