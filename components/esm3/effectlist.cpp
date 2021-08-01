#include "effectlist.hpp"

#include "reader.hpp"
#include "../esm/esmwriter.hpp"

namespace ESM3
{
#if 0
    void EffectList::load(Reader& reader)
    {
        mList.clear();
        while (reader.isNextSub("ENAM"))
        {
            add(reader);
        }
    }

    void EffectList::add(Reader& reader)
    {
        ENAMstruct s;
        reader.getHT(s, 24);
        mList.push_back(s);
    }
#endif
    void EffectList::save(ESM::ESMWriter& esm) const
    {
        for (std::vector<ENAMstruct>::const_iterator it = mList.begin(); it != mList.end(); ++it) {
            esm.writeHNT<ENAMstruct>("ENAM", *it, 24);
        }
    }
} // end namespace
