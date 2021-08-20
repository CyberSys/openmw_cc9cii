#include "custommarkerstate.hpp"

#include "reader.hpp"
#include "../esm/esmwriter.hpp"

namespace ESM3
{
    // equivalent to SUB_MPCD in REC_CELL
    // (called from StateManager::loadGame() via WindowManager::readRecord())
    void CustomMarker::load(Reader& esm)
    {
        while (esm.getSubRecordHeader())
        {
            const ESM3::SubRecordHeader& subHdr = esm.subRecordHeader();
            switch (subHdr.typeId)
            {
                case ESM3::SUB_POSX:
                {
                    esm.get(mWorldX);
                    break;
                }
                case ESM3::SUB_POSY:
                {
                    esm.get(mWorldY);

                    // read SUB_SPAC (worldspace) and SUB_CIDX (if paged)
                    mCell.load(esm);
                    break;
                }
                case ESM3::SUB_NOTE:
                {
                    esm.getString(mNote); // NOTE: string not null terminated
                    break;
                }
                default:
                    esm.skipSubRecordData();
                    break;
            }
        }
    }

    void CustomMarker::save(ESM::ESMWriter& esm) const
    {
        esm.writeHNT("POSX", mWorldX);
        esm.writeHNT("POSY", mWorldY);
        mCell.save(esm);
        if (!mNote.empty())
            esm.writeHNString("NOTE", mNote);
    }
}
