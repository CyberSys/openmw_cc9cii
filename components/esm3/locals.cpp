#include "locals.hpp"

#include "common.hpp"
#include "reader.hpp"
#include "../esm/esmwriter.hpp"

bool ESM3::Locals::load (Reader& reader)
{
    while (reader.getSubRecordHeader())
    {
        const ESM3::SubRecordHeader& subHdr = reader.subRecordHeader();
        switch (subHdr.typeId)
        {
            case ESM3::SUB_LOCA:
            {
                std::string id;
                reader.getString(id); // NOTE: not null terminated, unless omwsave :-(

                Variant value;
                value.read (reader, Variant::Format_Local);

                mVariables.emplace_back (id, value);
                break;
            }
            default:
                return true; // indicate that sub-record header was read
        }
    }

    return false;
}

void ESM3::Locals::save (ESM::ESMWriter& esm) const
{
    for (std::vector<std::pair<std::string, Variant> >::const_iterator iter (mVariables.begin());
        iter!=mVariables.end(); ++iter)
    {
        esm.writeHNString ("LOCA", iter->first);
        iter->second.write (esm, Variant::Format_Local);
    }
}
