#ifndef ESM3_BOOK_H
#define ESM3_BOOK_H

#include <string>

namespace ESM
{
    class ESMWriter;
}

namespace ESM3
{
    class Reader;

    /*
     * Books, magic scrolls, notes and so on
     */

    struct Book
    {
        static unsigned int sRecordId;
        /// Return a string descriptor for this record type. Currently used for debugging / error logs only.
        static std::string getRecordType() { return "Book"; }

#pragma pack(push, 1)
        struct BKDTstruct
        {
            float mWeight;
            int mValue, mIsScroll, mSkillId, mEnchant;
        };
#pragma pack(pop)

        BKDTstruct mData;
        std::string mName, mModel, mIcon, mScript, mEnchant, mText;
        unsigned int mRecordFlags;
        std::string mId;

        void load(Reader& reader, bool& isDeleted);
        void save(ESM::ESMWriter& esm, bool isDeleted = false) const;

        void blank();
        ///< Set record to default state (does not touch the ID).
    };
}
#endif
