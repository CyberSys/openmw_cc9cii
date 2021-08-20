#ifndef CSM_TOOLS_ENCHANTMENTCHECK_H
#define CSM_TOOLS_ENCHANTMENTCHECK_H

#include <components/esm3/ench.hpp>

#include "../world/idcollection.hpp"

#include "../doc/stage.hpp"

namespace CSMTools
{
    /// \brief Make sure that enchantment records are correct
    class EnchantmentCheckStage : public CSMDoc::Stage
    {
            const CSMWorld::IdCollection<ESM3::Enchantment>& mEnchantments;
            bool mIgnoreBaseRecords;

        public:

            EnchantmentCheckStage (const CSMWorld::IdCollection<ESM3::Enchantment>& enchantments);

            int setup() override;
            ///< \return number of steps

            void perform (int stage, CSMDoc::Messages& messages) override;
            ///< Messages resulting from this tage will be appended to \a messages.

    };
}

#endif
