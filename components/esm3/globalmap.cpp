#include "globalmap.hpp"

#include "reader.hpp"
#include "../esm/esmwriter.hpp"
#include "../esm/defs.hpp"

unsigned int ESM3::GlobalMap::sRecordId = ESM::REC_GMAP;

#if 0
void ESM3::GlobalMap::load (Reader& esm)
{
    esm.getHNT(mBounds, "BNDS");

    esm.getSubNameIs("DATA");
    esm.getSubHeader();
    mImageData.resize(esm.getSubSize());
    esm.getExact(&mImageData[0], mImageData.size());

    while (esm.isNextSub("MRK_"))
    {
        esm.getSubHeader();
        CellId cell;
        esm.getT(cell.first);
        esm.getT(cell.second);
        mMarkers.insert(cell);
    }
}
#endif
void ESM3::GlobalMap::save (ESM::ESMWriter& esm) const
{
    esm.writeHNT("BNDS", mBounds);

    esm.startSubRecord("DATA");
    esm.write(&mImageData[0], mImageData.size());
    esm.endRecord("DATA");

    for (std::set<CellId>::const_iterator it = mMarkers.begin(); it != mMarkers.end(); ++it)
    {
        esm.startSubRecord("MRK_");
        esm.writeT(it->first);
        esm.writeT(it->second);
        esm.endRecord("MRK_");
    }
}
