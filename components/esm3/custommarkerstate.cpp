#include "custommarkerstate.hpp"

#include "reader.hpp"
#include "../esm/esmwriter.hpp"

namespace ESM3
{
#if 0
    void CustomMarker::load(Reader& esm)
    {
        esm.getHNT(mWorldX, "POSX");
        esm.getHNT(mWorldY, "POSY");
        mCell.load(esm);
        mNote = esm.getHNOString("NOTE");
    }
#endif
    void CustomMarker::save(ESM::ESMWriter& esm) const
    {
        esm.writeHNT("POSX", mWorldX);
        esm.writeHNT("POSY", mWorldY);
        mCell.save(esm);
        if (!mNote.empty())
            esm.writeHNString("NOTE", mNote);
    }
}
