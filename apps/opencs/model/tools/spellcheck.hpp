#ifndef CSM_TOOLS_SPELLCHECK_H
#define CSM_TOOLS_SPELLCHECK_H

#include <components/esm3/spel.hpp>

#include "../world/idcollection.hpp"

#include "../doc/stage.hpp"

namespace CSMTools
{
    /// \brief VerifyStage: make sure that spell records are internally consistent
    class SpellCheckStage : public CSMDoc::Stage
    {
            const CSMWorld::IdCollection<ESM3::Spell>& mSpells;
            bool mIgnoreBaseRecords;

        public:

            SpellCheckStage (const CSMWorld::IdCollection<ESM3::Spell>& spells);

            int setup() override;
            ///< \return number of steps

            void perform (int stage, CSMDoc::Messages& messages) override;
            ///< Messages resulting from this tage will be appended to \a messages.
    };
}

#endif
