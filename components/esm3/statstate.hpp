#ifndef ESM_STATSTATE_H
#define ESM_STATSTATE_H

namespace ESM
{
    class ESMWriter;
}

namespace ESM3
{
    class Reader;

    // format 0, saved games only

    template<typename T>
    struct StatState
    {
        T mBase;
        T mMod; // Note: can either be the modifier, or the modified value.
                // A bit inconsistent, but we can't fix this without breaking compatibility.
        T mCurrent;
        float mDamage;
        float mProgress;

        StatState();

        void load (Reader& esm, bool intFallback = false);
        void save (ESM::ESMWriter& esm) const;
    };
}

#endif
