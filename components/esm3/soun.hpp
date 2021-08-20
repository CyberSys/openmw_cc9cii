#ifndef ESM3_SOUN_H
#define ESM3_SOUN_H

#include <string>

namespace ESM
{
    class ESMWriter;
}

namespace ESM3
{
    class Reader;

    struct SOUNstruct
    {
        unsigned char mVolume, mMinRange, mMaxRange;
    };

    struct Sound
    {
        static unsigned int sRecordId;
        /// Return a string descriptor for this record type. Currently used for debugging / error logs only.
        static std::string getRecordType() { return "Sound"; }

        SOUNstruct mData;
        std::string mId, mSound;

        void load(Reader& reader, bool& isDeleted);
        void save(ESM::ESMWriter& esm, bool isDeleted = false) const;

        void blank();
        ///< Set record to default state (does not touch the ID/index).
    };
}
#endif
