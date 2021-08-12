#ifndef OPENMW_ESSIMPORT_CONVERTACDT_H
#define OPENMW_ESSIMPORT_CONVERTACDT_H

#include <components/esm3/creaturestats.hpp>
#include <components/esm3/npcstats.hpp>
#include <components/esm3/skil.hpp>
#include <components/esm3/animationstate.hpp>

#include "importacdt.hpp"

namespace ESSImport
{

    // OpenMW uses Health,Magicka,Fatigue, MW uses Health,Fatigue,Magicka
    int translateDynamicIndex(int mwIndex);


    void convertACDT (const ACDT& acdt, ESM3::CreatureStats& cStats);
    void convertACSC (const ACSC& acsc, ESM3::CreatureStats& cStats);

    void convertNpcData (const ActorData& actorData, ESM3::NpcStats& npcStats);

    void convertANIS (const ANIS& anis, ESM3::AnimationState& state);
}

#endif
