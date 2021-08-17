#ifndef ESM3_ACTIVESPELLS_H
#define ESM3_ACTIVESPELLS_H

#include "effectlist.hpp"
#include "../esm/defs.hpp"

#include <string>
#include <map>

namespace ESM
{
    class ESMWriter;
}

namespace ESM3
{
    class Reader;

    // Parameters of an effect concerning lasting effects.
    // Note we are not using ENAMstruct since the magnitude may be modified by magic resistance, etc.
    // It could also be a negative magnitude, in case of inversing an effect, e.g. Absorb spell causes damage on target, but heals the caster.
    struct ActiveEffect
    {
        int mEffectId;
        float mMagnitude;
        int mArg; // skill or attribute
        float mDuration;
        float mTimeLeft;
        int mEffectIndex;
    };

    // format 0, saved games only
    struct ActiveSpells
    {
        struct ActiveSpellParams
        {
            std::vector<ActiveEffect> mEffects;
            std::string mDisplayName;
            int mCasterActorId;
        };

        typedef std::multimap<std::string, ActiveSpellParams > TContainer;
        TContainer mSpells;

        void load (Reader& esm);
        void save (ESM::ESMWriter& esm) const;
    };
}

#endif
