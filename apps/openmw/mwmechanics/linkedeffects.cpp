#include "linkedeffects.hpp"

#include <components/misc/rng.hpp>
#include <components/settings/settings.hpp>

#include "../mwbase/environment.hpp"
#include "../mwbase/world.hpp"

#include "../mwrender/animation.hpp"

#include "../mwworld/class.hpp"
#include "../mwworld/esmstore.hpp"

#include "creaturestats.hpp"

namespace MWMechanics
{

    bool reflectEffect(const ESM3::ENAMstruct& effect, const ESM3::MagicEffect* magicEffect,
                       const MWWorld::Ptr& caster, const MWWorld::Ptr& target, ESM3::EffectList& reflectedEffects)
    {
        if (caster.isEmpty() || caster == target || !target.getClass().isActor())
            return false;

        bool isHarmful = magicEffect->mData.mFlags & ESM3::MagicEffect::Harmful;
        bool isUnreflectable = magicEffect->mData.mFlags & ESM3::MagicEffect::Unreflectable;
        if (!isHarmful || isUnreflectable)
            return false;

        float reflect = target.getClass().getCreatureStats(target).getMagicEffects().get(ESM3::MagicEffect::Reflect).getMagnitude();
        if (Misc::Rng::roll0to99() >= reflect)
            return false;

        const ESM3::Static* reflectStatic = MWBase::Environment::get().getWorld()->getStore().get<ESM3::Static>().find ("VFX_Reflect");
        MWRender::Animation* animation = MWBase::Environment::get().getWorld()->getAnimation(target);
        if (animation && !reflectStatic->mModel.empty())
            animation->addEffect("meshes\\" + reflectStatic->mModel, ESM3::MagicEffect::Reflect, false, std::string());
        reflectedEffects.mList.emplace_back(effect);
        return true;
    }

    void absorbStat(const ESM3::ENAMstruct& effect, const ESM3::ActiveEffect& appliedEffect,
                    const MWWorld::Ptr& caster, const MWWorld::Ptr& target, bool reflected, const std::string& source)
    {
        if (caster.isEmpty() || caster == target)
            return;

        if (!target.getClass().isActor() || !caster.getClass().isActor())
            return;

        // Make sure callers don't do something weird
        if (effect.mEffectID < ESM3::MagicEffect::AbsorbAttribute || effect.mEffectID > ESM3::MagicEffect::AbsorbSkill)
            throw std::runtime_error("invalid absorb stat effect");

        if (appliedEffect.mMagnitude == 0)
            return;

        std::vector<ActiveSpells::ActiveEffect> absorbEffects;
        ActiveSpells::ActiveEffect absorbEffect = appliedEffect;
        absorbEffect.mMagnitude *= -1;
        absorbEffect.mEffectIndex = appliedEffect.mEffectIndex;
        absorbEffects.emplace_back(absorbEffect);

        // Morrowind negates reflected Absorb spells so the original caster won't be harmed.
        if (reflected && Settings::Manager::getBool("classic reflected absorb spells behavior", "Game"))
        {
            target.getClass().getCreatureStats(target).getActiveSpells().addSpell(std::string(), true,
                            absorbEffects, source, caster.getClass().getCreatureStats(caster).getActorId());
            return;
        }

        caster.getClass().getCreatureStats(caster).getActiveSpells().addSpell(std::string(), true,
                        absorbEffects, source, target.getClass().getCreatureStats(target).getActorId());
    }
}
