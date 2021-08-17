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
        mScriptedAnims.clear();

        while (esm.getSubRecordHeader())
        {
            if (esm.subRecordHeader().typeId == ESM3::SUB_ANIS)
            {
                ScriptedAnimation anim;

                esm.getString(anim.mGroup); // NOTE: string not null terminated
                esm.getSubRecordHeader();
                if (esm.subRecordHeader().typeId == ESM3::SUB_TIME)
                {
                    esm.get(anim.mTime);
                    esm.getSubRecordHeader();
                }

                if (esm.subRecordHeader().typeId == ESM3::SUB_ABST)
                {
                    esm.get(anim.mAbsolute);
                    esm.getSubRecordHeader();
                }

                assert(esm.subRecordHeader().typeId == ESM3::SUB_COUN);
                // workaround bug in earlier version where size_t was used
                if (esm.subRecordHeader().dataSize == 8)
                    esm.get(anim.mLoopCount);
                else
                {
                    uint32_t loopcount;
                    esm.get(loopcount);
                    anim.mLoopCount = (uint64_t) loopcount;
                }

                mScriptedAnims.push_back(anim);
            }
            else
            {
                esm.cacheSubRecordHeader();
                return;
            }
        }
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
