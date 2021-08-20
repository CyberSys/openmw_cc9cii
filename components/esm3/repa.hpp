#ifndef ESM3_REPA_H
#define ESM3_REPA_H

#include <string>

namespace ESM
{
    class ESMWriter;
}

namespace ESM3
{
    class Reader;

    struct Repair
    {
        static unsigned int sRecordId;
        /// Return a string descriptor for this record type. Currently used for debugging / error logs only.
        static std::string getRecordType() { return "Repair"; }

#pragma pack(push, 1)
        struct Data
        {
            float mWeight;
            int mValue;

            int mUses;
            float mQuality;
        }; // Size = 16
#pragma pack(pop)

        Data mData;
        unsigned int mRecordFlags;
        std::string mId, mName, mModel, mIcon, mScript;

        void load(Reader& reader, bool& isDeleted);
        void save(ESM::ESMWriter& esm, bool isDeleted = false) const;

        void blank();
        ///< Set record to default state (does not touch the ID).
    };
}
#endif
