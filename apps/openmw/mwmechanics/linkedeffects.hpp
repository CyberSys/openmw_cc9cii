#ifndef MWMECHANICS_LINKEDEFFECTS_H
#define MWMECHANICS_LINKEDEFFECTS_H

#include <string>

namespace ESM3
{
    struct ActiveEffect;
    struct EffectList;
    struct ENAMstruct;
    struct MagicEffect;
    struct Spell;
}

namespace MWWorld
{
    class Ptr;
}

namespace MWMechanics
{

    // Try to reflect a spell effect. If it's reflected, it's also put into the passed reflected effects list.
    bool reflectEffect(const ESM3::ENAMstruct& effect, const ESM3::MagicEffect* magicEffect,
                       const MWWorld::Ptr& caster, const MWWorld::Ptr& target, ESM3::EffectList& reflectedEffects);

    // Try to absorb a stat (skill, attribute, etc.) from the target and transfer it to the caster.
    void absorbStat(const ESM3::ENAMstruct& effect, const ESM3::ActiveEffect& appliedEffect,
                    const MWWorld::Ptr& caster, const MWWorld::Ptr& target, bool reflected, const std::string& source);
}

#endif
