#include "effectlist.hpp"

#include "reader.hpp"
#include "../esm/esmwriter.hpp"

namespace ESM3
{
    void EffectList::load(Reader& reader)
    {
        mList.clear();
        while (reader.getNextSubRecordType() == ESM3::SUB_ENAM && reader.getSubRecordHeader())
        {
            add(reader);
        }
    }

    void EffectList::add(Reader& reader)
    {
        ENAMstruct s;
        reader.get(s, 24);
        mList.push_back(s);
    }

    void EffectList::save(ESM::ESMWriter& esm) const
    {
        for (std::vector<ENAMstruct>::const_iterator it = mList.begin(); it != mList.end(); ++it) {
            esm.writeHNT<ENAMstruct>("ENAM", *it, 24);
        }
    }
} // end namespace
