#ifndef OPENMW_ESSIMPORT_KLST_H
#define OPENMW_ESSIMPORT_KLST_H

#include <string>
#include <map>

namespace ESM3
{
    class Reader;
}

namespace ESSImport
{

    /// Kill Stats
    struct KLST
    {
        void load(ESM3::Reader& esm);

        /// RefId, kill count
        std::map<std::string, int> mKillCounter;

        int mWerewolfKills;
    };

}

#endif
