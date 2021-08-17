#include "armor.hpp"

#include <components/esm3/armo.hpp>
#include <components/esm3/skil.hpp>
#include <components/esm3/gmst.hpp>

#include "../mwbase/environment.hpp"
#include "../mwbase/world.hpp"
#include "../mwbase/windowmanager.hpp"

#include "../mwworld/ptr.hpp"
#include "../mwworld/actionequip.hpp"
#include "../mwworld/inventorystore.hpp"
#include "../mwworld/cellstore.hpp"
#include "../mwworld/esmstore.hpp"
#include "../mwphysics/physicssystem.hpp"
#include "../mwworld/nullaction.hpp"
#include "../mwworld/containerstore.hpp"

#include "../mwrender/objects.hpp"
#include "../mwrender/renderinginterface.hpp"
#include "../mwmechanics/actorutil.hpp"
#include "../mwmechanics/weapontype.hpp"

#include "../mwgui/tooltips.hpp"

namespace MWClass
{

    void Armor::insertObjectRendering (const MWWorld::Ptr& ptr, const std::string& model, MWRender::RenderingInterface& renderingInterface) const
    {
        if (!model.empty()) {
            renderingInterface.getObjects().insertModel(ptr, model);
        }
    }

    std::string Armor::getModel(const MWWorld::ConstPtr &ptr) const
    {
        const MWWorld::LiveCellRef<ESM3::Armor> *ref = ptr.get<ESM3::Armor>();

        const std::string &model = ref->mBase->mModel;
        if (!model.empty()) {
            return "meshes\\" + model;
        }
        return "";
    }

    std::string Armor::getName (const MWWorld::ConstPtr& ptr) const
    {
        const MWWorld::LiveCellRef<ESM3::Armor> *ref = ptr.get<ESM3::Armor>();
        const std::string& name = ref->mBase->mName;

        return !name.empty() ? name : ref->mBase->mId;
    }

    std::shared_ptr<MWWorld::Action> Armor::activate (const MWWorld::Ptr& ptr,
        const MWWorld::Ptr& actor) const
    {
        return defaultItemActivate(ptr, actor);
    }

    bool Armor::hasItemHealth (const MWWorld::ConstPtr& ptr) const
    {
        return true;
    }

    int Armor::getItemMaxHealth (const MWWorld::ConstPtr& ptr) const
    {
        const MWWorld::LiveCellRef<ESM3::Armor> *ref = ptr.get<ESM3::Armor>();

        return ref->mBase->mData.mHealth;
    }

    std::string Armor::getScript (const MWWorld::ConstPtr& ptr) const
    {
        const MWWorld::LiveCellRef<ESM3::Armor> *ref = ptr.get<ESM3::Armor>();

        return ref->mBase->mScript;
    }

    std::pair<std::vector<int>, bool> Armor::getEquipmentSlots (const MWWorld::ConstPtr& ptr) const
    {
        const MWWorld::LiveCellRef<ESM3::Armor> *ref = ptr.get<ESM3::Armor>();

        std::vector<int> slots_;

        const int size = 11;

        static const int sMapping[size][2] =
        {
            { ESM3::Armor::Helmet, MWWorld::InventoryStore::Slot_Helmet },
            { ESM3::Armor::Cuirass, MWWorld::InventoryStore::Slot_Cuirass },
            { ESM3::Armor::LPauldron, MWWorld::InventoryStore::Slot_LeftPauldron },
            { ESM3::Armor::RPauldron, MWWorld::InventoryStore::Slot_RightPauldron },
            { ESM3::Armor::Greaves, MWWorld::InventoryStore::Slot_Greaves },
            { ESM3::Armor::Boots, MWWorld::InventoryStore::Slot_Boots },
            { ESM3::Armor::LGauntlet, MWWorld::InventoryStore::Slot_LeftGauntlet },
            { ESM3::Armor::RGauntlet, MWWorld::InventoryStore::Slot_RightGauntlet },
            { ESM3::Armor::Shield, MWWorld::InventoryStore::Slot_CarriedLeft },
            { ESM3::Armor::LBracer, MWWorld::InventoryStore::Slot_LeftGauntlet },
            { ESM3::Armor::RBracer, MWWorld::InventoryStore::Slot_RightGauntlet }
        };

        for (int i=0; i<size; ++i)
            if (sMapping[i][0]==ref->mBase->mData.mType)
            {
                slots_.push_back (int (sMapping[i][1]));
                break;
            }

        return std::make_pair (slots_, false);
    }

    int Armor::getEquipmentSkill (const MWWorld::ConstPtr& ptr) const
    {
        const MWWorld::LiveCellRef<ESM3::Armor> *ref = ptr.get<ESM3::Armor>();

        std::string typeGmst;

        switch (ref->mBase->mData.mType)
        {
            case ESM3::Armor::Helmet: typeGmst = "iHelmWeight"; break;
            case ESM3::Armor::Cuirass: typeGmst = "iCuirassWeight"; break;
            case ESM3::Armor::LPauldron:
            case ESM3::Armor::RPauldron: typeGmst = "iPauldronWeight"; break;
            case ESM3::Armor::Greaves: typeGmst = "iGreavesWeight"; break;
            case ESM3::Armor::Boots: typeGmst = "iBootsWeight"; break;
            case ESM3::Armor::LGauntlet:
            case ESM3::Armor::RGauntlet: typeGmst = "iGauntletWeight"; break;
            case ESM3::Armor::Shield: typeGmst = "iShieldWeight"; break;
            case ESM3::Armor::LBracer:
            case ESM3::Armor::RBracer: typeGmst = "iGauntletWeight"; break;
        }

        if (typeGmst.empty())
            return -1;

        const MWWorld::Store<ESM3::GameSetting> &gmst =
            MWBase::Environment::get().getWorld()->getStore().get<ESM3::GameSetting>();

        float iWeight = floor(gmst.find(typeGmst)->mValue.getFloat());

        float epsilon = 0.0005f;

        if (ref->mBase->mData.mWeight <= iWeight * gmst.find ("fLightMaxMod")->mValue.getFloat() + epsilon)
            return ESM3::Skill::LightArmor;

        if (ref->mBase->mData.mWeight <= iWeight * gmst.find ("fMedMaxMod")->mValue.getFloat() + epsilon)
            return ESM3::Skill::MediumArmor;

        else
            return ESM3::Skill::HeavyArmor;
    }

    int Armor::getValue (const MWWorld::ConstPtr& ptr) const
    {
        const MWWorld::LiveCellRef<ESM3::Armor> *ref = ptr.get<ESM3::Armor>();

        return ref->mBase->mData.mValue;
    }

    void Armor::registerSelf()
    {
        std::shared_ptr<Class> instance (new Armor);

        registerClass (typeid (ESM3::Armor).name(), instance);
    }

    std::string Armor::getUpSoundId (const MWWorld::ConstPtr& ptr) const
    {
        int es = getEquipmentSkill(ptr);
        if (es == ESM3::Skill::LightArmor)
            return std::string("Item Armor Light Up");
        else if (es == ESM3::Skill::MediumArmor)
            return std::string("Item Armor Medium Up");
        else
            return std::string("Item Armor Heavy Up");
    }

    std::string Armor::getDownSoundId (const MWWorld::ConstPtr& ptr) const
    {
        int es = getEquipmentSkill(ptr);
        if (es == ESM3::Skill::LightArmor)
            return std::string("Item Armor Light Down");
        else if (es == ESM3::Skill::MediumArmor)
            return std::string("Item Armor Medium Down");
        else
            return std::string("Item Armor Heavy Down");
    }

    std::string Armor::getInventoryIcon (const MWWorld::ConstPtr& ptr) const
    {
        const MWWorld::LiveCellRef<ESM3::Armor> *ref = ptr.get<ESM3::Armor>();

        return ref->mBase->mIcon;
    }

    MWGui::ToolTipInfo Armor::getToolTipInfo (const MWWorld::ConstPtr& ptr, int count) const
    {
        const MWWorld::LiveCellRef<ESM3::Armor> *ref = ptr.get<ESM3::Armor>();

        MWGui::ToolTipInfo info;
        info.caption = MyGUI::TextIterator::toTagsString(getName(ptr)) + MWGui::ToolTips::getCountString(count);
        info.icon = ref->mBase->mIcon;

        std::string text;

        // get armor type string (light/medium/heavy)
        std::string typeText;
        if (ref->mBase->mData.mWeight == 0)
            typeText = "";
        else
        {
            int armorType = getEquipmentSkill(ptr);
            if (armorType == ESM3::Skill::LightArmor)
                typeText = "#{sLight}";
            else if (armorType == ESM3::Skill::MediumArmor)
                typeText = "#{sMedium}";
            else
                typeText = "#{sHeavy}";
        }

        text += "\n#{sArmorRating}: " + MWGui::ToolTips::toString(static_cast<int>(getEffectiveArmorRating(ptr,
            MWMechanics::getPlayer())));

        int remainingHealth = getItemHealth(ptr);
        text += "\n#{sCondition}: " + MWGui::ToolTips::toString(remainingHealth) + "/"
                + MWGui::ToolTips::toString(ref->mBase->mData.mHealth);

        if (typeText != "")
            text += "\n#{sWeight}: " + MWGui::ToolTips::toString(ref->mBase->mData.mWeight) + " (" + typeText + ")";

        text += MWGui::ToolTips::getValueString(ref->mBase->mData.mValue, "#{sValue}");

        if (MWBase::Environment::get().getWindowManager()->getFullHelp()) {
            text += MWGui::ToolTips::getCellRefString(ptr.getCellRef());
            text += MWGui::ToolTips::getMiscString(ref->mBase->mScript, "Script");
        }

        info.enchant = ref->mBase->mEnchant;
        if (!info.enchant.empty())
            info.remainingEnchantCharge = static_cast<int>(ptr.getCellRef().getEnchantmentCharge());

        info.text = text;

        return info;
    }

    std::string Armor::getEnchantment (const MWWorld::ConstPtr& ptr) const
    {
        const MWWorld::LiveCellRef<ESM3::Armor> *ref = ptr.get<ESM3::Armor>();

        return ref->mBase->mEnchant;
    }

    std::string Armor::applyEnchantment(const MWWorld::ConstPtr &ptr, const std::string& enchId, int enchCharge, const std::string& newName) const
    {
        const MWWorld::LiveCellRef<ESM3::Armor> *ref = ptr.get<ESM3::Armor>();

        ESM3::Armor newItem = *ref->mBase;
        newItem.mId="";
        newItem.mName=newName;
        newItem.mData.mEnchant=enchCharge;
        newItem.mEnchant=enchId;
        const ESM3::Armor *record = MWBase::Environment::get().getWorld()->createRecord (newItem);
        return record->mId;
    }

    float Armor::getEffectiveArmorRating(const MWWorld::ConstPtr &ptr, const MWWorld::Ptr &actor) const
    {
        const MWWorld::LiveCellRef<ESM3::Armor> *ref = ptr.get<ESM3::Armor>();

        int armorSkillType = getEquipmentSkill(ptr);
        float armorSkill = actor.getClass().getSkill(actor, armorSkillType);

        const MWBase::World *world = MWBase::Environment::get().getWorld();
        int iBaseArmorSkill = world->getStore().get<ESM3::GameSetting>().find("iBaseArmorSkill")->mValue.getInteger();

        if(ref->mBase->mData.mWeight == 0)
            return ref->mBase->mData.mArmor;
        else
            return ref->mBase->mData.mArmor * armorSkill / static_cast<float>(iBaseArmorSkill);
    }

    std::pair<int, std::string> Armor::canBeEquipped(const MWWorld::ConstPtr &ptr, const MWWorld::Ptr &npc) const
    {
        const MWWorld::InventoryStore& invStore = npc.getClass().getInventoryStore(npc);

        if (getItemHealth(ptr) == 0)
            return std::make_pair(0, "#{sInventoryMessage1}");

        // slots that this item can be equipped in
        std::pair<std::vector<int>, bool> slots_ = getEquipmentSlots(ptr);

        if (slots_.first.empty())
            return std::make_pair(0, "");

        if (npc.getClass().isNpc())
        {
            std::string npcRace = npc.get<ESM3::NPC>()->mBase->mRace;

            // Beast races cannot equip shoes / boots, or full helms (head part vs hair part)
            const ESM3::Race* race = MWBase::Environment::get().getWorld()->getStore().get<ESM3::Race>().find(npcRace);
            if(race->mData.mFlags & ESM3::Race::Beast)
            {
                std::vector<ESM3::PartReference> parts = ptr.get<ESM3::Armor>()->mBase->mParts.mParts;

                for(std::vector<ESM3::PartReference>::iterator itr = parts.begin(); itr != parts.end(); ++itr)
                {
                    if((*itr).mPart == ESM3::PRT_Head)
                        return std::make_pair(0, "#{sNotifyMessage13}");
                    if((*itr).mPart == ESM3::PRT_LFoot || (*itr).mPart == ESM3::PRT_RFoot)
                        return std::make_pair(0, "#{sNotifyMessage14}");
                }
            }
        }

        for (std::vector<int>::const_iterator slot=slots_.first.begin();
            slot!=slots_.first.end(); ++slot)
        {
            // If equipping a shield, check if there's a twohanded weapon conflicting with it
            if(*slot == MWWorld::InventoryStore::Slot_CarriedLeft)
            {
                MWWorld::ConstContainerStoreIterator weapon = invStore.getSlot(MWWorld::InventoryStore::Slot_CarriedRight);
                if(weapon != invStore.end() && weapon->getTypeName() == typeid(ESM3::Weapon).name())
                {
                    const MWWorld::LiveCellRef<ESM3::Weapon> *ref = weapon->get<ESM3::Weapon>();
                    if (MWMechanics::getWeaponType(ref->mBase->mData.mType)->mFlags & ESM3::WeaponType::TwoHanded)
                        return std::make_pair(3,"");
                }

                return std::make_pair(1,"");
            }
        }
        return std::make_pair(1,"");
    }

    std::shared_ptr<MWWorld::Action> Armor::use (const MWWorld::Ptr& ptr, bool force) const
    {
        std::shared_ptr<MWWorld::Action> action(new MWWorld::ActionEquip(ptr, force));

        action->setSound(getUpSoundId(ptr));

        return action;
    }

    MWWorld::Ptr Armor::copyToCellImpl(const MWWorld::ConstPtr &ptr, MWWorld::CellStore &cell) const
    {
        const MWWorld::LiveCellRef<ESM3::Armor> *ref = ptr.get<ESM3::Armor>();

        return MWWorld::Ptr(cell.insert(ref), &cell);
    }

    int Armor::getEnchantmentPoints (const MWWorld::ConstPtr& ptr) const
    {
        const MWWorld::LiveCellRef<ESM3::Armor> *ref = ptr.get<ESM3::Armor>();

        return ref->mBase->mData.mEnchant;
    }

    bool Armor::canSell (const MWWorld::ConstPtr& item, int npcServices) const
    {
        return (npcServices & ESM3::NPC::Armor)
                || ((npcServices & ESM3::NPC::MagicItems) && !getEnchantment(item).empty());
    }

    float Armor::getWeight(const MWWorld::ConstPtr &ptr) const
    {
        const MWWorld::LiveCellRef<ESM3::Armor> *ref = ptr.get<ESM3::Armor>();
        return ref->mBase->mData.mWeight;
    }
}
