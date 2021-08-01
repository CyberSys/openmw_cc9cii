#ifndef CSM_WOLRD_METADATA_H
#define CSM_WOLRD_METADATA_H

#include <string>

namespace ESM
{
    class ESMWriter;
}

namespace ESM3
{
    class Reader;
}

namespace CSMWorld
{
    struct MetaData
    {
        std::string mId;

        int mFormat;
        std::string mAuthor;
        std::string mDescription;

        void blank();

        void load (ESM3::Reader& esm);
        void save (ESM::ESMWriter& esm) const;
    };
}

#endif
