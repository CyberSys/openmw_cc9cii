#include "statstate.hpp"

#include "reader.hpp"
#include "../esm/esmwriter.hpp"

namespace ESM3
{
    template<typename T>
    StatState<T>::StatState() : mBase(0), mMod(0), mCurrent(0), mDamage(0), mProgress(0) {}

    template<typename T>
    void StatState<T>::load(Reader& esm, bool intFallback)
    {
        // We changed stats values from integers to floats; ensure backwards compatibility
        if (intFallback)
        {
            int base = 0;
            esm.getSubRecordHeader(ESM3::SUB_STBA);
            esm.get(base);
            mBase = static_cast<T>(base);

            int mod = 0;
            if (esm.getNextSubRecordHeader(ESM3::SUB_STMO))
            {
                esm.get(mod);
                mMod = static_cast<T>(mod);
            }

            int current = 0;
            if (esm.getNextSubRecordHeader(ESM3::SUB_STCU))
            {
                esm.get(current);
                mCurrent = static_cast<T>(current);
            }

            int oldDamage = 0;
            if (esm.getNextSubRecordHeader(ESM3::SUB_STDA))
            {
                esm.get(oldDamage);
                mDamage = static_cast<float>(oldDamage);
            }
        }
        else
        {
            mBase = 0;
            esm.getSubRecordHeader(ESM3::SUB_STBA);
            esm.get(mBase);

            mMod = 0;
            if (esm.getNextSubRecordHeader(ESM3::SUB_STMO))
                esm.get(mMod);

            mCurrent = 0;
            if (esm.getNextSubRecordHeader(ESM3::SUB_STCU))
                esm.get(mCurrent);

            mDamage = 0;
            if (esm.getNextSubRecordHeader(ESM3::SUB_STDF))
                esm.get(mDamage);

            mProgress = 0;
        }

        if (esm.getNextSubRecordHeader(ESM3::SUB_STDF))
            esm.get(mDamage);

        mProgress = 0;
        if (esm.getNextSubRecordHeader(ESM3::SUB_STPR))
        esm.get(mProgress);
    }

    template<typename T>
    void StatState<T>::save(ESM::ESMWriter& esm) const
    {
        esm.writeHNT("STBA", mBase);

        if (mMod != 0)
            esm.writeHNT("STMO", mMod);

        if (mCurrent)
            esm.writeHNT("STCU", mCurrent);

        if (mDamage)
            esm.writeHNT("STDF", mDamage);

        if (mProgress)
            esm.writeHNT("STPR", mProgress);
    }
}

template struct ESM3::StatState<int>;
template struct ESM3::StatState<float>;
