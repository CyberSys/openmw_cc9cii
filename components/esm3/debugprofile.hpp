#ifndef ESM3_DEBUGPROFILE_H
#define ESM3_DEBUGPROFILE_H

#include <string>

namespace ESM
{
    class ESMWriter;
}

namespace ESM3
{
    class Reader;

    struct DebugProfile
    {
        static unsigned int sRecordId;

        enum Flags
        {
            Flag_Default = 1,       // add to newly opened scene subviews
            Flag_BypassNewGame = 2, // bypass regular game startup
            Flag_Global = 4         // make available from main menu (i.e. not location specific)
        };

        std::string mId;

        std::string mDescription;

        std::string mScriptText;

        unsigned int mFlags;

        void load (Reader& reader, bool &isDeleted);
        void save (ESM::ESMWriter& esm, bool isDeleted = false) const;

        /// Set record to default state (does not touch the ID).
        void blank();
    };
}

#endif
