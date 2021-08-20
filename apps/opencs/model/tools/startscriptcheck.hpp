#ifndef CSM_TOOLS_STARTSCRIPTCHECK_H
#define CSM_TOOLS_STARTSCRIPTCHECK_H

#include <components/esm3/sscr.hpp>
#include <components/esm3/scpt.hpp>

#include "../doc/stage.hpp"

#include "../world/idcollection.hpp"

namespace CSMTools
{
    class StartScriptCheckStage : public CSMDoc::Stage
    {
            const CSMWorld::IdCollection<ESM3::StartScript>& mStartScripts;
            const CSMWorld::IdCollection<ESM3::Script>& mScripts;
            bool mIgnoreBaseRecords;

        public:

            StartScriptCheckStage (const CSMWorld::IdCollection<ESM3::StartScript>& startScripts,
                const CSMWorld::IdCollection<ESM3::Script>& scripts);

            void perform(int stage, CSMDoc::Messages& messages) override;
            int setup() override;
    };
}

#endif
