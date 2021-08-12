#ifndef ESM3_DIALOGUESTATE_H
#define ESM3_DIALOGUESTATE_H

#include <string>
#include <vector>
#include <map>

namespace ESM
{
    class ESMWriter;
}

namespace ESM3
{
    class Reader;

    // format 0, saved games only

    struct DialogueState
    {
        // must be lower case topic IDs
        std::vector<std::string> mKnownTopics;

        // must be lower case faction IDs
        std::map<std::string, std::map<std::string, int> > mChangedFactionReaction;

        //void load (Reader& esm);
        void save (ESM::ESMWriter& esm) const;
    };
}

#endif
