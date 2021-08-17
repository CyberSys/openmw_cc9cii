#include "magiceffects.hpp"

#include "reader.hpp"
#include "../esm/esmwriter.hpp"

namespace ESM3
{
    void MagicEffects::load(Reader& esm)
    {
        bool finished = false;
        while (!finished && esm.getSubRecordHeader())
        {
            const ESM3::SubRecordHeader& subHdr = esm.subRecordHeader();
            switch (subHdr.typeId)
            {
                case ESM3::SUB_EFID:
                {
                    int id, base;
                    esm.get(id);

                    esm.getSubRecordHeader();
                    assert(esm.subRecordHeader().typeId == ESM3::SUB_BASE);
                    esm.get(base);

                    mEffects.insert(std::make_pair(id, base));
                    break;
                }
                default:
                    finished = true;
                    esm.cacheSubRecordHeader();
                    break;
            }
        }
    }

    void MagicEffects::save(ESM::ESMWriter& esm) const
    {
        for (std::map<int, int>::const_iterator it = mEffects.begin(); it != mEffects.end(); ++it)
        {
            esm.writeHNT("EFID", it->first);
            esm.writeHNT("BASE", it->second);
        }
    }
}
