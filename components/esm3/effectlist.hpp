#ifndef ESM3_EFFECTLIST_H
#define ESM3_EFFECTLIST_H

#include <cstdint>
#include <vector>

namespace ESM
{
    class ESMWriter;
}

namespace ESM3
{
    class Reader;

    #pragma pack(push, 1)
    /** Defines a spell effect. Shared between SPEL (Spells), ALCH
     (Potions) and ENCH (Item enchantments) records
     */
    struct ENAMstruct
    {
        // Magical effect, hard-coded ID
        short mEffectID;

        // Which skills/attributes are affected (for restore/drain spells
        // etc.)
        signed char mSkill, mAttribute; // -1 if N/A

        // Other spell parameters
        // TODO: These used to be signed int - not sure if there's code that relies on that
        std::uint32_t mRange; // 0 - self, 1 - touch, 2 - target (RangeType enum)
        std::uint32_t mArea, mDuration, mMagnMin, mMagnMax;
    };
    #pragma pack(pop)

    /// EffectList, ENAM subrecord
    struct EffectList
    {
        std::vector<ENAMstruct> mList;

        /// Load one effect, assumes subrecord name was already read
        void add(Reader& reader);

        /// Load all effects
        void load(Reader& reader); // was used by ENCH and ALCH

        void save(ESM::ESMWriter& esm) const;
    };

}

#endif
