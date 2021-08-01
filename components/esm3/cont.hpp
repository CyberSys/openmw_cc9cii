#ifndef ESM3_CONT_H
#define ESM3_CONT_H

#include <string>
#include <vector>

//#include "esmcommon.hpp"

namespace ESM
{
    class ESMWriter;
}

namespace ESM3
{
    class Reader;

    /*
     * Container definition
     */

    struct ContItem
    {
        std::uint32_t mCount;
        std::string mItem;
    };

    /// InventoryList, NPCO subrecord
    struct InventoryList
    {
        std::vector<ContItem> mList;

        /// Load one item, assumes subrecord name is already read
        void add(Reader& reader);

        void save(ESM::ESMWriter& esm) const;
    };

    struct Container
    {
        static unsigned int sRecordId;
        /// Return a string descriptor for this record type. Currently used for debugging / error logs only.
        static std::string getRecordType() { return "Container"; }

        enum Flags
        {
            Organic = 1, // Objects cannot be placed in this container
            Respawn = 2, // Respawns after 4 months
            Unknown = 8
        };

        unsigned int mRecordFlags;
        std::string mId, mName, mModel, mScript;

        float mWeight; // Not sure, might be max total weight allowed?
        int mFlags;
        InventoryList mInventory;

        void load(Reader& reader, bool& isDeleted);
        void save(ESM::ESMWriter& esm, bool isDeleted = false) const;

        void blank();
        ///< Set record to default state (does not touch the ID).
    };
}
#endif
