#include "enchanting.hpp"

#include <components/misc/rng.hpp>
#include <components/settings/settings.hpp>

#include "../mwworld/manualref.hpp"
#include "../mwworld/class.hpp"
#include "../mwworld/containerstore.hpp"
#include "../mwworld/esmstore.hpp"

#include "../mwbase/world.hpp"
#include "../mwbase/environment.hpp"
#include "../mwbase/mechanicsmanager.hpp"

#include "creaturestats.hpp"
#include "spellutil.hpp"
#include "actorutil.hpp"
#include "weapontype.hpp"

namespace MWMechanics
{
    Enchanting::Enchanting()
        : mCastStyle(ESM3::Enchantment::CastOnce)
        , mSelfEnchanting(false)
        , mWeaponType(-1)
    {}

    void Enchanting::setOldItem(const MWWorld::Ptr& oldItem)
    {
        mOldItemPtr=oldItem;
        mWeaponType = -1;
        mObjectType.clear();
        if(!itemEmpty())
        {
            mObjectType = mOldItemPtr.getTypeName();
            if (mObjectType == typeid(ESM3::Weapon).name())
                mWeaponType = mOldItemPtr.get<ESM3::Weapon>()->mBase->mData.mType;
        }
    }

    void Enchanting::setNewItemName(const std::string& s)
    {
        mNewItemName=s;
    }

    void Enchanting::setEffect(const ESM3::EffectList& effectList)
    {
        mEffectList=effectList;
    }

    int Enchanting::getCastStyle() const
    {
        return mCastStyle;
    }

    void Enchanting::setSoulGem(const MWWorld::Ptr& soulGem)
    {
        mSoulGemPtr=soulGem;
    }

    bool Enchanting::create()
    {
        const MWWorld::Ptr& player = getPlayer();
        MWWorld::ContainerStore& store = player.getClass().getContainerStore(player);
        ESM3::Enchantment enchantment;
        enchantment.mData.mFlags = 0;
        enchantment.mData.mType = mCastStyle;
        enchantment.mData.mCost = getBaseCastCost();

        store.remove(mSoulGemPtr, 1, player);

        //Exception for Azura Star, new one will be added after enchanting
        if(Misc::StringUtils::ciEqual(mSoulGemPtr.get<ESM3::Miscellaneous>()->mBase->mId, "Misc_SoulGem_Azura"))
            store.add("Misc_SoulGem_Azura", 1, player);

        if(mSelfEnchanting)
        {
            if(getEnchantChance() <= (Misc::Rng::roll0to99()))
                return false;

            mEnchanter.getClass().skillUsageSucceeded (mEnchanter, ESM3::Skill::Enchant, 2);
        }

        enchantment.mEffects = mEffectList;

        int count = getEnchantItemsCount();

        if(mCastStyle==ESM3::Enchantment::ConstantEffect)
            enchantment.mData.mCharge = 0;
        else
            enchantment.mData.mCharge = getGemCharge() / count;

        // Try to find a dynamic enchantment with the same stats, create a new one if not found.
        const ESM3::Enchantment* enchantmentPtr = getRecord(enchantment);
        if (enchantmentPtr == nullptr)
            enchantmentPtr = MWBase::Environment::get().getWorld()->createRecord (enchantment);

        // Apply the enchantment
        std::string newItemId = mOldItemPtr.getClass().applyEnchantment(mOldItemPtr, enchantmentPtr->mId, getGemCharge(), mNewItemName);

        // Add the new item to player inventory and remove the old one
        store.remove(mOldItemPtr, count, player);
        store.add(newItemId, count, player);

        if(!mSelfEnchanting)
            payForEnchantment();

        return true;
    }
    
    void Enchanting::nextCastStyle()
    {
        if (itemEmpty())
            return;

        const bool powerfulSoul = getGemCharge() >= \
                MWBase::Environment::get().getWorld()->getStore().get<ESM3::GameSetting>().find ("iSoulAmountForConstantEffect")->mValue.getInteger();
        if ((mObjectType == typeid(ESM3::Armor).name()) || (mObjectType == typeid(ESM3::Clothing).name()))
        { // Armor or Clothing
            switch(mCastStyle)
            {
                case ESM3::Enchantment::WhenUsed:
                    if (powerfulSoul)
                        mCastStyle = ESM3::Enchantment::ConstantEffect;
                    return;
                default: // takes care of Constant effect too
                    mCastStyle = ESM3::Enchantment::WhenUsed;
                    return;
            }
        }
        else if (mWeaponType != -1)
        { // Weapon
            ESM3::WeaponType::Class weapclass = MWMechanics::getWeaponType(mWeaponType)->mWeaponClass;
            switch(mCastStyle)
            {
                case ESM3::Enchantment::WhenStrikes:
                    if (weapclass == ESM3::WeaponType::Melee || weapclass == ESM3::WeaponType::Ranged)
                        mCastStyle = ESM3::Enchantment::WhenUsed;
                    return;
                case ESM3::Enchantment::WhenUsed:
                    if (powerfulSoul && weapclass != ESM3::WeaponType::Ammo && weapclass != ESM3::WeaponType::Thrown)
                        mCastStyle = ESM3::Enchantment::ConstantEffect;
                    else if (weapclass != ESM3::WeaponType::Ranged)
                        mCastStyle = ESM3::Enchantment::WhenStrikes;
                    return;
                default: // takes care of Constant effect too
                    mCastStyle = ESM3::Enchantment::WhenUsed;
                    if (weapclass != ESM3::WeaponType::Ranged)
                        mCastStyle = ESM3::Enchantment::WhenStrikes;
                    return;
            }
        }
        else if(mObjectType == typeid(ESM3::Book).name())
        { // Scroll or Book
            mCastStyle = ESM3::Enchantment::CastOnce;
            return;
        }

        // Fail case
        mCastStyle = ESM3::Enchantment::CastOnce;
    }

    /*
     * Vanilla enchant cost formula:
     *
     *  Touch/Self:          (min + max) * baseCost * 0.025 * duration + area * baseCost * 0.025
     *  Target:       1.5 * ((min + max) * baseCost * 0.025 * duration + area * baseCost * 0.025)
     *  Constant eff:        (min + max) * baseCost * 2.5              + area * baseCost * 0.025
     *
     *  For multiple effects - cost of each effect is multiplied by number of effects that follows +1.
     *
     *  Note: Minimal value inside formula for 'min' and 'max' is 1. So in vanilla:
     *        (0 + 0) == (1 + 0) == (1 + 1) => 2 or (2 + 0) == (1 + 2) => 3
     *
     *  Formula on UESPWiki is not entirely correct.
     */
    float Enchanting::getEnchantPoints(bool precise) const
    {
        if (mEffectList.mList.empty())
            // No effects added, cost = 0
            return 0;

        const MWWorld::ESMStore &store = MWBase::Environment::get().getWorld()->getStore();
        const float fEffectCostMult = store.get<ESM3::GameSetting>().find("fEffectCostMult")->mValue.getFloat();
        const float fEnchantmentConstantDurationMult = store.get<ESM3::GameSetting>().find("fEnchantmentConstantDurationMult")->mValue.getFloat();

        float enchantmentCost = 0.f;
        float cost = 0.f;
        for (const ESM3::ENAMstruct& effect : mEffectList.mList)
        {
            float baseCost = (store.get<ESM3::MagicEffect>().find(effect.mEffectID))->mData.mBaseCost;
            int magMin = std::max(1, (int)effect.mMagnMin); // NOTE: mMagnMin used to be int
            int magMax = std::max(1, (int)effect.mMagnMax); // NOTE: mMagnMax used to be int
            int area = std::max(1, (int)effect.mArea);      // NOTE: mArea used to be int
            float duration = static_cast<float>(effect.mDuration);
            if (mCastStyle == ESM3::Enchantment::ConstantEffect)
                duration = fEnchantmentConstantDurationMult;

            cost += ((magMin + magMax) * duration + area) * baseCost * fEffectCostMult * 0.05f;

            cost = std::max(1.f, cost);

            if (effect.mRange == ESM::RT_Target)
                cost *= 1.5f;

            enchantmentCost += precise ? cost : std::floor(cost);
        }

        return enchantmentCost;
    }

    const ESM3::Enchantment* Enchanting::getRecord(const ESM3::Enchantment& toFind) const
    {
        const MWWorld::Store<ESM3::Enchantment>& enchantments = MWBase::Environment::get().getWorld()->getStore().get<ESM3::Enchantment>();
        MWWorld::Store<ESM3::Enchantment>::iterator iter (enchantments.begin());
        iter += (enchantments.getSize() - enchantments.getDynamicSize());
        for (; iter != enchantments.end(); ++iter)
        {
            if (iter->mEffects.mList.size() != toFind.mEffects.mList.size())
                continue;

            if (iter->mData.mFlags != toFind.mData.mFlags
                    || iter->mData.mType != toFind.mData.mType
                    || iter->mData.mCost != toFind.mData.mCost
                    || iter->mData.mCharge != toFind.mData.mCharge)
                continue;

            // Don't choose an ID that came from the content files, would have unintended side effects
            if (!enchantments.isDynamic(iter->mId))
                continue;

            bool mismatch = false;

            for (int i=0; i<static_cast<int> (iter->mEffects.mList.size()); ++i)
            {
                const ESM3::ENAMstruct& first = iter->mEffects.mList[i];
                const ESM3::ENAMstruct& second = toFind.mEffects.mList[i];

                if (first.mEffectID!=second.mEffectID ||
                    first.mArea!=second.mArea ||
                    first.mRange!=second.mRange ||
                    first.mSkill!=second.mSkill ||
                    first.mAttribute!=second.mAttribute ||
                    first.mMagnMin!=second.mMagnMin ||
                    first.mMagnMax!=second.mMagnMax ||
                    first.mDuration!=second.mDuration)
                {
                    mismatch = true;
                    break;
                }
            }

            if (!mismatch)
                return &(*iter);
        }

        return nullptr;
    }

    int Enchanting::getBaseCastCost() const
    {
        if (mCastStyle == ESM3::Enchantment::ConstantEffect)
            return 0;

        return static_cast<int>(getEnchantPoints(false));
    }

    int Enchanting::getEffectiveCastCost() const
    {
        int baseCost = getBaseCastCost();
        MWWorld::Ptr player = getPlayer();
        return getEffectiveEnchantmentCastCost(static_cast<float>(baseCost), player);
    }


    int Enchanting::getEnchantPrice() const
    {
        if(mEnchanter.isEmpty())
            return 0;

        float priceMultipler = MWBase::Environment::get().getWorld()->getStore().get<ESM3::GameSetting>().find ("fEnchantmentValueMult")->mValue.getFloat();
        int price = MWBase::Environment::get().getMechanicsManager()->getBarterOffer(mEnchanter, static_cast<int>(getEnchantPoints() * priceMultipler), true);
        price *= getEnchantItemsCount() * getTypeMultiplier();
        return std::max(1, price);
    }

    int Enchanting::getGemCharge() const
    {
        const MWWorld::ESMStore &store = MWBase::Environment::get().getWorld()->getStore();
        if(soulEmpty())
            return 0;
        if(mSoulGemPtr.getCellRef().getSoul()=="")
            return 0;
        const ESM3::Creature* soul = store.get<ESM3::Creature>().search(mSoulGemPtr.getCellRef().getSoul());
        if(soul)
            return soul->mData.mSoul;
        else
            return 0;
    }

    int Enchanting::getMaxEnchantValue() const
    {
        if (itemEmpty())
            return 0;

        const MWWorld::ESMStore &store = MWBase::Environment::get().getWorld()->getStore();

        return static_cast<int>(mOldItemPtr.getClass().getEnchantmentPoints(mOldItemPtr) * store.get<ESM3::GameSetting>().find("fEnchantmentMult")->mValue.getFloat());
    }
    bool Enchanting::soulEmpty() const
    {
        return mSoulGemPtr.isEmpty();
    }

    bool Enchanting::itemEmpty() const
    {
        return mOldItemPtr.isEmpty();
    }

    void Enchanting::setSelfEnchanting(bool selfEnchanting)
    {
        mSelfEnchanting = selfEnchanting;
    }

    void Enchanting::setEnchanter(const MWWorld::Ptr& enchanter)
    {
        mEnchanter = enchanter;
        // Reset cast style
        mCastStyle = ESM3::Enchantment::CastOnce;
    }

    int Enchanting::getEnchantChance() const
    {
        const CreatureStats& stats = mEnchanter.getClass().getCreatureStats(mEnchanter);
        const MWWorld::Store<ESM3::GameSetting>& gmst = MWBase::Environment::get().getWorld()->getStore().get<ESM3::GameSetting>();
        const float a = static_cast<float>(mEnchanter.getClass().getSkill(mEnchanter, ESM3::Skill::Enchant));
        const float b = static_cast<float>(stats.getAttribute (ESM::Attribute::Intelligence).getModified());
        const float c = static_cast<float>(stats.getAttribute (ESM::Attribute::Luck).getModified());
        const float fEnchantmentChanceMult = gmst.find("fEnchantmentChanceMult")->mValue.getFloat();
        const float fEnchantmentConstantChanceMult = gmst.find("fEnchantmentConstantChanceMult")->mValue.getFloat();

        float x = (a - getEnchantPoints() * fEnchantmentChanceMult * getTypeMultiplier() * getEnchantItemsCount() + 0.2f * b + 0.1f * c) * stats.getFatigueTerm();
        if (mCastStyle == ESM3::Enchantment::ConstantEffect)
            x *= fEnchantmentConstantChanceMult;

        return static_cast<int>(x);
    }

    int Enchanting::getEnchantItemsCount() const
    {
        int count = 1;
        float enchantPoints = getEnchantPoints();
        if (mWeaponType != -1 && enchantPoints > 0)
        {
            ESM3::WeaponType::Class weapclass = MWMechanics::getWeaponType(mWeaponType)->mWeaponClass;
            if (weapclass == ESM3::WeaponType::Thrown || weapclass == ESM3::WeaponType::Ammo)
            {
                static const float multiplier = std::max(0.f, std::min(1.0f, Settings::Manager::getFloat("projectiles enchant multiplier", "Game")));
                MWWorld::Ptr player = getPlayer();
                int itemsInInventoryCount = player.getClass().getContainerStore(player).count(mOldItemPtr.getCellRef().getRefId());
                count = std::min(itemsInInventoryCount, std::max(1, int(getGemCharge() * multiplier / enchantPoints)));
            }
        }

        return count;
    }

    float Enchanting::getTypeMultiplier() const
    {
        static const bool useMultiplier = Settings::Manager::getFloat("projectiles enchant multiplier", "Game") > 0;
        if (useMultiplier && mWeaponType != -1 && getEnchantPoints() > 0)
        {
            ESM3::WeaponType::Class weapclass = MWMechanics::getWeaponType(mWeaponType)->mWeaponClass;
            if (weapclass == ESM3::WeaponType::Thrown || weapclass == ESM3::WeaponType::Ammo)
                return 0.125f;
        }

        return 1.f;
    }

    void Enchanting::payForEnchantment() const
    {
        const MWWorld::Ptr& player = getPlayer();
        MWWorld::ContainerStore& store = player.getClass().getContainerStore(player);

        store.remove(MWWorld::ContainerStore::sGoldId, getEnchantPrice(), player);

        // add gold to NPC trading gold pool
        CreatureStats& enchanterStats = mEnchanter.getClass().getCreatureStats(mEnchanter);
        enchanterStats.setGoldPool(enchanterStats.getGoldPool() + getEnchantPrice());
    }
}
