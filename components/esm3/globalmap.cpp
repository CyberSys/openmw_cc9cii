#include "globalmap.hpp"

#include "reader.hpp"
#include "../esm/esmwriter.hpp"
#include "../esm/defs.hpp"

namespace ESM3
{
    unsigned int GlobalMap::sRecordId = ESM::REC_GMAP;

    // NOTE: "MRK_" equivalent to each loaded exterior REC_CELL (that's been visited - see flag)
    // TODO: find out what is "mark location" in SUB_PNAM of REC_PCDT
    //
    // NOTE: "DATA" equivalent to SUB_MAPD in REC_FMAP (but scaled)
    //       mBounds is derived from the image data
    //
    // (called from StateManager::loadGame() via WindowManager::readRecord() and MapWindow::readRecord())
    void GlobalMap::load (Reader& esm)
    {
        while (esm.getSubRecordHeader())
        {
            const ESM3::SubRecordHeader& subHdr = esm.subRecordHeader();
            switch (subHdr.typeId)
            {
                case ESM3::SUB_BNDS:
                {
                    esm.get(mBounds);
                    break;
                }
                case ESM3::SUB_DATA:
                {
                    mImageData.resize(subHdr.dataSize);
                    esm.get(mImageData[0], mImageData.size());
                    break;
                }
                case ESM3::SUB_MRK_:
                {
                    CellId cell;
                    esm.get(cell.first);
                    esm.get(cell.second);
                    mMarkers.insert(cell);
                    break;
                }
                default:
                    esm.skipSubRecordData();
                    break;
            }
        }
    }

    void GlobalMap::save (ESM::ESMWriter& esm) const
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
}
