#include "dialoguestate.hpp"

#include "reader.hpp"
#include "../esm/esmwriter.hpp"

void ESM3::DialogueState::load (Reader& esm)
{
    while (esm.getNextSubRecordType() == ESM3::SUB_TOPI && esm.getSubRecordHeader())
    {
        std::string topic;
        esm.getString(topic); // NOTE: string not null terminated
        mKnownTopics.push_back (topic);
    }

    while (esm.getNextSubRecordType() == ESM3::SUB_FACT && esm.getSubRecordHeader())
    {
        std::string faction;
        esm.getString(faction); // NOTE: string not null terminated

        while (esm.getNextSubRecordType() == ESM3::SUB_REA2 && esm.getSubRecordHeader())
        {
            std::string faction2;
            esm.getString(faction2); // NOTE: string not null terminated

            int reaction;
            esm.getSubRecordHeader();
            assert(esm.subRecordHeader().typeId == ESM3::SUB_INTV);
            esm.get(reaction);
            mChangedFactionReaction[faction][faction2] = reaction;
        }

        // no longer used
        while (esm.getNextSubRecordType() == ESM3::SUB_REAC && esm.getSubRecordHeader())
        {
            esm.skipSubRecordData();
            esm.getSubRecordHeader();
            esm.skipSubRecordData();
        }
    }
}

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
