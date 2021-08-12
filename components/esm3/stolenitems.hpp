#ifndef ESM3_STOLENITEMS_H
#define ESM3_STOLENITEMS_H

#include <map>
#include <string>

namespace ESM
{
    class ESMWriter;
}

namespace ESM3
{
    class Reader;

    // format 0, saved games only
    struct StolenItems
    {
        typedef std::map<std::string, std::map<std::pair<std::string, bool>, int> > StolenItemsMap;
        StolenItemsMap mStolenItems;

        void load(Reader& esm);
        void write(ESM::ESMWriter& esm) const;
    };

}

#endif
