#ifndef ESM3_SPELLLIST_H
#define ESM3_SPELLLIST_H

#include <vector>
#include <string>

namespace ESM
{
    class ESMWriter;
}

namespace ESM3
{
    class Reader;

    /** A list of references to spells and spell effects. This is shared
     between the records BSGN, NPC and RACE.
     NPCS subrecord.
     */
    struct SpellList
    {
        std::vector<std::string> mList;

        /// Is this spell ID in mList?
        bool exists(const std::string& spell) const;

        /// Load one spell, assumes the subrecord name was already read
        void add(Reader& esm);

        void save(ESM::ESMWriter& esm) const;
    };
}

#endif

