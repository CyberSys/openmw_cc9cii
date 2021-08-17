#ifndef ESM3_MAGICEFFECTS_H
#define ESM3_MAGICEFFECTS_H

#include <map>
#include <string>

namespace ESM
{
    class ESMWriter;
}

namespace ESM3
{
    class Reader;

    // format 0, saved games only
    struct MagicEffects
    {
        // <Effect Id, Base value>
        std::map<int, int> mEffects;

        void load (Reader& esm);
        void save (ESM::ESMWriter& esm) const;
    };

    struct SummonKey
    {
        SummonKey(int effectId, const std::string& sourceId, int index):
            mEffectId(effectId), mSourceId(sourceId), mEffectIndex(index)
        {}

        bool operator==(const SummonKey &other) const
        {
            return mEffectId == other.mEffectId &&
                    mSourceId == other.mSourceId &&
                    mEffectIndex == other.mEffectIndex;
        }

        bool operator<(const SummonKey &other) const
        {
            if (mEffectId < other.mEffectId)
                return true;
            if (mEffectId > other.mEffectId)
                return false;

            if (mSourceId < other.mSourceId)
                return true;
            if (mSourceId > other.mSourceId)
                return false;

            return mEffectIndex < other.mEffectIndex;
        }

        int mEffectId;
        std::string mSourceId;
        int mEffectIndex;
    };
}

#endif
