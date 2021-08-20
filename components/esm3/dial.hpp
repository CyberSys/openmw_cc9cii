#ifndef ESM3_DIAL_H
#define ESM3_DIAL_H

#include <string>
#include <list>
#include <map>

#include "info.hpp"

namespace ESM
{
    class ESMWriter;
}

namespace ESM3
{
    class Reader;

    /*
     * Dialogue topic and journal entries. The actual data is contained in
     * the INFO records following the DIAL.
     */

    struct Dialogue
    {
        static unsigned int sRecordId;
        /// Return a string descriptor for this record type. Currently used for debugging / error logs only.
        static std::string getRecordType() { return "Dialogue"; }

        enum Type
        {
            Topic = 0,
            Voice = 1,
            Greeting = 2,
            Persuasion = 3,
            Journal = 4,
            Unknown = -1 // Used for deleted dialogues
        };

        std::string mId;
        signed char mType;

        typedef std::list<DialInfo> InfoContainer;

        // Parameters: Info ID, (Info iterator, Deleted flag)
        typedef std::map<std::string, std::pair<InfoContainer::iterator, bool> > LookupMap;

        InfoContainer mInfo;

        // This is only used during the loading phase to speed up DialInfo merging.
        LookupMap mLookup;

        void load(Reader& reader, bool& isDeleted);
        ///< Loads all sub-records of Dialogue record
        void loadId(Reader& reader);
        ///< Loads NAME sub-record of Dialogue record
        void loadData(Reader& reader, bool& isDeleted);
        ///< Loads all sub-records of Dialogue record, except NAME sub-record

        void save(ESM::ESMWriter& esm, bool isDeleted = false) const;

        /// Remove all INFOs that are deleted
        void clearDeletedInfos();

        /// Read the next info record
        /// @param merge Merge with existing list, or just push each record to the end of the list?
        void readInfo (Reader& reader, bool merge);

        void blank();
        ///< Set record to default state (does not touch the ID and does not change the type).
    };
}
#endif
