#ifndef OPENMW_ESSIMPORT_CELLREF_H
#define OPENMW_ESSIMPORT_CELLREF_H

#include <string>

#include <components/esm3/cellref.hpp>

#include "importacdt.hpp"

namespace ESM3
{
    class Reader;
}

namespace ESSImport
{

    struct CellRef : public ActorData
    {
        std::string mIndexedRefId;

        std::string mScript;

        bool mEnabled;

        bool mDeleted;

        void load(ESM3::Reader& esm) override;

        ~CellRef() override = default;
    };

}

#endif
