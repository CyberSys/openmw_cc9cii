#ifndef ESM3_ALCH_H
#define ESM3_ALCH_H

#include <string>

#include "effectlist.hpp" // currently for save() only

namespace ESM
{
    class ESMWriter;
}

namespace ESM3
{
    class Reader;

    /*
     * Alchemy item (potions)
     */

    struct Potion
    {
        static unsigned int sRecordId;

        /// Return a string descriptor for this record type. Currently used for debugging / error logs only.
        static std::string getRecordType() { return "Potion"; }

#pragma pack(push, 1)
        struct ALDTstruct
        {
            float mWeight;
            int mValue;
            int mAutoCalc;
        };
#pragma pack(pop)

        ALDTstruct mData;

        unsigned int mRecordFlags;
        std::string mId, mName, mModel, mIcon, mScript;
        EffectList mEffects;

        void load(Reader& reader, bool& isDeleted);
        void save(ESM::ESMWriter& esm, bool isDeleted = false) const;

        void blank();
        ///< Set record to default state (does not touch the ID).
    };
}
#endif
