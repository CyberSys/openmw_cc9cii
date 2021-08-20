#ifndef ESM3_SPELLSTATE_H
#define ESM3_SPELLSTATE_H

#include <map>
#include <vector>
#include <string>
#include <set>

#include "../esm/defs.hpp"

namespace ESM
{
    class ESMWriter;
}

namespace ESM3
{
    class Reader;

    // NOTE: spell ids must be lower case
    struct SpellState
    {
        struct CorprusStats
        {
            int mWorsenings;
            ESM::TimeStamp mNextWorsening;
        };

        struct PermanentSpellEffectInfo
        {
            int mId;
            int mArg;
            float mMagnitude;
        };

        struct SpellParams
        {
            std::map<int, float> mEffectRands;
            std::set<int> mPurgedEffects;
        };
        typedef std::map<std::string, SpellParams> TContainer;
        TContainer mSpells;

        // FIXME: obsolete, used only for old saves
        std::map<std::string, std::vector<PermanentSpellEffectInfo> > mPermanentSpellEffects;
        std::map<std::string, CorprusStats> mCorprusSpells;

        std::map<std::string, ESM::TimeStamp> mUsedPowers;

        std::string mSelectedSpell;

        void load (Reader& esm);
        void save (ESM::ESMWriter& esm) const;
    };

}

#endif
