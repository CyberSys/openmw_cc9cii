#ifndef ESM3_SKIL_H
#define ESM3_SKIL_H

#include <array>
#include <string>
#include <cstdint>

namespace ESM
{
    class ESMWriter;
}

namespace ESM3
{
    class Reader;

    /*
     * Skill information
     *
     */

    struct Skill
    {
        static unsigned int sRecordId;
        /// Return a string descriptor for this record type. Currently used for debugging / error logs only.
        static std::string getRecordType() { return "Skill"; }

        std::string mId;

#pragma pack(push, 1)
        struct SKDTstruct
        {
            // TODO: These used to be signed int - there are existing code that relies on that
            std::uint32_t mAttribute;      // see defs.hpp
            std::uint32_t mSpecialization; // 0 - Combat, 1 - Magic, 2 - Stealth
            float mUseValue[4];            // How much skill improves through use. Meaning
                                           // of each field depends on what skill this
                                           // is. We should document this better later.
        }; // Total size: 24 bytes
#pragma pack(pop)

        SKDTstruct mData;

        // Skill index. Skils don't have an id ("NAME") like most records,
        // they only have a numerical index that matches one of the
        // hard-coded skills in the game.
        std::uint32_t mIndex;

        std::string mDescription;

        enum SkillEnum
        {
            Block = 0,
            Armorer = 1,
            MediumArmor = 2,
            HeavyArmor = 3,
            BluntWeapon = 4,
            LongBlade = 5,
            Axe = 6,
            Spear = 7,
            Athletics = 8,
            Enchant = 9,
            Destruction = 10,
            Alteration = 11,
            Illusion = 12,
            Conjuration = 13,
            Mysticism = 14,
            Restoration = 15,
            Alchemy = 16,
            Unarmored = 17,
            Security = 18,
            Sneak = 19,
            Acrobatics = 20,
            LightArmor = 21,
            ShortBlade = 22,
            Marksman = 23,
            Mercantile = 24,
            Speechcraft = 25,
            HandToHand = 26,
            Length
        };

        static const std::string sSkillNames[Length];
        static const std::string sSkillNameIds[Length];
        static const std::string sIconNames[Length];
        static const std::array<SkillEnum, Length> sSkillIds;

        void load(Reader& reader, bool& isDeleted);
        void save(ESM::ESMWriter& esm, bool isDeleted = false) const;

        void blank();
         ///< Set record to default state (does not touch the ID/index).

        static std::string indexToId (int index);
    };
}
#endif
