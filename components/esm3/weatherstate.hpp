#ifndef ESM3_WEATHERSTATE_H
#define ESM3_WEATHERSTATE_H

#include <map>
#include <string>
#include <vector>

namespace ESM
{
    class ESMWriter;
}

namespace ESM3
{
    class Reader;

    struct RegionWeatherState
    {
        int mWeather;
        std::vector<char> mChances;
    };

    struct WeatherState
    {
        std::string mCurrentRegion;
        float mTimePassed;
        bool mFastForward;
        float mWeatherUpdateTime;
        float mTransitionFactor;
        int mCurrentWeather;
        int mNextWeather;
        int mQueuedWeather;
        std::map<std::string, RegionWeatherState> mRegions;

        void load(Reader& esm);
        void save(ESM::ESMWriter& esm) const;
    };
}

#endif
