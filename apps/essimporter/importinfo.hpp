#ifndef OPENMW_ESSIMPORT_IMPORTINFO_H
#define OPENMW_ESSIMPORT_IMPORTINFO_H

#include <string>

namespace ESM3
{
    class Reader;
}

namespace ESSImport
{

    struct INFO
    {
        std::string mInfo;
        std::string mActorRefId;

        void load(ESM3::Reader& esm);
    };

}

#endif
