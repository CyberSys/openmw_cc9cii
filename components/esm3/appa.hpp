#ifndef ESM3_APPA_H
#define ESM3_APPA_H

#include <string>

namespace ESM
{
    class ESMWriter;
}

namespace ESM3
{
    class Reader;

    /*
     * Alchemist apparatus
     */

    struct Apparatus
    {
        static unsigned int sRecordId;
        /// Return a string descriptor for this record type. Currently used for debugging / error logs only.
        static std::string getRecordType() { return "Apparatus"; }

        enum AppaType
        {
            MortarPestle = 0,
            Alembic = 1,
            Calcinator = 2,
            Retort = 3
        };

#pragma pack(push, 1)
        struct AADTstruct
        {
            int mType;
            float mQuality;
            float mWeight;
            int mValue;
        };
#pragma pack(pop)

        AADTstruct mData;
        unsigned int mRecordFlags;
        std::string mId, mModel, mIcon, mScript, mName;

        void load(Reader& reader, bool& isDeleted);
        void save(ESM::ESMWriter& esm, bool isDeleted = false) const;

        void blank();
        ///< Set record to default state (does not touch the ID).
    };
}
#endif
