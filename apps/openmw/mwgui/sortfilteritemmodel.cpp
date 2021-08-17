#include "sortfilteritemmodel.hpp"

#include <components/misc/stringops.hpp>
#include <components/debug/debuglog.hpp>
#include <components/esm3/alch.hpp>
#include <components/esm3/appa.hpp>
#include <components/esm3/armo.hpp>
#include <components/esm3/book.hpp>
#include <components/esm3/clot.hpp>
#include <components/esm3/ingr.hpp>
#include <components/esm3/lock.hpp>
#include <components/esm3/ligh.hpp>
#include <components/esm3/misc.hpp>
#include <components/esm3/prob.hpp>
#include <components/esm3/repa.hpp>
#include <components/esm3/weap.hpp>
#include <components/esm3/ench.hpp>

#include "../mwbase/environment.hpp"
#include "../mwbase/world.hpp"

#include "../mwworld/class.hpp"
#include "../mwworld/nullaction.hpp"
#include "../mwworld/esmstore.hpp"

#include "../mwmechanics/alchemy.hpp"

namespace
{
    bool compareType(const std::string& type1, const std::string& type2)
    {
        // this defines the sorting order of types. types that are first in the vector appear before other types.
        std::vector<std::string> mapping;
        mapping.emplace_back(typeid(ESM3::Weapon).name() );
        mapping.emplace_back(typeid(ESM3::Armor).name() );
        mapping.emplace_back(typeid(ESM3::Clothing).name() );
        mapping.emplace_back(typeid(ESM3::Potion).name() );
        mapping.emplace_back(typeid(ESM3::Ingredient).name() );
        mapping.emplace_back(typeid(ESM3::Apparatus).name() );
        mapping.emplace_back(typeid(ESM3::Book).name() );
        mapping.emplace_back(typeid(ESM3::Light).name() );
        mapping.emplace_back(typeid(ESM3::Miscellaneous).name() );
        mapping.emplace_back(typeid(ESM3::Lockpick).name() );
        mapping.emplace_back(typeid(ESM3::Repair).name() );
        mapping.emplace_back(typeid(ESM3::Probe).name() );

        assert( std::find(mapping.begin(), mapping.end(), type1) != mapping.end() );
        assert( std::find(mapping.begin(), mapping.end(), type2) != mapping.end() );

        return std::find(mapping.begin(), mapping.end(), type1) < std::find(mapping.begin(), mapping.end(), type2);
    }

    struct Compare
    {
        bool mSortByType;
        Compare() : mSortByType(true) {}
        bool operator() (const MWGui::ItemStack& left, const MWGui::ItemStack& right)
        {
            if (mSortByType && left.mType != right.mType)
                return left.mType < right.mType;

            float result = 0;

            // compare items by type
            std::string leftName = left.mBase.getTypeName();
            std::string rightName = right.mBase.getTypeName();

            if (leftName != rightName)
                return compareType(leftName, rightName);

            // compare items by name
            leftName = Misc::StringUtils::lowerCaseUtf8(left.mBase.getClass().getName(left.mBase));
            rightName = Misc::StringUtils::lowerCaseUtf8(right.mBase.getClass().getName(right.mBase));

            result = leftName.compare(rightName);
            if (result != 0)
                return result < 0;

            // compare items by enchantment:
            // 1. enchanted items showed before non-enchanted
            // 2. item with lesser charge percent comes after items with more charge percent
            // 3. item with constant effect comes before items with non-constant effects
            int leftChargePercent = -1;
            int rightChargePercent = -1;
            leftName = left.mBase.getClass().getEnchantment(left.mBase);
            rightName = right.mBase.getClass().getEnchantment(right.mBase);

            if (!leftName.empty())
            {
                const ESM3::Enchantment* ench = MWBase::Environment::get().getWorld()->getStore().get<ESM3::Enchantment>().search(leftName);
                if (ench)
                {
                    if (ench->mData.mType == ESM3::Enchantment::ConstantEffect)
                        leftChargePercent = 101;
                    else
                        leftChargePercent = static_cast<int>(left.mBase.getCellRef().getNormalizedEnchantmentCharge(ench->mData.mCharge) * 100);
                }
            }

            if (!rightName.empty())
            {
                const ESM3::Enchantment* ench = MWBase::Environment::get().getWorld()->getStore().get<ESM3::Enchantment>().search(rightName);
                if (ench)
                {
                    if (ench->mData.mType == ESM3::Enchantment::ConstantEffect)
                        rightChargePercent = 101;
                    else
                        rightChargePercent = static_cast<int>(right.mBase.getCellRef().getNormalizedEnchantmentCharge(ench->mData.mCharge) * 100);
                }
            }

            result = leftChargePercent - rightChargePercent;
            if (result != 0)
                return result > 0;

            // compare items by condition
            if (left.mBase.getClass().hasItemHealth(left.mBase) && right.mBase.getClass().hasItemHealth(right.mBase))
            {
                result = left.mBase.getClass().getItemHealth(left.mBase) - right.mBase.getClass().getItemHealth(right.mBase);
                if (result != 0)
                    return result > 0;
            }

            // compare items by remaining usage time
            result = left.mBase.getClass().getRemainingUsageTime(left.mBase) - right.mBase.getClass().getRemainingUsageTime(right.mBase);
            if (result != 0)
                return result > 0;

            // compare items by value
            result = left.mBase.getClass().getValue(left.mBase) - right.mBase.getClass().getValue(right.mBase);
            if (result != 0)
                return result > 0;

            // compare items by weight
            result = left.mBase.getClass().getWeight(left.mBase) - right.mBase.getClass().getWeight(right.mBase);
            if (result != 0)
                return result > 0;

            // compare items by Id
            leftName = left.mBase.getCellRef().getRefId();
            rightName = right.mBase.getCellRef().getRefId();

            result = leftName.compare(rightName);
            return result < 0;
        }
    };
}

namespace MWGui
{

    SortFilterItemModel::SortFilterItemModel(ItemModel *sourceModel)
        : mCategory(Category_All)
        , mFilter(0)
        , mSortByType(true)
        , mNameFilter("")
        , mEffectFilter("")
    {
        mSourceModel = sourceModel;
    }

    bool SortFilterItemModel::allowedToUseItems() const
    {
        return mSourceModel->allowedToUseItems();
    }

    void SortFilterItemModel::addDragItem (const MWWorld::Ptr& dragItem, size_t count)
    {
        mDragItems.emplace_back(dragItem, count);
    }

    void SortFilterItemModel::clearDragItems()
    {
        mDragItems.clear();
    }

    bool SortFilterItemModel::filterAccepts (const ItemStack& item)
    {
        MWWorld::Ptr base = item.mBase;

        int category = 0;
        if (base.getTypeName() == typeid(ESM3::Armor).name()
                || base.getTypeName() == typeid(ESM3::Clothing).name())
            category = Category_Apparel;
        else if (base.getTypeName() == typeid(ESM3::Weapon).name())
            category = Category_Weapon;
        else if (base.getTypeName() == typeid(ESM3::Ingredient).name()
                     || base.getTypeName() == typeid(ESM3::Potion).name())
            category = Category_Magic;
        else if (base.getTypeName() == typeid(ESM3::Miscellaneous).name()
                 || base.getTypeName() == typeid(ESM3::Ingredient).name()
                 || base.getTypeName() == typeid(ESM3::Repair).name()
                 || base.getTypeName() == typeid(ESM3::Lockpick).name()
                 || base.getTypeName() == typeid(ESM3::Light).name()
                 || base.getTypeName() == typeid(ESM3::Apparatus).name()
                 || base.getTypeName() == typeid(ESM3::Book).name()
                 || base.getTypeName() == typeid(ESM3::Probe).name())
            category = Category_Misc;

        if (item.mFlags & ItemStack::Flag_Enchanted)
            category |= Category_Magic;

        if (!(category & mCategory))
            return false;

        if (mFilter & Filter_OnlyIngredients)
        {
            if (base.getTypeName() != typeid(ESM3::Ingredient).name())
                return false;

            if (!mNameFilter.empty() && !mEffectFilter.empty())
                throw std::logic_error("name and magic effect filter are mutually exclusive");

            if (!mNameFilter.empty())
            {
                const auto itemName = Misc::StringUtils::lowerCaseUtf8(base.getClass().getName(base));
                return itemName.find(mNameFilter) != std::string::npos;
            }

            if (!mEffectFilter.empty())
            {
                MWWorld::Ptr player = MWBase::Environment::get().getWorld ()->getPlayerPtr();
                const auto alchemySkill = player.getClass().getSkill(player, ESM3::Skill::Alchemy);

                const auto effects = MWMechanics::Alchemy::effectsDescription(base, alchemySkill);

                for (const auto& effect : effects)
                {
                    const auto ciEffect = Misc::StringUtils::lowerCaseUtf8(effect);

                    if (ciEffect.find(mEffectFilter) != std::string::npos)
                        return true;
                }
                return false;
            }
            return true;
        }

        if ((mFilter & Filter_OnlyEnchanted) && !(item.mFlags & ItemStack::Flag_Enchanted))
            return false;
        if ((mFilter & Filter_OnlyChargedSoulstones) && (base.getTypeName() != typeid(ESM3::Miscellaneous).name()
                                                     || base.getCellRef().getSoul() == "" || !MWBase::Environment::get().getWorld()->getStore().get<ESM3::Creature>().search(base.getCellRef().getSoul())))
            return false;
        if ((mFilter & Filter_OnlyRepairTools) && (base.getTypeName() != typeid(ESM3::Repair).name()))
            return false;
        if ((mFilter & Filter_OnlyEnchantable) && (item.mFlags & ItemStack::Flag_Enchanted
                                               || (base.getTypeName() != typeid(ESM3::Armor).name()
                                                   && base.getTypeName() != typeid(ESM3::Clothing).name()
                                                   && base.getTypeName() != typeid(ESM3::Weapon).name()
                                                   && base.getTypeName() != typeid(ESM3::Book).name())))
            return false;
        if ((mFilter & Filter_OnlyEnchantable) && base.getTypeName() == typeid(ESM3::Book).name()
                && !base.get<ESM3::Book>()->mBase->mData.mIsScroll)
            return false;

        if ((mFilter & Filter_OnlyUsableItems) && base.getClass().getScript(base).empty())
        {
            std::shared_ptr<MWWorld::Action> actionOnUse = base.getClass().use(base);
            if (!actionOnUse || actionOnUse->isNullAction())
                return false;
        }

        if ((mFilter & Filter_OnlyRepairable) && (
                    !base.getClass().hasItemHealth(base)
                    || (base.getClass().getItemHealth(base) == base.getClass().getItemMaxHealth(base))
                    || (base.getTypeName() != typeid(ESM3::Weapon).name()
                        && base.getTypeName() != typeid(ESM3::Armor).name())))
            return false;

        if (mFilter & Filter_OnlyRechargable)
        {
            if (!(item.mFlags & ItemStack::Flag_Enchanted))
                return false;

            std::string enchId = base.getClass().getEnchantment(base);
            const ESM3::Enchantment* ench = MWBase::Environment::get().getWorld()->getStore().get<ESM3::Enchantment>().search(enchId);
            if (!ench)
            {
                Log(Debug::Warning) << "Warning: Can't find enchantment '" << enchId << "' on item " << base.getCellRef().getRefId();
                return false;
            }

            if (base.getCellRef().getEnchantmentCharge() >= ench->mData.mCharge
                    || base.getCellRef().getEnchantmentCharge() == -1)
                return false;
        }

        std::string compare = Misc::StringUtils::lowerCaseUtf8(item.mBase.getClass().getName(item.mBase));
        if(compare.find(mNameFilter) == std::string::npos)
            return false;

        return true;
    }

    ItemStack SortFilterItemModel::getItem (ModelIndex index)
    {
        if (index < 0)
            throw std::runtime_error("Invalid index supplied");
        if (mItems.size() <= static_cast<size_t>(index))
            throw std::runtime_error("Item index out of range");
        return mItems[index];
    }

    size_t SortFilterItemModel::getItemCount()
    {
        return mItems.size();
    }

    void SortFilterItemModel::setCategory (int category)
    {
        mCategory = category;
    }

    void SortFilterItemModel::setFilter (int filter)
    {
        mFilter = filter;
    }

    void SortFilterItemModel::setNameFilter (const std::string& filter)
    {
        mNameFilter = Misc::StringUtils::lowerCaseUtf8(filter);
    }

    void SortFilterItemModel::setEffectFilter (const std::string& filter)
    {
        mEffectFilter = Misc::StringUtils::lowerCaseUtf8(filter);
    }

    void SortFilterItemModel::update()
    {
        mSourceModel->update();

        size_t count = mSourceModel->getItemCount();

        mItems.clear();
        for (size_t i=0; i<count; ++i)
        {
            ItemStack item = mSourceModel->getItem(i);

            for (std::vector<std::pair<MWWorld::Ptr, size_t> >::iterator it = mDragItems.begin(); it != mDragItems.end(); ++it)
            {
                if (item.mBase == it->first)
                {
                    if (item.mCount < it->second)
                        throw std::runtime_error("Dragging more than present in the model");
                    item.mCount -= it->second;
                }
            }

            if (item.mCount > 0 && filterAccepts(item))
                mItems.push_back(item);
        }

        Compare cmp;
        cmp.mSortByType = mSortByType;
        std::sort(mItems.begin(), mItems.end(), cmp);
    }

    void SortFilterItemModel::onClose()
    {
        mSourceModel->onClose();
    }

    bool SortFilterItemModel::onDropItem(const MWWorld::Ptr &item, int count)
    {
        return mSourceModel->onDropItem(item, count);
    }

    bool SortFilterItemModel::onTakeItem(const MWWorld::Ptr &item, int count)
    {
        return mSourceModel->onTakeItem(item, count);
    }
}
