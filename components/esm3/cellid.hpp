#ifndef ESM3_CELLID_H
#define ESM3_CELLID_H

#include <string>

namespace ESM
{
    class ESMWriter;
}

namespace ESM3
{
    class Reader;

    struct CellId
    {
        struct CellIndex
        {
            int mX;
            int mY;
        };

        std::string mWorldspace;
        CellIndex mIndex;
        bool mPaged;

        static const std::string sDefaultWorldspace;

        void load (Reader& reader);
        void save (ESM::ESMWriter &esm) const;
    };

    bool operator== (const CellId& left, const CellId& right);
    bool operator!= (const CellId& left, const CellId& right);
    bool operator< (const CellId& left, const CellId& right);
}

#endif
