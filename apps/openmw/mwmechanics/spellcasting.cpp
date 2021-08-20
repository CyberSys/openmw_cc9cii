#include "spellcasting.hpp"

#include <components/misc/constants.hpp>
#include <components/misc/rng.hpp>

#include "../mwbase/windowmanager.hpp"
#include "../mwbase/soundmanager.hpp"
#include "../mwbase/mechanicsmanager.hpp"
#include "../mwbase/environment.hpp"
#include "../mwbase/world.hpp"

#include "../mwworld/containerstore.hpp"
#include "../mwworld/actionteleport.hpp"
#include "../mwworld/player.hpp"
#include "../mwworld/class.hpp"
#include "../mwworld/cellstore.hpp"
#include "../mwworld/esmstore.hpp"
#include "../mwworld/inventorystore.hpp"

#include "../mwrender/animation.hpp"

#include "actorutil.hpp"
#include "aifollow.hpp"
#include "creaturestats.hpp"
#include "linkedeffects.hpp"
#include "spellabsorption.hpp"
#include "spellresistance.hpp"
#include "spellutil.hpp"
#include "summoning.hpp"
#include "tickableeffects.hpp"
#include "weapontype.hpp"

namespace MWMechanics
{
    CastSpell::CastSpell(const MWWorld::Ptr &caster, const MWWorld::Ptr &target, const bool fromProjectile, const bool manualSpell)
        : mCaster(caster)
        , mTarget(target)
        , mFromProjectile(fromProjectile)
        , mManualSpell(manualSpell)
    {
    }

    void CastSpell::launchMagicBolt ()
    {
        osg::Vec3f fallbackDirection(0, 1, 0);
        osg::Vec3f offset(0, 0, 0);
        if (!mTarget.isEmpty() && mTarget.getClass().isActor())
            offset.z() = MWBase::Environment::get().getWorld()->getHalfExtents(mTarget).z();

        // Fall back to a "caster to target" direction if we have no other means of determining it
        // (e.g. when cast by a non-actor)
        if (!mTarget.isEmpty())
            fallbackDirection =
                (mTarget.getRefData().getPosition().asVec3() + offset) -
                (mCaster.getRefData().getPosition().asVec3());

        MWBase::Environment::get().getWorld()->launchMagicBolt(mId, mCaster, fallbackDirection);
    }

    void CastSpell::inflict(const MWWorld::Ptr &target, const MWWorld::Ptr &caster,
                            const ESM3::EffectList &effects, ESM::RangeType range, bool reflected, bool exploded)
    {
        const bool targetIsActor = !target.isEmpty() && target.getClass().isActor();
        if (targetIsActor)
        {
            // Early-out for characters that have departed.
            const auto& stats = target.getClass().getCreatureStats(target);
            if (stats.isDead() && stats.isDeathAnimationFinished())
                return;
        }

        // If none of the effects need to apply, we can early-out
        bool found = false;
        for (const ESM3::ENAMstruct& effect : effects.mList)
        {
            if ((int)effect.mRange == range) // NOTE: mRange used to be int
            {
                found = true;
                break;
            }
        }
        if (!found)
            return;

        const ESM3::Spell* spell = MWBase::Environment::get().getWorld()->getStore().get<ESM3::Spell>().search (mId);
        if (spell && targetIsActor && (spell->mData.mType == ESM3::Spell::ST_Disease || spell->mData.mType == ESM3::Spell::ST_Blight))
        {
            int requiredResistance = (spell->mData.mType == ESM3::Spell::ST_Disease) ?
                ESM3::MagicEffect::ResistCommonDisease
                : ESM3::MagicEffect::ResistBlightDisease;
            float x = target.getClass().getCreatureStats(target).getMagicEffects().get(requiredResistance).getMagnitude();

            if (Misc::Rng::roll0to99() <= x)
            {
                // Fully resisted, show message
                if (target == getPlayer())
                    MWBase::Environment::get().getWindowManager()->messageBox("#{sMagicPCResisted}");
                return;
            }
        }

        ESM3::EffectList reflectedEffects;
        std::vector<ActiveSpells::ActiveEffect> appliedLastingEffects;

        // HACK: cache target's magic effects here, and add any applied effects to it. Use the cached effects for determining resistance.
        // This is required for Weakness effects in a spell to apply to any subsequent effects in the spell.
        // Otherwise, they'd only apply after the whole spell was added.
        MagicEffects targetEffects;
        if (targetIsActor)
            targetEffects += target.getClass().getCreatureStats(target).getMagicEffects();

        bool castByPlayer = (!caster.isEmpty() && caster == getPlayer());

        ActiveSpells targetSpells;
        if (targetIsActor)
            targetSpells = target.getClass().getCreatureStats(target).getActiveSpells();

        bool canCastAnEffect = false;    // For bound equipment.If this remains false
                                         // throughout the iteration of this spell's
                                         // effects, we display a "can't re-cast" message

        // Try absorbing the spell. Some handling must still happen for absorbed effects.
        bool absorbed = absorbSpell(mId, caster, target);

        int currentEffectIndex = 0;
        for (std::vector<ESM3::ENAMstruct>::const_iterator effectIt (effects.mList.begin());
             !target.isEmpty() && effectIt != effects.mList.end(); ++effectIt, ++currentEffectIndex)
        {
            if ((int)effectIt->mRange != range) // NOTE: mRange used to be int
                continue;

            const ESM3::MagicEffect *magicEffect =
                MWBase::Environment::get().getWorld()->getStore().get<ESM3::MagicEffect>().find (
                effectIt->mEffectID);

            // Re-casting a bound equipment effect has no effect if the spell is still active
            if (magicEffect->mData.mFlags & ESM3::MagicEffect::NonRecastable && targetSpells.isSpellActive(mId))
            {
                if (effectIt == (effects.mList.end() - 1) && !canCastAnEffect && castByPlayer)
                    MWBase::Environment::get().getWindowManager()->messageBox("#{sMagicCannotRecast}");
                continue;
            }
            canCastAnEffect = true;

            if (!checkEffectTarget(effectIt->mEffectID, target, caster, castByPlayer))
                continue;

            // caster needs to be an actor for linked effects (e.g. Absorb)
            if (magicEffect->mData.mFlags & ESM3::MagicEffect::CasterLinked
                    && (caster.isEmpty() || !caster.getClass().isActor()))
                continue;

            // Notify the target actor they've been hit
            bool isHarmful = magicEffect->mData.mFlags & ESM3::MagicEffect::Harmful;
            if (target.getClass().isActor() && target != caster && !caster.isEmpty() && isHarmful)
                target.getClass().onHit(target, 0.0f, true, MWWorld::Ptr(), caster, osg::Vec3f(), true);

            // Avoid proceeding further for absorbed spells.
            if (absorbed)
                continue;

            // Reflect harmful effects
            if (!reflected && reflectEffect(*effectIt, magicEffect, caster, target, reflectedEffects))
                continue;

            // Try resisting.
            float magnitudeMult = getEffectMultiplier(effectIt->mEffectID, target, caster, spell, &targetEffects);
            if (magnitudeMult == 0)
            {
                // Fully resisted, show message
                if (target == getPlayer())
                    MWBase::Environment::get().getWindowManager()->messageBox("#{sMagicPCResisted}");
                else if (castByPlayer)
                    MWBase::Environment::get().getWindowManager()->messageBox("#{sMagicTargetResisted}");
            }
            else
            {
                float magnitude = effectIt->mMagnMin + Misc::Rng::rollDice(effectIt->mMagnMax - effectIt->mMagnMin + 1);
                magnitude *= magnitudeMult;

                if (!target.getClass().isActor())
                {
                    // non-actor objects have no list of active magic effects, so have to apply instantly
                    if (!applyInstantEffect(target, caster, EffectKey(*effectIt), magnitude))
                        continue;
                }
                else // target.getClass().isActor() == true
                {
                    ActiveSpells::ActiveEffect effect;
                    effect.mEffectId = effectIt->mEffectID;
                    effect.mArg = MWMechanics::EffectKey(*effectIt).mArg;
                    effect.mMagnitude = magnitude;
                    effect.mTimeLeft = 0.f;
                    effect.mEffectIndex = currentEffectIndex;

                    // Avoid applying absorb effects if the caster is the target
                    // We still need the spell to be added
                    if (caster == target
                        && effectIt->mEffectID >= ESM3::MagicEffect::AbsorbAttribute
                        && effectIt->mEffectID <= ESM3::MagicEffect::AbsorbSkill)
                    {
                        effect.mMagnitude = 0;
                    }

                    // Avoid applying harmful effects to the player in god mode
                    if (target == getPlayer() && MWBase::Environment::get().getWorld()->getGodModeState() && isHarmful)
                    {
                        effect.mMagnitude = 0;
                    }

                    bool effectAffectsHealth = isHarmful || effectIt->mEffectID == ESM3::MagicEffect::RestoreHealth;
                    if (castByPlayer && target != caster && !target.getClass().getCreatureStats(target).isDead() && effectAffectsHealth)
                    {
                        // If player is attempting to cast a harmful spell on or is healing a living target, show the target's HP bar.
                        MWBase::Environment::get().getWindowManager()->setEnemy(target);
                    }

                    bool hasDuration = !(magicEffect->mData.mFlags & ESM3::MagicEffect::NoDuration);
                    effect.mDuration = hasDuration ? static_cast<float>(effectIt->mDuration) : 1.f;

                    bool appliedOnce = magicEffect->mData.mFlags & ESM3::MagicEffect::AppliedOnce;
                    if (!appliedOnce)
                        effect.mDuration = std::max(1.f, effect.mDuration);

                    if (effect.mDuration == 0)
                    {
                        // We still should add effect to list to allow GetSpellEffects to detect this spell
                        appliedLastingEffects.push_back(effect);

                        // duration 0 means apply full magnitude instantly
                        bool wasDead = target.getClass().getCreatureStats(target).isDead();
                        effectTick(target.getClass().getCreatureStats(target), target, EffectKey(*effectIt), effect.mMagnitude);
                        bool isDead = target.getClass().getCreatureStats(target).isDead();

                        if (!wasDead && isDead)
                            MWBase::Environment::get().getMechanicsManager()->actorKilled(target, caster);
                    }
                    else
                    {
                        effect.mTimeLeft = effect.mDuration;

                        targetEffects.add(MWMechanics::EffectKey(*effectIt), MWMechanics::EffectParam(effect.mMagnitude));

                        // add to list of active effects, to apply in next frame
                        appliedLastingEffects.push_back(effect);

                        // Unequip all items, if a spell with the ExtraSpell effect was casted
                        if (effectIt->mEffectID == ESM3::MagicEffect::ExtraSpell && target.getClass().hasInventoryStore(target))
                        {
                            MWWorld::InventoryStore& store = target.getClass().getInventoryStore(target);
                            store.unequipAll(target);
                        }

                        // Command spells should have their effect, including taking the target out of combat, each time the spell successfully affects the target
                        if (((effectIt->mEffectID == ESM3::MagicEffect::CommandHumanoid && target.getClass().isNpc())
                        || (effectIt->mEffectID == ESM3::MagicEffect::CommandCreature && target.getTypeName() == typeid(ESM3::Creature).name()))
                        && !caster.isEmpty() && caster.getClass().isActor() && target != getPlayer() && effect.mMagnitude >= target.getClass().getCreatureStats(target).getLevel())
                        {
                            MWMechanics::AiFollow package(caster, true);
                            target.getClass().getCreatureStats(target).getAiSequence().stack(package, target);
                        }

                        // For absorb effects, also apply the effect to the caster - but with a negative
                        // magnitude, since we're transferring stats from the target to the caster
                        if (effectIt->mEffectID >= ESM3::MagicEffect::AbsorbAttribute && effectIt->mEffectID <= ESM3::MagicEffect::AbsorbSkill)
                            absorbStat(*effectIt, effect, caster, target, reflected, mSourceName);
                    }
                }

                // Re-casting a summon effect will remove the creature from previous castings of that effect.
                if (isSummoningEffect(effectIt->mEffectID) && targetIsActor)
                {
                    CreatureStats& targetStats = target.getClass().getCreatureStats(target);
                    ESM3::SummonKey key(effectIt->mEffectID, mId, currentEffectIndex);
                    auto findCreature = targetStats.getSummonedCreatureMap().find(key);
                    if (findCreature != targetStats.getSummonedCreatureMap().end())
                    {
                        MWBase::Environment::get().getMechanicsManager()->cleanupSummonedCreature(target, findCreature->second);
                        targetStats.getSummonedCreatureMap().erase(findCreature);
                    }
                }

                if (target.getClass().isActor() || magicEffect->mData.mFlags & ESM3::MagicEffect::NoDuration)
                {
                    static const std::string schools[] = {
                        "alteration", "conjuration", "destruction", "illusion", "mysticism", "restoration"
                    };

                    MWBase::SoundManager *sndMgr = MWBase::Environment::get().getSoundManager();
                    if(!magicEffect->mHitSound.empty())
                        sndMgr->playSound3D(target, magicEffect->mHitSound, 1.0f, 1.0f);
                    else
                        sndMgr->playSound3D(target, schools[magicEffect->mData.mSchool]+" hit", 1.0f, 1.0f);

                    // Add VFX
                    const ESM3::Static* castStatic;
                    if (!magicEffect->mHit.empty())
                        castStatic = MWBase::Environment::get().getWorld()->getStore().get<ESM3::Static>().find (magicEffect->mHit);
                    else
                        castStatic = MWBase::Environment::get().getWorld()->getStore().get<ESM3::Static>().find ("VFX_DefaultHit");

                    bool loop = (magicEffect->mData.mFlags & ESM3::MagicEffect::ContinuousVfx) != 0;
                    // Note: in case of non actor, a free effect should be fine as well
                    MWRender::Animation* anim = MWBase::Environment::get().getWorld()->getAnimation(target);
                    if (anim && !castStatic->mModel.empty())
                        anim->addEffect("meshes\\" + castStatic->mModel, magicEffect->mIndex, loop, "", magicEffect->mParticle);
                }
            }
        }

        if (!exploded)
            MWBase::Environment::get().getWorld()->explodeSpell(mHitPosition, effects, caster, target, range, mId, mSourceName, mFromProjectile);

        if (!target.isEmpty())
        {
            if (!reflectedEffects.mList.empty())
                inflict(caster, target, reflectedEffects, range, true, exploded);

            if (!appliedLastingEffects.empty())
            {
                int casterActorId = -1;
                if (!caster.isEmpty() && caster.getClass().isActor())
                    casterActorId = caster.getClass().getCreatureStats(caster).getActorId();
                target.getClass().getCreatureStats(target).getActiveSpells().addSpell(mId, mStack, appliedLastingEffects,
                        mSourceName, casterActorId);
            }
        }
    }

    bool CastSpell::applyInstantEffect(const MWWorld::Ptr &target, const MWWorld::Ptr &caster, const MWMechanics::EffectKey& effect, float magnitude)
    {
        short effectId = effect.mId;
        if (target.getClass().canLock(target))
        {
            if (effectId == ESM3::MagicEffect::Lock)
            {
                const MWWorld::ESMStore& store = MWBase::Environment::get().getWorld()->getStore();
                const ESM3::MagicEffect *magiceffect = store.get<ESM3::MagicEffect>().find(effectId);
                MWRender::Animation* animation = MWBase::Environment::get().getWorld()->getAnimation(target);
                if (animation)
                    animation->addSpellCastGlow(magiceffect);
                if (target.getCellRef().getLockLevel() < magnitude) //If the door is not already locked to a higher value, lock it to spell magnitude
                {
                    if (caster == getPlayer())
                        MWBase::Environment::get().getWindowManager()->messageBox("#{sMagicLockSuccess}");
                    target.getCellRef().lock(static_cast<int>(magnitude));
                }
                return true;
            }
            else if (effectId == ESM3::MagicEffect::Open)
            {
                if (!caster.isEmpty())
                {
                    MWBase::Environment::get().getMechanicsManager()->unlockAttempted(getPlayer(), target);
                    // Use the player instead of the caster for vanilla crime compatibility
                }
                const MWWorld::ESMStore& store = MWBase::Environment::get().getWorld()->getStore();
                const ESM3::MagicEffect *magiceffect = store.get<ESM3::MagicEffect>().find(effectId);
                MWRender::Animation* animation = MWBase::Environment::get().getWorld()->getAnimation(target);
                if (animation)
                    animation->addSpellCastGlow(magiceffect);
                if (target.getCellRef().getLockLevel() <= magnitude)
                {
                    if (target.getCellRef().getLockLevel() > 0)
                    {
                        MWBase::Environment::get().getSoundManager()->playSound3D(target, "Open Lock", 1.f, 1.f);

                        if (caster == getPlayer())
                            MWBase::Environment::get().getWindowManager()->messageBox("#{sMagicOpenSuccess}");
                    }
                    target.getCellRef().unlock();
                }
                else
                {
                    MWBase::Environment::get().getSoundManager()->playSound3D(target, "Open Lock Fail", 1.f, 1.f);
                }

                return true;
            }
        }
        else if (target.getClass().isActor() && effectId == ESM3::MagicEffect::Dispel)
        {
            target.getClass().getCreatureStats(target).getActiveSpells().purgeAll(magnitude, true);
            return true;
        }
        else if(target.getClass().isActor() && effectId >= ESM3::MagicEffect::CalmHumanoid && effectId <= ESM3::MagicEffect::RallyCreature)
        {
            // Treat X Humanoid spells on creatures and X Creature spells on NPCs as instant effects and remove their VFX
            bool affectsCreatures = (effectId - ESM3::MagicEffect::CalmHumanoid) & 1;
            if(affectsCreatures == target.getClass().isNpc())
            {
                MWBase::Environment::get().getWorld()->getAnimation(target)->removeEffect(effectId);
                return true;
            }
        }
        else if(target.getClass().isActor() && effectId == ESM3::MagicEffect::TurnUndead)
        {
            // Diverge from vanilla by giving scripts a chance to detect Turn Undead on non-undead, but still remove the effect and VFX
            if(target.getClass().isNpc() || target.get<ESM3::Creature>()->mBase->mData.mType != ESM3::Creature::Undead)
            {
                MWBase::Environment::get().getWorld()->getAnimation(target)->removeEffect(effectId);
                return true;
            }
        }
        else if (target.getClass().isActor() && target == getPlayer())
        {
            MWRender::Animation* anim = MWBase::Environment::get().getWorld()->getAnimation(mCaster);
            bool teleportingEnabled = MWBase::Environment::get().getWorld()->isTeleportingEnabled();

            if (effectId == ESM3::MagicEffect::DivineIntervention || effectId == ESM3::MagicEffect::AlmsiviIntervention)
            {
                if (!teleportingEnabled)
                {
                    if (caster == getPlayer())
                        MWBase::Environment::get().getWindowManager()->messageBox("#{sTeleportDisabled}");
                    return true;
                }
                std::string marker = (effectId == ESM3::MagicEffect::DivineIntervention) ? "divinemarker" : "templemarker";
                MWBase::Environment::get().getWorld()->teleportToClosestMarker(target, marker);
                anim->removeEffect(effectId);
                const ESM3::Static* fx = MWBase::Environment::get().getWorld()->getStore().get<ESM3::Static>()
                    .search("VFX_Summon_end");
                if (fx)
                    anim->addEffect("meshes\\" + fx->mModel, -1);
                return true;
            }
            else if (effectId == ESM3::MagicEffect::Mark)
            {
                if (teleportingEnabled)
                {
                    MWBase::Environment::get().getWorld()->getPlayer().markPosition(
                                target.getCell(), target.getRefData().getPosition());
                }
                else if (caster == getPlayer())
                {
                    MWBase::Environment::get().getWindowManager()->messageBox("#{sTeleportDisabled}");
                }
                return true;
            }
            else if (effectId == ESM3::MagicEffect::Recall)
            {
                if (!teleportingEnabled)
                {
                    if (caster == getPlayer())
                        MWBase::Environment::get().getWindowManager()->messageBox("#{sTeleportDisabled}");
                    return true;
                }

                MWWorld::CellStore* markedCell = nullptr;
                ESM::Position markedPosition;

                MWBase::Environment::get().getWorld()->getPlayer().getMarkedPosition(markedCell, markedPosition);
                if (markedCell)
                {
                    MWWorld::ActionTeleport action(markedCell->isExterior() ? "" : markedCell->getCell()->mName,
                                            markedPosition, false);
                    action.execute(target);
                    anim->removeEffect(effectId);
                }
                return true;
            }
        }
        return false;
    }


    bool CastSpell::cast(const std::string &id)
    {
        const MWWorld::ESMStore& store = MWBase::Environment::get().getWorld()->getStore();
        if (const auto spell = store.get<ESM3::Spell>().search(id))
            return cast(spell);

        if (const auto potion = store.get<ESM3::Potion>().search(id))
            return cast(potion);

        if (const auto ingredient = store.get<ESM3::Ingredient>().search(id))
            return cast(ingredient);

        throw std::runtime_error("ID type cannot be casted");
    }

    bool CastSpell::cast(const MWWorld::Ptr &item, bool launchProjectile)
    {
        std::string enchantmentName = item.getClass().getEnchantment(item);
        if (enchantmentName.empty())
            throw std::runtime_error("can't cast an item without an enchantment");

        mSourceName = item.getClass().getName(item);
        mId = item.getCellRef().getRefId();

        const ESM3::Enchantment* enchantment = MWBase::Environment::get().getWorld()->getStore().get<ESM3::Enchantment>().find(enchantmentName);

        mStack = false;

        bool godmode = mCaster == MWMechanics::getPlayer() && MWBase::Environment::get().getWorld()->getGodModeState();
        bool isProjectile = false;
        if (item.getTypeName() == typeid(ESM3::Weapon).name())
        {
            int type = item.get<ESM3::Weapon>()->mBase->mData.mType;
            ESM3::WeaponType::Class weapclass = MWMechanics::getWeaponType(type)->mWeaponClass;
            isProjectile = (weapclass == ESM3::WeaponType::Thrown || weapclass == ESM3::WeaponType::Ammo);
        }
        int type = enchantment->mData.mType;

        // Check if there's enough charge left
        if (!godmode && (type == ESM3::Enchantment::WhenUsed || (!isProjectile && type == ESM3::Enchantment::WhenStrikes)))
        {
            int castCost = getEffectiveEnchantmentCastCost(static_cast<float>(enchantment->mData.mCost), mCaster);

            if (item.getCellRef().getEnchantmentCharge() == -1)
                item.getCellRef().setEnchantmentCharge(static_cast<float>(enchantment->mData.mCharge));

            if (item.getCellRef().getEnchantmentCharge() < castCost)
            {
                if (mCaster == getPlayer())
                {
                    MWBase::Environment::get().getWindowManager()->messageBox("#{sMagicInsufficientCharge}");

                    // Failure sound
                    int school = 0;
                    if (!enchantment->mEffects.mList.empty())
                    {
                        short effectId = enchantment->mEffects.mList.front().mEffectID;
                        const ESM3::MagicEffect* magicEffect = MWBase::Environment::get().getWorld()->getStore().get<ESM3::MagicEffect>().find(effectId);
                        school = magicEffect->mData.mSchool;
                    }

                    static const std::string schools[] = {
                        "alteration", "conjuration", "destruction", "illusion", "mysticism", "restoration"
                    };
                    MWBase::SoundManager *sndMgr = MWBase::Environment::get().getSoundManager();
                    sndMgr->playSound3D(mCaster, "Spell Failure " + schools[school], 1.0f, 1.0f);
                }
                return false;
            }
            // Reduce charge
            item.getCellRef().setEnchantmentCharge(item.getCellRef().getEnchantmentCharge() - castCost);
        }

        if (type == ESM3::Enchantment::WhenUsed)
        {
            if (mCaster == getPlayer())
                mCaster.getClass().skillUsageSucceeded (mCaster, ESM3::Skill::Enchant, 1);
        }
        else if (type == ESM3::Enchantment::CastOnce)
        {
            if (!godmode)
                item.getContainerStore()->remove(item, 1, mCaster);
        }
        else if (type == ESM3::Enchantment::WhenStrikes)
        {
            if (mCaster == getPlayer())
                mCaster.getClass().skillUsageSucceeded (mCaster, ESM3::Skill::Enchant, 3);
        }

        inflict(mCaster, mCaster, enchantment->mEffects, ESM::RT_Self);

        if (isProjectile || !mTarget.isEmpty())
            inflict(mTarget, mCaster, enchantment->mEffects, ESM::RT_Touch);

        if (launchProjectile)
            launchMagicBolt();
        else if (isProjectile || !mTarget.isEmpty())
            inflict(mTarget, mCaster, enchantment->mEffects, ESM::RT_Target);

        return true;
    }

    bool CastSpell::cast(const ESM3::Potion* potion)
    {
        mSourceName = potion->mName;
        mId = potion->mId;
        mStack = true;

        inflict(mCaster, mCaster, potion->mEffects, ESM::RT_Self);

        return true;
    }

    bool CastSpell::cast(const ESM3::Spell* spell)
    {
        mSourceName = spell->mName;
        mId = spell->mId;
        mStack = false;

        const MWWorld::ESMStore& store = MWBase::Environment::get().getWorld()->getStore();

        int school = 0;

        bool godmode = mCaster == MWMechanics::getPlayer() && MWBase::Environment::get().getWorld()->getGodModeState();

        if (mCaster.getClass().isActor() && !mAlwaysSucceed && !mManualSpell)
        {
            school = getSpellSchool(spell, mCaster);

            CreatureStats& stats = mCaster.getClass().getCreatureStats(mCaster);

            if (!godmode)
            {
                // Reduce fatigue (note that in the vanilla game, both GMSTs are 0, and there's no fatigue loss)
                static const float fFatigueSpellBase = store.get<ESM3::GameSetting>().find("fFatigueSpellBase")->mValue.getFloat();
                static const float fFatigueSpellMult = store.get<ESM3::GameSetting>().find("fFatigueSpellMult")->mValue.getFloat();
                DynamicStat<float> fatigue = stats.getFatigue();
                const float normalizedEncumbrance = mCaster.getClass().getNormalizedEncumbrance(mCaster);

                float fatigueLoss = MWMechanics::calcSpellCost(*spell) * (fFatigueSpellBase + normalizedEncumbrance * fFatigueSpellMult);
                fatigue.setCurrent(fatigue.getCurrent() - fatigueLoss);
                stats.setFatigue(fatigue);

                bool fail = false;

                // Check success
                float successChance = getSpellSuccessChance(spell, mCaster, nullptr, true, false);
                if (Misc::Rng::roll0to99() >= successChance)
                {
                    if (mCaster == getPlayer())
                        MWBase::Environment::get().getWindowManager()->messageBox("#{sMagicSkillFail}");
                    fail = true;
                }

                if (fail)
                {
                    // Failure sound
                    static const std::string schools[] = {
                        "alteration", "conjuration", "destruction", "illusion", "mysticism", "restoration"
                    };

                    MWBase::SoundManager *sndMgr = MWBase::Environment::get().getSoundManager();
                    sndMgr->playSound3D(mCaster, "Spell Failure " + schools[school], 1.0f, 1.0f);
                    return false;
                }
            }

            // A power can be used once per 24h
            if (spell->mData.mType == ESM3::Spell::ST_Power)
                stats.getSpells().usePower(spell);
        }

        if (!mManualSpell && mCaster == getPlayer() && spellIncreasesSkill(spell))
            mCaster.getClass().skillUsageSucceeded(mCaster, spellSchoolToSkill(school), 0);

        // A non-actor doesn't play its spell cast effects from a character controller, so play them here
        if (!mCaster.getClass().isActor())
            playSpellCastingEffects(spell->mEffects.mList);

        inflict(mCaster, mCaster, spell->mEffects, ESM::RT_Self);

        if (!mTarget.isEmpty())
            inflict(mTarget, mCaster, spell->mEffects, ESM::RT_Touch);

        launchMagicBolt();

        return true;
    }

    bool CastSpell::cast (const ESM3::Ingredient* ingredient)
    {
        mId = ingredient->mId;
        mStack = true;
        mSourceName = ingredient->mName;

        ESM3::ENAMstruct effect;
        effect.mEffectID = ingredient->mData.mEffectID[0];
        effect.mSkill = ingredient->mData.mSkills[0];
        effect.mAttribute = ingredient->mData.mAttributes[0];
        effect.mRange = ESM::RT_Self;
        effect.mArea = 0;

        const MWWorld::ESMStore& store = MWBase::Environment::get().getWorld()->getStore();
        const auto magicEffect = store.get<ESM3::MagicEffect>().find(effect.mEffectID);
        const MWMechanics::CreatureStats& creatureStats = mCaster.getClass().getCreatureStats(mCaster);

        float x = (mCaster.getClass().getSkill(mCaster, ESM3::Skill::Alchemy) +
                    0.2f * creatureStats.getAttribute (ESM::Attribute::Intelligence).getModified()
                    + 0.1f * creatureStats.getAttribute (ESM::Attribute::Luck).getModified())
                    * creatureStats.getFatigueTerm();

        int roll = Misc::Rng::roll0to99();
        if (roll > x)
        {
            // "X has no effect on you"
            std::string message = store.get<ESM3::GameSetting>().find("sNotifyMessage50")->mValue.getString();
            message = Misc::StringUtils::format(message, ingredient->mName);
            MWBase::Environment::get().getWindowManager()->messageBox(message);
            return false;
        }

        float magnitude = 0;
        float y = roll / std::min(x, 100.f);
        y *= 0.25f * x;
        if (magicEffect->mData.mFlags & ESM3::MagicEffect::NoDuration)
            effect.mDuration = 1;
        else
            effect.mDuration = static_cast<int>(y);
        if (!(magicEffect->mData.mFlags & ESM3::MagicEffect::NoMagnitude))
        {
            if (!(magicEffect->mData.mFlags & ESM3::MagicEffect::NoDuration))
                magnitude = floor((0.05f * y) / (0.1f * magicEffect->mData.mBaseCost));
            else
                magnitude = floor(y / (0.1f * magicEffect->mData.mBaseCost));
            magnitude = std::max(1.f, magnitude);
        }
        else
            magnitude = 1;

        effect.mMagnMax = static_cast<int>(magnitude);
        effect.mMagnMin = static_cast<int>(magnitude);

        ESM3::EffectList effects;
        effects.mList.push_back(effect);

        inflict(mCaster, mCaster, effects, ESM::RT_Self);

        return true;
    }

    void CastSpell::playSpellCastingEffects(const std::string &spellid, bool enchantment)
    {
        const MWWorld::ESMStore& store = MWBase::Environment::get().getWorld()->getStore();
        if (enchantment)
        {
            if (const auto spell = store.get<ESM3::Enchantment>().search(spellid))
                playSpellCastingEffects(spell->mEffects.mList);
        }
        else
        {
            if (const auto spell = store.get<ESM3::Spell>().search(spellid))
                playSpellCastingEffects(spell->mEffects.mList);
        }
    }

    void CastSpell::playSpellCastingEffects(const std::vector<ESM3::ENAMstruct>& effects)
    {
        const MWWorld::ESMStore& store = MWBase::Environment::get().getWorld()->getStore();
        std::vector<std::string> addedEffects;
        for (const ESM3::ENAMstruct& effectData : effects)
        {
            const auto effect = store.get<ESM3::MagicEffect>().find(effectData.mEffectID);

            const ESM3::Static* castStatic;

            if (!effect->mCasting.empty())
                castStatic = store.get<ESM3::Static>().find (effect->mCasting);
            else
                castStatic = store.get<ESM3::Static>().find ("VFX_DefaultCast");

            // check if the effect was already added
            if (std::find(addedEffects.begin(), addedEffects.end(), "meshes\\" + castStatic->mModel) != addedEffects.end())
                continue;

            MWRender::Animation* animation = MWBase::Environment::get().getWorld()->getAnimation(mCaster);
            if (animation)
            {
                animation->addEffect("meshes\\" + castStatic->mModel, effect->mIndex, false, "", effect->mParticle);
            }
            else
            {
                // If the caster has no animation, add the effect directly to the effectManager
                // We must scale and position it manually
                float scale = mCaster.getCellRef().getScale();
                osg::Vec3f pos (mCaster.getRefData().getPosition().asVec3());
                if (!mCaster.getClass().isNpc())
                {
                    osg::Vec3f bounds (MWBase::Environment::get().getWorld()->getHalfExtents(mCaster) * 2.f);
                    scale *= std::max({bounds.x(), bounds.y(), bounds.z() / 2.f}) / 64.f;
                    float offset = 0.f;
                    if (bounds.z() < 128.f)
                        offset = bounds.z() - 128.f;
                    else if (bounds.z() < bounds.x() + bounds.y())
                        offset = 128.f - bounds.z();
                    if (MWBase::Environment::get().getWorld()->isFlying(mCaster))
                        offset /= 20.f;
                    pos.z() += offset * scale;
                }
                else
                {
                    // Additionally use the NPC's height
                    osg::Vec3f npcScaleVec (1.f, 1.f, 1.f);
                    mCaster.getClass().adjustScale(mCaster, npcScaleVec, true);
                    scale *= npcScaleVec.z();
                }
                scale = std::max(scale, 1.f);
                MWBase::Environment::get().getWorld()->spawnEffect("meshes\\" + castStatic->mModel, effect->mParticle, pos, scale);
            }

            if (animation && !mCaster.getClass().isActor())
                animation->addSpellCastGlow(effect);

            static const std::string schools[] = {
                "alteration", "conjuration", "destruction", "illusion", "mysticism", "restoration"
            };

            addedEffects.push_back("meshes\\" + castStatic->mModel);

            MWBase::SoundManager *sndMgr = MWBase::Environment::get().getSoundManager();
            if(!effect->mCastSound.empty())
                sndMgr->playSound3D(mCaster, effect->mCastSound, 1.0f, 1.0f);
            else
                sndMgr->playSound3D(mCaster, schools[effect->mData.mSchool]+" cast", 1.0f, 1.0f);
        }
    }
}
