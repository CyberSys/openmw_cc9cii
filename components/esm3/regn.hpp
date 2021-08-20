#ifndef ESM_REGN_H
#define ESM_REGN_H

#include <string>
#include <vector>

//#include "esmcommon.hpp"

namespace ESM
{
    class ESMWriter;
}

namespace ESM3
{
    class Reader;

    /*
     * Region data
     */

    struct Region
    {
        static unsigned int sRecordId;
        /// Return a string descriptor for this record type. Currently used for debugging / error logs only.
        static std::string getRecordType() { return "Region"; }

#pragma pack(push, 1)
        struct WEATstruct
        {
            // These are probabilities that add up to 100
            unsigned char mClear, mCloudy, mFoggy, mOvercast, mRain, mThunder, mAsh, mBlight,
            // Unknown weather, probably snow and something. Only
            // present in file version 1.3.
            // the engine uses mA as "snow" and mB as "blizard"
                    mA, mB;
        }; // 10 bytes
#pragma pack(pop)

        // Reference to a sound that is played randomly in this region
        struct SoundRef
        {
            std::string   mSound;
            unsigned char mChance;
        };

        WEATstruct mData;
        int mMapColor; // RGBA

        // sleepList refers to a leveled list of creatures you can meet if
        // you sleep outside in this region.
        std::string mId, mName, mSleepList;

        std::vector<SoundRef> mSoundList;

        void load(Reader& reader, bool& isDeleted);
        void save(ESM::ESMWriter& esm, bool isDeleted = false) const;

        void blank();
        ///< Set record to default state (does not touch the ID/index).
    };
}
#endif
