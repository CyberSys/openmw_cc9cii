#include "globalscript.hpp"

#include "reader.hpp"
#include "../esm/esmwriter.hpp"

#if 0
void ESM3::GlobalScript::load (Reader& esm)
{
    mId = esm.getHNString ("NAME");

    mLocals.load (esm);

    mRunning = 0;
    esm.getHNOT (mRunning, "RUN_");

    mTargetRef.unset();
    mTargetId = esm.getHNOString ("TARG");
    if (esm.peekNextSub("FRMR"))
        mTargetRef.load(esm, true, "FRMR");
}
#endif
void ESM3::GlobalScript::save (ESM::ESMWriter& esm) const
{
    esm.writeHNString ("NAME", mId);

    mLocals.save (esm);

    if (mRunning)
        esm.writeHNT ("RUN_", mRunning);

    if (!mTargetId.empty())
    {
        esm.writeHNOString ("TARG", mTargetId);
        if (mTargetRef.isSet())
            mTargetRef.save (esm, true, "FRMR");
    }
}
