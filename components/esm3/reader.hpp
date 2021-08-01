#ifndef ESM3_READER_H
#define ESM3_READER_H

#include <cstdint>
#include <cassert>
#include <vector>
#include <sstream>

#include <components/to_utf8/to_utf8.hpp>
#include <components/files/constrainedfilestream.hpp>

#include "common.hpp"
#include "tes3.hpp"
#include "../esm/reader.hpp"

namespace ESM3
{
    struct ReaderContext
    {
        std::string     filename;         // in case we need to reopen to restore the context
        std::uint32_t   modIndex;         // the sequential position of this file in the load order:
                                          //  0x00 reserved, 0xFF in-game

        std::size_t     filePos;          // File position. Only used for stored contexts, not regularly
                                          // updated within the reader itself.

        // for keeping track of things
        std::size_t     fileRead;         // number of bytes read, incl. the current record

        RecordHeader    recordHeader;     // header of the current record or group being processed
        SubRecordHeader subRecordHeader;  // header of the current sub record being processed
        std::uint32_t   recordRead;       // bytes read from the sub records, incl. the current one

        // When working with multiple esX files, we will generate lists of all files that
        //  actually contribute to a specific cell. Therefore, we need to store the index
        //  of the file belonging to this contest. See CellStore::(list/load)refs for details.
        int index;
        std::vector<int> parentFileIndices;
    };

    class Reader : public ESM::Reader
    {
        ESM3::Header         mHeader;     // ESM3 header

        ReaderContext        mCtx;

        ToUTF8::Utf8Encoder* mEncoder;

        size_t mFileSize;

        Files::IStreamPtr    mStream;

        std::vector<Reader> *mGlobalReaderList;

        /// Raw opening. Opens the file and sets everything up but doesn't
        /// parse the header.
        void openRaw(Files::IStreamPtr _esm, const std::string &name);

        /// Load ES file from a new stream, parses the header. Closes the
        /// currently open file first, if any.
        void open(Files::IStreamPtr _esm, const std::string &name);

        void clearCtx();

        [[noreturn]] void reportSubSizeMismatch(size_t want, size_t got) {
                fail("record size mismatch, requested " +
                        std::to_string(want) +
                        ", got" +
                        std::to_string(got));
        }

    public:

        Reader(Files::IStreamPtr esmStream, const std::string& filename);
        Reader(); // FIXME: public as loadland.cpp uses it
        ~Reader() {}

        // FIXME: should be private but ESMTool uses it
        void openRaw(const std::string &filename);

        /** Close the file, resets all information. After calling close()
            the structure may be reused to load a new file.
        */
        void close() final;

        inline bool isEsm4() const final { return false; }

        /// Sets font encoder for ESM strings
        inline void setEncoder(ToUTF8::Utf8Encoder* encoder) final { mEncoder = encoder; };

        inline const std::vector<ESM::MasterData> &getGameFiles() const final { return mHeader.mMaster; }

        inline int getRecordCount() const final { return mHeader.mData.records; }
        inline const std::string getAuthor() const final { return mHeader.mData.author; }
        inline int getFormat() const final { return mHeader.mFormat; };
        inline const std::string getDesc() const final { return mHeader.mData.desc; }

        std::string getName() const { return mCtx.filename; }; // used by ESM::CellRef for debugging
        //const ESM3::Header& getHeader() const { return mHeader; }
        inline unsigned int esmVersion() const { return mHeader.mData.version.ui; }
        inline unsigned int numRecords() const { return mHeader.mData.records; }

        inline bool hasMoreRecs() const final { return (mFileSize - mCtx.fileRead) > 0; }

        size_t getFileSize() const { return mFileSize; }

        /// Get the current position in the file. Make sure that the file has been opened!
        size_t getFileOffset() const { return mStream->tellg(); }; // only used for debug logging

        /** Save the current file position and information in a ESM_Context
            struct
         */
        ReaderContext getContext();

        /** Restore a previously saved context */
        void restoreContext(const ReaderContext &rc);

        // Read x bytes of header. The caller can then decide whether to process or skip the data.
        bool getRecordHeader();

        inline const RecordHeader& hdr() const { return mCtx.recordHeader; }

        /// Get record flags of last record
        unsigned int getRecordFlags() { return mCtx.recordHeader.flags; }

        // Skip the data part of a record
        // Note: assumes the header was read correctly (partial skip is allowed)
        void skipRecordData();

        // Used by Variant and Cell
        // NOTE: mCtx.recordRead is updated when sub-record header is read
        inline bool hasMoreSubs() { return mCtx.recordRead < mCtx.recordHeader.dataSize; }


        // Read x bytes of header. The caller can then decide whether to process or skip the data.
        bool getSubRecordHeader();

        // Skip the data part of a subrecord
        // Note: assumes the header was read correctly and nothing else was read
        void skipSubRecordData() { mStream->ignore(mCtx.subRecordHeader.dataSize); }

        void skipSubRecordData(std::uint32_t size) { mStream->ignore(size); }

        inline const SubRecordHeader& subRecordHeader() const { return mCtx.subRecordHeader; }

        template<typename T>
        void get(T& t, std::size_t size = sizeof(T)) { mStream->read((char*)&t, size); }

        template<typename T>
        bool getExact(T& t) {
            mStream->read((char*)&t, sizeof(T));
            return mStream->gcount() == sizeof(T); // FIXME: try/catch block needed?
        }

        bool getZString(std::string& str) {
            return getStringImpl(str, mCtx.subRecordHeader.dataSize, mStream, mEncoder, true);
        }

        bool getString(std::string& str) {
            return getStringImpl(str, mCtx.subRecordHeader.dataSize, mStream, mEncoder);
        }

        // This is a quick hack for multiple esm/esp files. Each plugin introduces its own
        //  terrain palette, but Reader does not pass a reference to the correct plugin
        //  to the individual load() methods. This hack allows to pass this reference
        //  indirectly to the load() method.
        void setIndex(const int index) { mCtx.index = index;}
        int getIndex() {return mCtx.index;}

        // used by ESMStore to track dependencies
        // see 2175f13b67e9075455bc944b0e32cdd0cb9b5ceb
        //     896ab44d1e919852aae03be9ecb71378f031b6f5
        void setGlobalReaderList(std::vector<Reader> *list) {mGlobalReaderList = list;}
        std::vector<Reader> *getGlobalReaderList() {return mGlobalReaderList;}

        void addParentFileIndex(int index) { mCtx.parentFileIndices.push_back(index); }
        const std::vector<int>& getParentFileIndices() const { return mCtx.parentFileIndices; } // used by ESM3::Cell

        // FIXME: for testing only
        //bool checkReadFile() { return (mCtx.readFile != mCtx.filePos+mCtx.recordHeader.dataSize); }

        /// Used for error handling
        [[noreturn]] void fail(const std::string &msg);
    };
}
#endif // ESM3_READER_H
