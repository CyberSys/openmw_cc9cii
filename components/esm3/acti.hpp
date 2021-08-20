#ifndef ESM3_ACTI_H
#define ESM3_ACTI_H

#include <string>

namespace ESM
{
    class ESMWriter;
}

namespace ESM3
{
    class Reader;

    struct Activator
    {
        static unsigned int sRecordId;
        /// Return a string descriptor for this record type. Currently used for debugging / error logs only.
        static std::string getRecordType() { return "Activator"; }

        unsigned int mRecordFlags;
        std::string mId, mName, mScript, mModel;

        void load(Reader& reader, bool& isDeleted);
        void save(ESM::ESMWriter& esm, bool isDeleted = false) const;

        void blank();
        ///< Set record to default state (does not touch the ID).
    };
}
#endif
