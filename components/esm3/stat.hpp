#ifndef ESM3_STAT_H
#define ESM3_STAT_H

#include <string>

namespace ESM
{
    class ESMWriter;
}

namespace ESM3
{
    class Reader;

    /*
     * Definition of static object.
     *
     * A stat record is basically just a reference to a nif file. Some
     * esps seem to contain copies of the STAT entries from the esms, and
     * the esms themselves contain several identical entries. Perhaps all
     * statics referenced in a file is also put in the file? Since we are
     * only reading files it doesn't much matter to us, but it would if we
     * were writing our own ESM/ESPs. You can check some files later when
     * you decode the CELL blocks, if you want to test this hypothesis.
     */

    struct Static
    {
        static unsigned int sRecordId;
        /// Return a string descriptor for this record type. Currently used for debugging / error logs only.
        static std::string getRecordType() { return "Static"; }

        unsigned int mRecordFlags;
        std::string mId, mModel;

        void load(Reader& reader, bool& isDeleted);
        void save(ESM::ESMWriter& esm, bool isDeleted = false) const;

        void blank();
        ///< Set record to default state (does not touch the ID).
    };
}
#endif
