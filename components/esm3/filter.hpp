#ifndef ESM3_FILTER_H
#define ESM3_FILTER_H

#include <string>

namespace ESM
{
    class ESMWriter;
}

namespace ESM3
{
    class Reader;

    struct Filter
    {
        static unsigned int sRecordId;

        std::string mId;

        std::string mDescription;

        std::string mFilter;

        void load (Reader& reader, bool& isDeleted);
        void save (ESM::ESMWriter& esm, bool isDeleted = false) const;

        void blank();
        ///< Set record to default state (does not touch the ID).
    };
}

#endif
