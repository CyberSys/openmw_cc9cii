#ifndef ESM3_GLOB_H
#define ESM3_GLOB_H

#include <string>

#include "variant.hpp"

namespace ESM
{
    class ESMWriter;
}

namespace ESM3
{
    class Reader;

    /*
     * Global script variables
     */

    struct Global
    {
        static unsigned int sRecordId;
        /// Return a string descriptor for this record type. Currently used for debugging / error logs only.
        static std::string getRecordType() { return "Global"; }

        std::string mId;
        Variant mValue;

        void load(Reader& reader, bool& isDeleted);
        void save(ESM::ESMWriter& esm, bool isDeleted = false) const;

        void blank();
        ///< Set record to default state (does not touch the ID).
    };

    bool operator== (const Global& left, const Global& right);
}
#endif
