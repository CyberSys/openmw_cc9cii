#ifndef OPENMW_AUTOCALCSPELL_H
#define OPENMW_AUTOCALCSPELL_H

#include <string>
#include <vector>

namespace ESM3
{
    struct Spell;
    struct Race;
}

namespace MWMechanics
{

/// Contains algorithm for calculating an NPC's spells based on stats
/// @note We might want to move this code to a component later, so the editor can use it for preview purposes

std::vector<std::string> autoCalcNpcSpells(const unsigned int* actorSkills, const unsigned int* actorAttributes, const ESM3::Race* race);

std::vector<std::string> autoCalcPlayerSpells(const unsigned int* actorSkills, const unsigned int* actorAttributes, const ESM3::Race* race);

// Helpers

bool attrSkillCheck (const ESM3::Spell* spell, const unsigned int* actorSkills, const unsigned int* actorAttributes);

void calcWeakestSchool(const ESM3::Spell* spell, const unsigned int* actorSkills, int& effectiveSchool, float& skillTerm);

float calcAutoCastChance(const ESM3::Spell* spell, const unsigned int* actorSkills, const unsigned int* actorAttributes, int effectiveSchool);

}

#endif
