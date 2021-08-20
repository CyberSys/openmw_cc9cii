#include "enchantmentcheck.hpp"

#include "../prefs/state.hpp"

#include "../world/universalid.hpp"

CSMTools::EnchantmentCheckStage::EnchantmentCheckStage (const CSMWorld::IdCollection<ESM3::Enchantment>& enchantments)
    : mEnchantments (enchantments)
{
    mIgnoreBaseRecords = false;
}

int CSMTools::EnchantmentCheckStage::setup()
{
    mIgnoreBaseRecords = CSMPrefs::get()["Reports"]["ignore-base-records"].isTrue();

    return mEnchantments.getSize();
}

void CSMTools::EnchantmentCheckStage::perform (int stage, CSMDoc::Messages& messages)
{
    const CSMWorld::Record<ESM3::Enchantment>& record = mEnchantments.getRecord (stage);

    // Skip "Base" records (setting!) and "Deleted" records
    if ((mIgnoreBaseRecords && record.mState == CSMWorld::RecordBase::State_BaseOnly) || record.isDeleted())
        return;

    const ESM3::Enchantment& enchantment = record.get();

    CSMWorld::UniversalId id (CSMWorld::UniversalId::Type_Enchantment, enchantment.mId);

    if (enchantment.mData.mType > ESM3::Enchantment::ConstantEffect)
        messages.add(id, "Invalid type", "", CSMDoc::Message::Severity_Error);

    // cost and charge are 32-bit unsigned and hence cannot be negative
#if 0
    if (enchantment.mData.mCost < 0)
        messages.add(id, "Cost is negative", "", CSMDoc::Message::Severity_Error);

    if (enchantment.mData.mCharge < 0)
        messages.add(id, "Charge is negative", "", CSMDoc::Message::Severity_Error);
#endif
    if (enchantment.mData.mCost > enchantment.mData.mCharge)
        messages.add(id, "Cost is higher than charge", "", CSMDoc::Message::Severity_Error);

    if (enchantment.mEffects.mList.empty())
    {
        messages.add(id, "Enchantment doesn't have any magic effects", "", CSMDoc::Message::Severity_Warning);
    }
    else
    {
        std::vector<ESM3::ENAMstruct>::const_iterator effect = enchantment.mEffects.mList.begin();

        for (size_t i = 1; i <= enchantment.mEffects.mList.size(); i++)
        {
            const std::string number = std::to_string(i);
            // At the time of writing this effects, attributes and skills are hardcoded
            if (effect->mEffectID < 0 || effect->mEffectID > 142)
            {
                messages.add(id, "Effect #" + number + " is invalid", "", CSMDoc::Message::Severity_Error);
                ++effect;
                continue;
            }

            if (effect->mSkill < -1 || effect->mSkill > 26)
                messages.add(id, "Effect #" + number + " affected skill is invalid", "", CSMDoc::Message::Severity_Error);
            if (effect->mAttribute < -1 || effect->mAttribute > 7)
                messages.add(id, "Effect #" + number + " affected attribute is invalid", "", CSMDoc::Message::Severity_Error);
            // Range, Area, Duration, Min/Max Magnitude are 32-bin unsigned
            if (effect->mRange > 2)
                messages.add(id, "Effect #" + number + " range is invalid", "", CSMDoc::Message::Severity_Error);
#if 0
            if (effect->mArea < 0)
                messages.add(id, "Effect #" + number + " area is negative", "", CSMDoc::Message::Severity_Error);
            if (effect->mDuration < 0)
                messages.add(id, "Effect #" + number + " duration is negative", "", CSMDoc::Message::Severity_Error);
            if (effect->mMagnMin < 0)
                messages.add(id, "Effect #" + number + " minimum magnitude is negative", "", CSMDoc::Message::Severity_Error);
            if (effect->mMagnMax < 0)
                messages.add(id, "Effect #" + number + " maximum magnitude is negative", "", CSMDoc::Message::Severity_Error);
#endif
            if (effect->mMagnMin > effect->mMagnMax)
                messages.add(id, "Effect #" + number + " minimum magnitude is higher than maximum magnitude", "", CSMDoc::Message::Severity_Error);
            ++effect;
        }
    }
}
