#ifndef OPENMW_ESSIMPORT_GAME_H
#define OPENMW_ESSIMPORT_GAME_H

namespace ESM3
{
    class Reader;
}

namespace ESSImport
{

    /// Weather data
    struct GAME
    {
#pragma pack(push, 1)
        struct GMDT
        {
           char mCellName[64] {};
           int mFogColour {0};
           float mFogDensity {0.f};
           int mCurrentWeather {0}, mNextWeather {0};
           int mWeatherTransition {0}; // 0-100 transition between weathers, top 3 bytes may be garbage
           float mTimeOfNextTransition {0.f}; // weather changes when gamehour == timeOfNextTransition
           int mMasserPhase {0}, mSecundaPhase {0}; // top 3 bytes may be garbage
        };
#pragma pack(pop)

        GMDT mGMDT;

        void load(ESM3::Reader& esm);
    };

}

#endif
