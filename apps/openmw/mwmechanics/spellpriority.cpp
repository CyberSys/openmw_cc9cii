#include "spellpriority.hpp"
#include "weaponpriority.hpp"

#include <components/esm3/ench.hpp>
#include <components/esm3/mgef.hpp>
#include <components/esm3/spel.hpp>

#include "../mwbase/environment.hpp"
#include "../mwbase/world.hpp"
#include "../mwbase/mechanicsmanager.hpp"

#include "../mwworld/class.hpp"
#include "../mwworld/esmstore.hpp"
#include "../mwworld/inventorystore.hpp"
#include "../mwworld/cellstore.hpp"

#include "creaturestats.hpp"
#include "spellresistance.hpp"
#include "weapontype.hpp"
#include "summoning.hpp"
#include "spellutil.hpp"

namespace
{
    int numEffectsToDispel (const MWWorld::Ptr& actor, int effectFilter=-1, bool negative = true)
    {
        int toCure=0;
        const MWMechanics::ActiveSpells& activeSpells = actor.getClass().getCreatureStats(actor).getActiveSpells();
        for (MWMechanics::ActiveSpells::TIterator it = activeSpells.begin(); it != activeSpells.end(); ++it)
        {
            // if the effect filter is not specified, take in account only spells effects. Leave potions, enchanted items etc.
            if (effectFilter == -1)
            {
                const ESM3::Spell* spell = MWBase::Environment::get().getWorld()->getStore().get<ESM3::Spell>().search(it->first);
                if (!spell || spell->mData.mType != ESM3::Spell::ST_Spell)
                    continue;
            }

            const MWMechanics::ActiveSpells::ActiveSpellParams& params = it->second;
            for (std::vector<MWMechanics::ActiveSpells::ActiveEffect>::const_iterator effectIt = params.mEffects.begin();
                effectIt != params.mEffects.end(); ++effectIt)
            {
                int effectId = effectIt->mEffectId;
                if (effectFilter != -1 && effectId != effectFilter)
                    continue;
                const ESM3::MagicEffect* magicEffect = MWBase::Environment::get().getWorld()->getStore().get<ESM3::MagicEffect>().find(effectId);

                if (effectIt->mDuration <= 3) // Don't attempt to dispel if effect runs out shortly anyway
                    continue;

                if (negative && magicEffect->mData.mFlags & ESM3::MagicEffect::Harmful)
                    ++toCure;

                if (!negative && !(magicEffect->mData.mFlags & ESM3::MagicEffect::Harmful))
                    ++toCure;
            }
        }
        return toCure;
    }

    float getSpellDuration (const MWWorld::Ptr& actor, const std::string& spellId)
    {
        float duration = 0;
        const MWMechanics::ActiveSpells& activeSpells = actor.getClass().getCreatureStats(actor).getActiveSpells();
        for (MWMechanics::ActiveSpells::TIterator it = activeSpells.begin(); it != activeSpells.end(); ++it)
        {
            if (it->first != spellId)
                continue;

            const MWMechanics::ActiveSpells::ActiveSpellParams& params = it->second;
            for (std::vector<MWMechanics::ActiveSpells::ActiveEffect>::const_iterator effectIt = params.mEffects.begin();
                effectIt != params.mEffects.end(); ++effectIt)
            {
                if (effectIt->mDuration > duration)
                    duration = effectIt->mDuration;
            }
        }
        return duration;
    }
}

namespace MWMechanics
{
    int getRangeTypes (const ESM3::EffectList& effects)
    {
        int types = 0;
        for (std::vector<ESM3::ENAMstruct>::const_iterator it = effects.mList.begin(); it != effects.mList.end(); ++it)
        {
            if (it->mRange == ESM::RT_Self)
                types |= RangeTypes::Self;
            else if (it->mRange == ESM::RT_Touch)
                types |= RangeTypes::Touch;
            else if (it->mRange == ESM::RT_Target)
                types |= RangeTypes::Target;
        }
        return types;
    }

    float ratePotion (const MWWorld::Ptr &item, const MWWorld::Ptr& actor)
    {
        if (item.getTypeName() != typeid(ESM3::Potion).name())
            return 0.f;

        const ESM3::Potion* potion = item.get<ESM3::Potion>()->mBase;
        return rateEffects(potion->mEffects, actor, MWWorld::Ptr());
    }

    float rateSpell(const ESM3::Spell *spell, const MWWorld::Ptr &actor, const MWWorld::Ptr& enemy)
    {
        const CreatureStats& stats = actor.getClass().getCreatureStats(actor);

        float successChance = MWMechanics::getSpellSuccessChance(spell, actor);
        if (successChance == 0.f)
            return 0.f;

        if (spell->mData.mType != ESM3::Spell::ST_Spell)
            return 0.f;

        // Don't make use of racial bonus spells, like MW. Can be made optional later
        if (actor.getClass().isNpc())
        {
            std::string raceid = actor.get<ESM3::NPC>()->mBase->mRace;
            const ESM3::Race* race = MWBase::Environment::get().getWorld()->getStore().get<ESM3::Race>().find(raceid);
            if (race->mPowers.exists(spell->mId))
                return 0.f;
        }

        // Spells don't stack, so early out if the spell is still active on the target
        int types = getRangeTypes(spell->mEffects);
        if ((types & Self) && stats.getActiveSpells().isSpellActive(spell->mId))
            return 0.f;
        if ( ((types & Touch) || (types & Target)) && enemy.getClass().getCreatureStats(enemy).getActiveSpells().isSpellActive(spell->mId))
            return 0.f;

        return rateEffects(spell->mEffects, actor, enemy) * (successChance / 100.f);
    }

    float rateMagicItem(const MWWorld::Ptr &ptr, const MWWorld::Ptr &actor, const MWWorld::Ptr& enemy)
    {
        if (ptr.getClass().getEnchantment(ptr).empty())
            return 0.f;

        const ESM3::Enchantment* enchantment = MWBase::Environment::get().getWorld()->getStore().get<ESM3::Enchantment>().find(ptr.getClass().getEnchantment(ptr));

        // Spells don't stack, so early out if the spell is still active on the target
        int types = getRangeTypes(enchantment->mEffects);
        if ((types & Self) && actor.getClass().getCreatureStats(actor).getActiveSpells().isSpellActive(ptr.getCellRef().getRefId()))
            return 0.f;

        if (types & (Touch|Target) && getSpellDuration(enemy, ptr.getCellRef().getRefId()) > 3)
            return 0.f;

        if (enchantment->mData.mType == ESM3::Enchantment::CastOnce)
        {
            return rateEffects(enchantment->mEffects, actor, enemy);
        }
        else if (enchantment->mData.mType == ESM3::Enchantment::WhenUsed)
        {
            MWWorld::InventoryStore& store = actor.getClass().getInventoryStore(actor);

            // Creatures can not wear armor/clothing, so allow creatures to use non-equipped items,
            if (actor.getClass().isNpc() && !store.isEquipped(ptr))
                return 0.f;

            int castCost = getEffectiveEnchantmentCastCost(static_cast<float>(enchantment->mData.mCost), actor);

            if (ptr.getCellRef().getEnchantmentCharge() != -1
               && ptr.getCellRef().getEnchantmentCharge() < castCost)
                return 0.f;

            float rating = rateEffects(enchantment->mEffects, actor, enemy);

            rating *= 1.25f; // prefer rechargable magic items over spells
            return rating;
        }

        return 0.f;
    }

    float rateEffect(const ESM3::ENAMstruct &effect, const MWWorld::Ptr &actor, const MWWorld::Ptr &enemy)
    {
        // NOTE: enemy may be empty

        float rating = 1;
        switch (effect.mEffectID)
        {
        case ESM3::MagicEffect::Soultrap:
        case ESM3::MagicEffect::AlmsiviIntervention:
        case ESM3::MagicEffect::DivineIntervention:
        case ESM3::MagicEffect::CalmHumanoid:
        case ESM3::MagicEffect::CalmCreature:
        case ESM3::MagicEffect::FrenzyHumanoid:
        case ESM3::MagicEffect::FrenzyCreature:
        case ESM3::MagicEffect::DemoralizeHumanoid:
        case ESM3::MagicEffect::DemoralizeCreature:
        case ESM3::MagicEffect::RallyHumanoid:
        case ESM3::MagicEffect::RallyCreature:
        case ESM3::MagicEffect::Charm:
        case ESM3::MagicEffect::DetectAnimal:
        case ESM3::MagicEffect::DetectEnchantment:
        case ESM3::MagicEffect::DetectKey:
        case ESM3::MagicEffect::Telekinesis:
        case ESM3::MagicEffect::Mark:
        case ESM3::MagicEffect::Recall:
        case ESM3::MagicEffect::Jump:
        case ESM3::MagicEffect::WaterBreathing:
        case ESM3::MagicEffect::SwiftSwim:
        case ESM3::MagicEffect::WaterWalking:
        case ESM3::MagicEffect::SlowFall:
        case ESM3::MagicEffect::Light:
        case ESM3::MagicEffect::Lock:
        case ESM3::MagicEffect::Open:
        case ESM3::MagicEffect::TurnUndead:
        case ESM3::MagicEffect::WeaknessToCommonDisease:
        case ESM3::MagicEffect::WeaknessToBlightDisease:
        case ESM3::MagicEffect::WeaknessToCorprusDisease:
        case ESM3::MagicEffect::CureCommonDisease:
        case ESM3::MagicEffect::CureBlightDisease:
        case ESM3::MagicEffect::CureCorprusDisease:
        case ESM3::MagicEffect::ResistBlightDisease:
        case ESM3::MagicEffect::ResistCommonDisease:
        case ESM3::MagicEffect::ResistCorprusDisease:
        case ESM3::MagicEffect::Invisibility:
        case ESM3::MagicEffect::Chameleon:
        case ESM3::MagicEffect::NightEye:
        case ESM3::MagicEffect::Vampirism:
        case ESM3::MagicEffect::StuntedMagicka:
        case ESM3::MagicEffect::ExtraSpell:
        case ESM3::MagicEffect::RemoveCurse:
        case ESM3::MagicEffect::CommandCreature:
        case ESM3::MagicEffect::CommandHumanoid:
            return 0.f;

        case ESM3::MagicEffect::Blind:
            {
                if (enemy.isEmpty())
                    return 0.f;

                const CreatureStats& stats = enemy.getClass().getCreatureStats(enemy);

                // Enemy can't attack
                if (stats.isParalyzed() || stats.getKnockedDown())
                    return 0.f;

                // Enemy doesn't attack
                if (stats.getDrawState() != MWMechanics::DrawState_Weapon)
                    return 0.f;

                break;
            }

        case ESM3::MagicEffect::Sound:
            {
                if (enemy.isEmpty())
                    return 0.f;

                const CreatureStats& stats = enemy.getClass().getCreatureStats(enemy);

                // Enemy can't cast spells
                if (stats.getMagicEffects().get(ESM3::MagicEffect::Silence).getMagnitude() > 0)
                    return 0.f;

                if (stats.isParalyzed() || stats.getKnockedDown())
                    return 0.f;

                // Enemy doesn't cast spells
                if (stats.getDrawState() != MWMechanics::DrawState_Spell)
                    return 0.f;

                break;
            }

        case ESM3::MagicEffect::Silence:
            {
                if (enemy.isEmpty())
                    return 0.f;

                const CreatureStats& stats = enemy.getClass().getCreatureStats(enemy);

                // Enemy can't cast spells
                if (stats.isParalyzed() || stats.getKnockedDown())
                    return 0.f;

                // Enemy doesn't cast spells
                if (stats.getDrawState() != MWMechanics::DrawState_Spell)
                    return 0.f;
                break;
            }

        case ESM3::MagicEffect::RestoreAttribute:
            return 0.f; // TODO: implement based on attribute damage
        case ESM3::MagicEffect::RestoreSkill:
            return 0.f; // TODO: implement based on skill damage

        case ESM3::MagicEffect::ResistFire:
        case ESM3::MagicEffect::ResistFrost:
        case ESM3::MagicEffect::ResistMagicka:
        case ESM3::MagicEffect::ResistNormalWeapons:
        case ESM3::MagicEffect::ResistParalysis:
        case ESM3::MagicEffect::ResistPoison:
        case ESM3::MagicEffect::ResistShock:
        case ESM3::MagicEffect::SpellAbsorption:
        case ESM3::MagicEffect::Reflect:
            return 0.f; // probably useless since we don't know in advance what the enemy will cast

        // don't cast these for now as they would make the NPC cast the same effect over and over again, especially when they have potions
        case ESM3::MagicEffect::FortifyAttribute:
        case ESM3::MagicEffect::FortifyHealth:
        case ESM3::MagicEffect::FortifyMagicka:
        case ESM3::MagicEffect::FortifyFatigue:
        case ESM3::MagicEffect::FortifySkill:
        case ESM3::MagicEffect::FortifyMaximumMagicka:
        case ESM3::MagicEffect::FortifyAttack:
            return 0.f;

        case ESM3::MagicEffect::Burden:
            {
                if (enemy.isEmpty())
                    return 0.f;

                // Ignore enemy without inventory
                if (!enemy.getClass().hasInventoryStore(enemy))
                    return 0.f;

                // burden makes sense only to overburden an enemy
                float burden = enemy.getClass().getEncumbrance(enemy) - enemy.getClass().getCapacity(enemy);
                if (burden > 0)
                    return 0.f;

                if ((effect.mMagnMin + effect.mMagnMax)/2.f > -burden)
                    rating *= 3;
                else
                    return 0.f;

                break;
            }

        case ESM3::MagicEffect::Feather:
            {
                // Ignore actors without inventory
                if (!actor.getClass().hasInventoryStore(actor))
                    return 0.f;

                // feather makes sense only for overburden actors
                float burden = actor.getClass().getEncumbrance(actor) - actor.getClass().getCapacity(actor);
                if (burden <= 0)
                    return 0.f;

                if ((effect.mMagnMin + effect.mMagnMax)/2.f >= burden)
                    rating *= 3;
                else
                    return 0.f;

                break;
            }

        case ESM3::MagicEffect::Levitate:
            return 0.f; // AI isn't designed to take advantage of this, and could be perceived as unfair anyway
        case ESM3::MagicEffect::BoundBoots:
        case ESM3::MagicEffect::BoundHelm:
            if (actor.getClass().isNpc())
            {
                // Beast races can't wear helmets or boots
                std::string raceid = actor.get<ESM3::NPC>()->mBase->mRace;
                const ESM3::Race* race = MWBase::Environment::get().getWorld()->getStore().get<ESM3::Race>().find(raceid);
                if (race->mData.mFlags & ESM3::Race::Beast)
                    return 0.f;
            }
            else
                return 0.f;

            break;
        // Creatures can not wear armor
        case ESM3::MagicEffect::BoundCuirass:
        case ESM3::MagicEffect::BoundGloves:
            if (!actor.getClass().isNpc())
                return 0.f;
            break;

        case ESM3::MagicEffect::BoundLongbow:
            // AI should not summon the bow if there is no suitable ammo.
            if (rateAmmo(actor, enemy, getWeaponType(ESM3::Weapon::MarksmanBow)->mAmmoType) <= 0.f)
                return 0.f;
            break;

        case ESM3::MagicEffect::RestoreHealth:
        case ESM3::MagicEffect::RestoreMagicka:
        case ESM3::MagicEffect::RestoreFatigue:
            if (effect.mRange == ESM::RT_Self)
            {
                int priority = 1;
                if (effect.mEffectID == ESM3::MagicEffect::RestoreHealth)
                    priority = 10;
                const MWMechanics::CreatureStats& stats = actor.getClass().getCreatureStats(actor);
                const DynamicStat<float>& current = stats.getDynamic(effect.mEffectID - ESM3::MagicEffect::RestoreHealth);
                // NB: this currently assumes the hardcoded magic effect flags are used
                const float magnitude = (effect.mMagnMin + effect.mMagnMax)/2.f;
                const float toHeal = magnitude * std::max(1, (int)effect.mDuration); // NOTE: mDuration used to be int
                // Effect doesn't heal more than we need, *or* we are below 1/2 health
                if (current.getModified() - current.getCurrent() > toHeal
                        || current.getCurrent() < current.getModified()*0.5)
                {
                    return 10000.f * priority
                            - (toHeal - (current.getModified()-current.getCurrent())); // prefer the most fitting potion
                }
                else
                    return -10000.f * priority; // Save for later
            }
            break;

        case ESM3::MagicEffect::Dispel:
        {
            int numPositive = 0;
            int numNegative = 0;
            int diff = 0;

            if (effect.mRange == ESM::RT_Self)
            {
                numPositive = numEffectsToDispel(actor, -1, false);
                numNegative = numEffectsToDispel(actor);

                diff = numNegative - numPositive;
            }
            else
            {
                if (enemy.isEmpty())
                    return 0.f;

                numPositive = numEffectsToDispel(enemy, -1, false);
                numNegative = numEffectsToDispel(enemy);

                diff = numPositive - numNegative;

                // if rating < 0 here, the spell will be considered as negative later
                rating *= -1;
            }

            if (diff <= 0)
                return 0.f;

            rating *= (diff) / 5.f;

            break;
        }

        // Prefer Cure effects over Dispel, because Dispel also removes positive effects
        case ESM3::MagicEffect::CureParalyzation:
            return 1001.f * numEffectsToDispel(actor, ESM3::MagicEffect::Paralyze);

        case ESM3::MagicEffect::CurePoison:
            return 1001.f * numEffectsToDispel(actor, ESM3::MagicEffect::Poison);
        case ESM3::MagicEffect::DisintegrateArmor:
            {
                if (enemy.isEmpty())
                    return 0.f;

                // Ignore enemy without inventory
                if (!enemy.getClass().hasInventoryStore(enemy))
                    return 0.f;

                MWWorld::InventoryStore& inv = enemy.getClass().getInventoryStore(enemy);

                // According to UESP
                static const int armorSlots[] = {
                    MWWorld::InventoryStore::Slot_CarriedLeft,
                    MWWorld::InventoryStore::Slot_Cuirass,
                    MWWorld::InventoryStore::Slot_LeftPauldron,
                    MWWorld::InventoryStore::Slot_RightPauldron,
                    MWWorld::InventoryStore::Slot_LeftGauntlet,
                    MWWorld::InventoryStore::Slot_RightGauntlet,
                    MWWorld::InventoryStore::Slot_Helmet,
                    MWWorld::InventoryStore::Slot_Greaves,
                    MWWorld::InventoryStore::Slot_Boots
                };

                bool enemyHasArmor = false;

                // Ignore enemy without armor
                for (unsigned int i=0; i<sizeof(armorSlots)/sizeof(int); ++i)
                {
                    MWWorld::ContainerStoreIterator item = inv.getSlot(armorSlots[i]);

                    if (item != inv.end() && (item.getType() == MWWorld::ContainerStore::Type_Armor))
                    {
                        enemyHasArmor = true;
                        break;
                    }
                }

                if (!enemyHasArmor)
                    return 0.f;

                break;
            }

        case ESM3::MagicEffect::DisintegrateWeapon:
            {
                if (enemy.isEmpty())
                    return 0.f;

                // Ignore enemy without inventory
                if (!enemy.getClass().hasInventoryStore(enemy))
                    return 0.f;

                MWWorld::InventoryStore& inv = enemy.getClass().getInventoryStore(enemy);
                MWWorld::ContainerStoreIterator item = inv.getSlot(MWWorld::InventoryStore::Slot_CarriedRight);

                // Ignore enemy without weapons
                if (item == inv.end() || (item.getType() != MWWorld::ContainerStore::Type_Weapon))
                    return 0.f;

                break;
            }

        case ESM3::MagicEffect::DamageAttribute:
        case ESM3::MagicEffect::DrainAttribute:
            if (!enemy.isEmpty() && enemy.getClass().getCreatureStats(enemy).getAttribute(effect.mAttribute).getModified() <= 0)
                return 0.f;
            {
                if (effect.mAttribute >= 0 && effect.mAttribute < ESM::Attribute::Length)
                {
                    const float attributePriorities[ESM::Attribute::Length] = {
                        1.0f, // Strength
                        0.5f, // Intelligence
                        0.6f, // Willpower
                        0.7f, // Agility
                        0.5f, // Speed
                        0.8f, // Endurance
                        0.7f, // Personality
                        0.3f // Luck
                    };
                    rating *= attributePriorities[effect.mAttribute];
                }
            }
            break;

        case ESM3::MagicEffect::DamageSkill:
        case ESM3::MagicEffect::DrainSkill:
            if (enemy.isEmpty() || !enemy.getClass().isNpc())
                return 0.f;
            if (enemy.getClass().getSkill(enemy, effect.mSkill) <= 0)
                return 0.f;
            break;

        default:
            break;
        }

        // Allow only one summoned creature at time
        if (isSummoningEffect(effect.mEffectID))
        {
            MWMechanics::CreatureStats& creatureStats = actor.getClass().getCreatureStats(actor);

            if (!creatureStats.getSummonedCreatureMap().empty())
                return 0.f;
        }

        // Underwater casting not possible
        if (effect.mRange == ESM::RT_Target)
        {
            if (MWBase::Environment::get().getWorld()->isUnderwater(MWWorld::ConstPtr(actor), 0.75f))
                return 0.f;

            if (enemy.isEmpty())
                return 0.f;

            if (MWBase::Environment::get().getWorld()->isUnderwater(MWWorld::ConstPtr(enemy), 0.75f))
                return 0.f;
        }

        const ESM3::MagicEffect* magicEffect = MWBase::Environment::get().getWorld()->getStore().get<ESM3::MagicEffect>().find(effect.mEffectID);
        if (magicEffect->mData.mFlags & ESM3::MagicEffect::Harmful)
        {
            rating *= -1.f;

            if (enemy.isEmpty())
                return 0.f;

            // Check resistance for harmful effects
            CreatureStats& stats = enemy.getClass().getCreatureStats(enemy);

            float resistance = MWMechanics::getEffectResistanceAttribute(effect.mEffectID, &stats.getMagicEffects());

            rating *= (1.f - std::min(resistance, 100.f) / 100.f);
        }

        // for harmful no-magnitude effects (e.g. silence) check if enemy is already has them
        // for non-harmful no-magnitude effects (e.g. bound items) check if actor is already has them
        if (magicEffect->mData.mFlags & ESM3::MagicEffect::NoMagnitude)
        {
            if (magicEffect->mData.mFlags & ESM3::MagicEffect::Harmful)
            {
                CreatureStats& stats = enemy.getClass().getCreatureStats(enemy);

                if (stats.getMagicEffects().get(effect.mEffectID).getMagnitude() > 0)
                    return 0.f;
            }
            else
            {
                CreatureStats& stats = actor.getClass().getCreatureStats(actor);

                if (stats.getMagicEffects().get(effect.mEffectID).getMagnitude() > 0)
                    return 0.f;
            }
        }

        rating *= calcEffectCost(effect, magicEffect);

        // Currently treating all "on target" or "on touch" effects to target the enemy actor.
        // Combat AI is egoistic, so doesn't consider applying positive effects to friendly actors.
        if (effect.mRange != ESM::RT_Self)
            rating *= -1.f;

        return rating;
    }

    float rateEffects(const ESM3::EffectList &list, const MWWorld::Ptr& actor, const MWWorld::Ptr& enemy)
    {
        // NOTE: enemy may be empty

        float rating = 0.f;
        float ratingMult = 1.f; // NB: this multiplier is applied to the effect rating, not the final rating

        const MWWorld::Store<ESM3::GameSetting>& gmst = MWBase::Environment::get().getWorld()->getStore().get<ESM3::GameSetting>();
        static const float fAIMagicSpellMult = gmst.find("fAIMagicSpellMult")->mValue.getFloat();
        static const float fAIRangeMagicSpellMult = gmst.find("fAIRangeMagicSpellMult")->mValue.getFloat();

        for (std::vector<ESM3::ENAMstruct>::const_iterator it = list.mList.begin(); it != list.mList.end(); ++it)
        {
            ratingMult = (it->mRange == ESM::RT_Target) ? fAIRangeMagicSpellMult : fAIMagicSpellMult;

            rating += rateEffect(*it, actor, enemy) * ratingMult;
        }
        return rating;
    }

    float vanillaRateSpell(const ESM3::Spell* spell, const MWWorld::Ptr& actor, const MWWorld::Ptr& enemy)
    {
        const MWWorld::Store<ESM3::GameSetting>& gmst = MWBase::Environment::get().getWorld()->getStore().get<ESM3::GameSetting>();

        static const float fAIMagicSpellMult = gmst.find("fAIMagicSpellMult")->mValue.getFloat();
        static const float fAIRangeMagicSpellMult = gmst.find("fAIRangeMagicSpellMult")->mValue.getFloat();

        float mult = fAIMagicSpellMult;

        for (std::vector<ESM3::ENAMstruct>::const_iterator effectIt =
             spell->mEffects.mList.begin(); effectIt != spell->mEffects.mList.end(); ++effectIt)
        {
            if (effectIt->mRange == ESM::RT_Target)
            {
                if (!MWBase::Environment::get().getWorld()->isSwimming(enemy))
                    mult = fAIRangeMagicSpellMult;
                else
                    mult = 0.0f;
                break;
            }
        }

        return MWMechanics::getSpellSuccessChance(spell, actor) * mult;
    }
}
