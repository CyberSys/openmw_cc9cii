#ifndef ESM_RACE_H
#define ESM_RACE_H

#include <string>
#include <cstdint>

#include "spelllist.hpp"

namespace ESM
{
    class ESMWriter;
}

namespace ESM3
{
    class Reader;

    /*
     * Race definition
     */

    struct Race
    {
        static unsigned int sRecordId;
        /// Return a string descriptor for this record type. Currently used for debugging / error logs only.
        static std::string getRecordType() { return "Race"; }

        enum Flags
        {
            Playable = 0x01,
            Beast = 0x02
        };

#pragma pack(push, 1)
        struct SkillBonus
        {
            std::int32_t mSkill; // SkillEnum
            std::int32_t mBonus;
        };

        struct MaleFemaleF
        {
            float mMale, mFemale;

            float getValue (bool male) const;
        };

        struct MaleFemale
        {
            std::uint32_t mMale, mFemale;

            std::uint32_t getValue (bool male) const;
        };

        struct RADTstruct
        {
            // List of skills that get a bonus
            SkillBonus mBonus[7];

            // Attribute values for male/female
            MaleFemale mAttributeValues[8];

            // The actual eye level height (in game units) is (probably) given
            // as 'height' times 128. This has not been tested yet.
            MaleFemaleF mHeight, mWeight;

            std::uint32_t mFlags; // 0x1 - playable, 0x2 - beast race

        }; // Size = 140 bytes
#pragma pack(pop)

        RADTstruct mData;

        std::string mId, mName, mDescription;
        SpellList mPowers;

        void load(Reader& reader, bool& isDeleted);
        void save(ESM::ESMWriter& esm, bool isDeleted = false) const;

        void blank();
        ///< Set record to default state (does not touch the ID/index).
    };
}
#endif
