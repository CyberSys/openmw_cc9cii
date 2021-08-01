#ifndef ESM3_ENCH_H
#define ESM3_ENCH_H

#include <string>
#include <cstdint>

#include "effectlist.hpp"

namespace ESM
{
    class ESMWriter;
}

namespace ESM3
{
    class Reader;

    /*
     * Enchantments
     */

    struct Enchantment
    {
        static unsigned int sRecordId;
        /// Return a string descriptor for this record type. Currently used for debugging / error logs only.
        static std::string getRecordType() { return "Enchantment"; }

        enum Type
        {
            CastOnce = 0,
            WhenStrikes = 1,
            WhenUsed = 2,
            ConstantEffect = 3
        };

        enum Flags
        {
            Autocalc = 0x01
        };

#pragma pack(push, 1)
        struct ENDTstruct
        {
            std::uint32_t mType;
            std::uint32_t mCost;
            std::uint32_t mCharge;
            std::uint32_t mFlags;
        };
#pragma pack(pop)

        std::string mId;
        ENDTstruct mData;
        EffectList mEffects;

        void load(Reader& reader, bool& isDeleted);
        void save(ESM::ESMWriter& esm, bool isDeleted = false) const;

        void blank();
        ///< Set record to default state (does not touch the ID).
    };
}
#endif
