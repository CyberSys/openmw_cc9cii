#ifndef ESM3_TES3_H
#define ESM3_TES3_H

#include <vector>

#include "../esm/common.hpp" // ESMVersion, MasterData
//#include "../esm/esmcommon.hpp"

namespace ESM
{
    class ESMWriter;
}

namespace ESM3
{
    class Reader;

#pragma pack(push, 1)
    struct Data
    {
        /* File format version. This is actually a float, the supported
            versions are 1.2 and 1.3. These correspond to:
            1.2 = 0x3f99999a and 1.3 = 0x3fa66666
        */
        ESM::ESMVersion version;
        std::uint32_t   type;    // 0=esp, 1=esm, 32=ess (unused)
        std::string     author;  // Author's name
        std::string     desc;    // File description
        std::uint32_t   records; // Number of records
    };

    struct GMDT
    {
        float mCurrentHealth;
        float mMaximumHealth;
        float mHour;
        unsigned char unknown1[12];
        char mCurrentCell[64];       // FIXME: convert to NAME64
        unsigned char unknown2[4];
        char mPlayerName[32];        // FIXME: convert to NAME32
    };
#pragma pack(pop)

    /// \brief File header record
    struct Header
    {
        static const int CurrentFormat = 0; // most recent known format

        GMDT mGameData;                   // Used in .ess savegames only
        std::vector<unsigned char> mSCRD; // Used in .ess savegames only, unknown
        std::vector<unsigned char> mSCRS; // Used in .ess savegames only, screenshot

        Data mData;
        int mFormat;
        std::vector<ESM::MasterData> mMaster;

        void blank();

        void load (Reader &esm);
        void save (ESM::ESMWriter &esm);
    };

}

#endif // ESM3_TES3_H
