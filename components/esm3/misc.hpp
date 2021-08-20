#ifndef ESM3_MISC_H
#define ESM3_MISC_H

#include <string>

namespace ESM
{
    class ESMWriter;
}

namespace ESM3
{
    class Reader;

    /*
     * Misc inventory items, basically things that have no use but can be
     * carried, bought and sold. It also includes keys.
     */

    struct Miscellaneous
    {
        static unsigned int sRecordId;
        /// Return a string descriptor for this record type. Currently used for debugging / error logs only.
        static std::string getRecordType() { return "Miscellaneous"; }

#pragma pack(push, 1)
        struct MCDTstruct
        {
            float mWeight;
            int mValue;
            int mIsKey; // There are many keys in Morrowind.esm that has this
                       // set to 0. TODO: Check what this field corresponds to
                       // in the editor.
        };
#pragma pack(pop)

        MCDTstruct mData;

        unsigned int mRecordFlags;
        std::string mId, mName, mModel, mIcon, mScript;

        void load(Reader& reader, bool& isDeleted);
        void save(ESM::ESMWriter& esm, bool isDeleted = false) const;

        void blank();
        ///< Set record to default state (does not touch the ID).
    };
}
#endif
