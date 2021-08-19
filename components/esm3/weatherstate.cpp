#include "weatherstate.hpp"

#include "reader.hpp"
#include "../esm/esmwriter.hpp"

namespace
{
    const char* currentRegionRecord     = "CREG";
    const char* timePassedRecord        = "TMPS";
    const char* fastForwardRecord       = "FAST";
    const char* weatherUpdateTimeRecord = "WUPD";
    const char* transitionFactorRecord  = "TRFC";
    const char* currentWeatherRecord    = "CWTH";
    const char* nextWeatherRecord       = "NWTH";
    const char* queuedWeatherRecord     = "QWTH";
    const char* regionNameRecord        = "RGNN";
    const char* regionWeatherRecord     = "RGNW";
    const char* regionChanceRecord      = "RGNC";
}

namespace ESM3
{
    // NOTE: equivalent to REC_GMDT
    // (called from StateManager::loadGame() via WeatherManager::readRecord())
    void WeatherState::load(Reader& esm)
    {
        while (esm.getSubRecordHeader())
        {
            const ESM3::SubRecordHeader& subHdr = esm.subRecordHeader();
            switch (subHdr.typeId)
            {
                case ESM3::SUB_CREG: esm.getZString(mCurrentRegion); break;
                case ESM3::SUB_TMPS: esm.get(mTimePassed); break;
                case ESM3::SUB_FAST: esm.get(mFastForward); break;
                case ESM3::SUB_WUPD: esm.get(mWeatherUpdateTime); break;
                case ESM3::SUB_TRFC: esm.get(mTransitionFactor); break;
                case ESM3::SUB_CWTH: esm.get(mCurrentWeather); break;
                case ESM3::SUB_NWTH: esm.get(mNextWeather); break;
                case ESM3::SUB_QWTH: esm.get(mQueuedWeather); break;
                case ESM3::SUB_RGNN:
                {
                    std::string regionID;
                    esm.getZString(regionID);

                    RegionWeatherState region;
                    esm.getSubRecordHeader(ESM3::SUB_RGNW);
                    esm.get(region.mWeather);

                    while (esm.getNextSubRecordHeader(ESM3::SUB_RGNC))
                    {
                        char chance;
                        esm.get(chance);
                        region.mChances.push_back(chance);
                    }

                    mRegions.insert(std::make_pair(regionID, region));
                    break;
                }
                default:
                    esm.cacheSubRecordHeader(); // World::readRecord() chains these...
                    return;
            }
        }
    }

    void WeatherState::save(ESM::ESMWriter& esm) const
    {
        esm.writeHNCString(currentRegionRecord, mCurrentRegion);
        esm.writeHNT(timePassedRecord, mTimePassed);
        esm.writeHNT(fastForwardRecord, mFastForward);
        esm.writeHNT(weatherUpdateTimeRecord, mWeatherUpdateTime);
        esm.writeHNT(transitionFactorRecord, mTransitionFactor);
        esm.writeHNT(currentWeatherRecord, mCurrentWeather);
        esm.writeHNT(nextWeatherRecord, mNextWeather);
        esm.writeHNT(queuedWeatherRecord, mQueuedWeather);

        std::map<std::string, RegionWeatherState>::const_iterator it = mRegions.begin();
        for(; it != mRegions.end(); ++it)
        {
            esm.writeHNCString(regionNameRecord, it->first.c_str());
            esm.writeHNT(regionWeatherRecord, it->second.mWeather);
            for(size_t i = 0; i < it->second.mChances.size(); ++i)
            {
                esm.writeHNT(regionChanceRecord, it->second.mChances[i]);
            }
        }
    }
}
