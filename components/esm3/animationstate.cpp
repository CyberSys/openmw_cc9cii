#include "animationstate.hpp"

#include "reader.hpp"
#include "../esm/esmwriter.hpp"

namespace ESM3
{
    bool AnimationState::empty() const
    {
        return mScriptedAnims.empty();
    }

    // NOTE: this method is called from ObjectState::load()
    // NOTE: assumed the sub-record header has *not* been read
    void AnimationState::load(Reader& esm)
    {
#if 0
        mScriptedAnims.clear();

        while (esm.getSubRecordHeader() && esm.subRecordHeader().typeId == ESM3::SUB_ANIS)
        {
            ScriptedAnimation anim;

            esm.getZString(anim.mGroup); // FIXME: check
            esm.getSubRecordHeader();
            if (esm.subRecordHeader().typeId == ESM3::SUB_TIME)
                esm.get(anim.mTime);

            esm.getHNOT(anim.mAbsolute, "ABST");

            esm.getSubNameIs("COUN");
            // workaround bug in earlier version where size_t was used
            esm.getSubHeader();
            if (esm.getSubSize() == 8)
                esm.getT(anim.mLoopCount);
            else
            {
                uint32_t loopcount;
                esm.get(loopcount);
                anim.mLoopCount = (uint64_t) loopcount;
            }

            mScriptedAnims.push_back(anim);
        }
#endif
    }

    void AnimationState::save(ESM::ESMWriter& esm) const
    {
        for (ScriptedAnimations::const_iterator iter = mScriptedAnims.begin(); iter != mScriptedAnims.end(); ++iter)
        {
            esm.writeHNString("ANIS", iter->mGroup);
            if (iter->mTime > 0)
                esm.writeHNT("TIME", iter->mTime);
            if (iter->mAbsolute)
                esm.writeHNT("ABST", iter->mAbsolute);
            esm.writeHNT("COUN", iter->mLoopCount);
        }
    }
}
