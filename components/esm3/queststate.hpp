#ifndef ESM3_QUESTSTATE_H
#define ESM3_QUESTSTATE_H

#include <string>

namespace ESM
{
    class ESMWriter;
}

namespace ESM3
{
    class Reader;

    // format 0, saved games only

    struct QuestState
    {
        std::string mTopic; // lower case id
        int mState;
        unsigned char mFinished;

        void load (Reader& esm);
        void save (ESM::ESMWriter& esm) const;
    };
}

#endif
