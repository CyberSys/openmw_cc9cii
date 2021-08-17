#ifndef ESM3_QUICKKEYS_H
#define ESM3_QUICKKEYS_H

#include <vector>
#include <string>

namespace ESM
{
    class ESMWriter;
}

namespace ESM3
{
    class Reader;

    struct QuickKeys
    {
        struct QuickKey
        {
            int mType;
            std::string mId; // Spell or Item ID
        };

        std::vector<QuickKey> mKeys;

        void load (Reader& esm);
        void save (ESM::ESMWriter& esm) const;
    };
}

#endif
