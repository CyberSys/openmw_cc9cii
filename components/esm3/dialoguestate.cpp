#include "dialoguestate.hpp"

#include "reader.hpp"
#include "../esm/esmwriter.hpp"

#if 0
void ESM3::DialogueState::load (Reader& esm)
{
    while (esm.isNextSub ("TOPI"))
        mKnownTopics.push_back (esm.getHString());

    while (esm.isNextSub ("FACT"))
    {
        std::string faction = esm.getHString();

        while (esm.isNextSub("REA2"))
        {
            std::string faction2 = esm.getHString();
            int reaction;
            esm.getHNT(reaction, "INTV");
            mChangedFactionReaction[faction][faction2] = reaction;
        }

        // no longer used
        while (esm.isNextSub ("REAC"))
        {
            esm.skipHSub();
            esm.getSubName();
            esm.skipHSub();
        }
    }
}
#endif
void ESM3::DialogueState::save (ESM::ESMWriter& esm) const
{
    for (std::vector<std::string>::const_iterator iter (mKnownTopics.begin());
        iter!=mKnownTopics.end(); ++iter)
    {
        esm.writeHNString ("TOPI", *iter);
    }

    for (std::map<std::string, std::map<std::string, int> >::const_iterator iter = mChangedFactionReaction.begin();
         iter != mChangedFactionReaction.end(); ++iter)
    {
        esm.writeHNString ("FACT", iter->first);

        for (std::map<std::string, int>::const_iterator reactIter = iter->second.begin();
             reactIter != iter->second.end(); ++reactIter)
        {
            esm.writeHNString ("REA2", reactIter->first);
            esm.writeHNT ("INTV", reactIter->second);
        }
    }
}
