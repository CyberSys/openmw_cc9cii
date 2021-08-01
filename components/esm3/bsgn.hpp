#ifndef ESM3_BSGN_H
#define ESM3_BSGN_H

#include <string>

#include "spelllist.hpp"

namespace ESM
{
    class ESMWriter;
}

namespace ESM3
{
    class Reader;

    struct BirthSign
    {
        static unsigned int sRecordId;
        /// Return a string descriptor for this record type. Currently used for debugging / error logs only.
        static std::string getRecordType() { return "BirthSign"; }

        std::string mId, mName, mDescription, mTexture;

        // List of powers and abilities that come with this birth sign.
        SpellList mPowers;

        void load(Reader& reader, bool& isDeleted);
        void save(ESM::ESMWriter& esm, bool isDeleted = false) const;

        void blank();
        ///< Set record to default state (does not touch the ID/index).
    };
}
#endif
