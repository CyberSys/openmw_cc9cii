#include "spelllist.hpp"

#include <components/misc/stringops.hpp>

#include "common.hpp"
#include "reader.hpp"
#include "../esm/esmwriter.hpp"

namespace ESM3
{
    void SpellList::add(Reader& reader)
    {
        std::string spell;
        reader.getZString(spell);
        mList.push_back(spell);
    }

    void SpellList::save(ESM::ESMWriter& esm) const
    {
        for (std::vector<std::string>::const_iterator it = mList.begin(); it != mList.end(); ++it) {
            esm.writeHNString("NPCS", *it, 32);
        }
    }

    bool SpellList::exists(const std::string &spell) const
    {
        for (std::vector<std::string>::const_iterator it = mList.begin(); it != mList.end(); ++it)
            if (Misc::StringUtils::ciEqual(*it, spell))
                return true;
        return false;
    }
}
