#ifndef ESM3_INGR_H
#define ESM3_INGR_H

#include <string>

namespace ESM
{
    class ESMWriter;
}

namespace ESM3
{
    class Reader;

    /*
     * Alchemy ingredient
     */

    struct Ingredient
    {
        static unsigned int sRecordId;
        /// Return a string descriptor for this record type. Currently used for debugging / error logs only.
        static std::string getRecordType() { return "Ingredient"; }

#pragma pack(push, 1)
        struct IRDTstruct
        {
            float mWeight;
            int mValue;
            int mEffectID[4]; // Effect, 0 or -1 means none
            int mSkills[4]; // SkillEnum related to effect
            int mAttributes[4]; // Attribute related to effect
        };
#pragma pack(pop)

        IRDTstruct mData;
        unsigned int mRecordFlags;
        std::string mId, mName, mModel, mIcon, mScript;

        void load(Reader& reader, bool& isDeleted);
        void save(ESM::ESMWriter& esm, bool isDeleted = false) const;

        void blank();
        ///< Set record to default state (does not touch the ID).
    };
}
#endif
