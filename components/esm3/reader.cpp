#include "reader.hpp"

//#ifdef NDEBUG // FIXME: debugging only
//#undef NDEBUG
//#endif

#include <cassert>
#include <stdexcept>

namespace ESM3
{

Reader::Reader(Files::IStreamPtr esmStream, const std::string& filename)
{
    open(esmStream, filename);
}

// FIXME: only required by ESM3::Land
Reader::Reader() : mEncoder(nullptr), mFileSize(0)
{
    clearCtx();
}

void Reader::openRaw(Files::IStreamPtr esmStream, const std::string& filename)
{
    close();
    mStream = esmStream;

    mCtx.fileRead = 0;
    mCtx.filePos = 0; // FIXME: for temp testing only
    mCtx.filename = filename;
    mStream->seekg(0, mStream->end);
    mFileSize = mStream->tellg();
    mStream->seekg(0, mStream->beg);
}

void Reader::open(Files::IStreamPtr esmStream, const std::string &filename)
{
    openRaw(esmStream, filename);

    getRecordHeader();
    if (mCtx.recordHeader.typeId == ESM3::REC_TES3)
    {
        mHeader.load(*this);
    }
    else
        fail("Not a valid Morrowind file");
}

// FIXME: redundant but ESMTool uses it
void Reader::open(const std::string &filename)
{
    Files::IStreamPtr esmStream(Files::openConstrainedFileStream (filename.c_str ()));

    open(esmStream, filename);
}

// FIXME: redundant but ESMTool uses it
void Reader::openRaw(const std::string& filename)
{
    openRaw(Files::openConstrainedFileStream(filename.c_str()), filename);
}

void Reader::close()
{
    mStream.reset();
    clearCtx();
    mHeader.blank();
}

ReaderContext Reader::getContext()
{
    // Update the file position before returning
    mCtx.filePos = mStream->tellg();
    return mCtx;
}

void Reader::restoreContext(const ReaderContext &rc)
{
    // Reopen the file if necessary
    if (mCtx.filename != rc.filename)
        openRaw(rc.filename);

    //bool reSeek = mCtx.filePos != rc.subHdrCached ? rc.filePos - sizeof(SubRecordHeader) : rc.filePos;

    // Copy the data
    mCtx = rc;

    if (mCtx.subHdrCached)
    {
        mCtx.subHdrCached = false;
        mCtx.recordRead -= sizeof(SubRecordHeader) + mCtx.subRecordHeader.dataSize;
        mCtx.filePos -= sizeof(SubRecordHeader);
    }

    // Make sure we seek to the right place
    //if (reSeek)
        mStream->seekg(mCtx.filePos);
}

void Reader::clearCtx()
{
   mCtx.filename.clear();
   mCtx.modIndex = 0;
   mCtx.parentFileIndices.clear();
   mCtx.fileRead = 0;
   mCtx.fileRead = 0;
   mCtx.recordRead = 0;
   mCtx.subHdrTypeRead = false;
   mCtx.subHdrCached = false;
}

bool Reader::getRecordHeader()
{
    mStream->read((char*)&mCtx.recordHeader, sizeof(RecordHeader));
    std::size_t bytesRead = (std::size_t)mStream->gcount();

    //mCtx.filePos = mStream->tellg(); // FIXME: temp testing only

    // NOTE: Fragile code below! Assumes record data will be read or skipped in full.
    //       It aims to avoid updating mCtx.fileRead each time anything is read.
    //       Also allows methods to get sub-records using another reader while still
    //       keeping trck of how much of the file has been read (see Land::loadData())
    mCtx.fileRead += (sizeof(RecordHeader) + mCtx.recordHeader.dataSize);

    mCtx.recordRead = 0; // for keeping track of sub records

    return bytesRead == sizeof(RecordHeader);
}

bool Reader::getSubRecordHeader()
{
    if (mCtx.subHdrCached)
    {
        mCtx.subHdrCached = false;
        return true;
    }

    bool result = false;

    //mCtx.filePos = mStream->tellg(); // FIXME: temp testing only

    assert(mCtx.recordRead <= mCtx.recordHeader.dataSize && "Read more from the stream than the record size.");
    if (mCtx.recordHeader.dataSize - mCtx.recordRead >= sizeof(SubRecordHeader))
    {
        // not happy about it but there are too many instances of existing code that rely on this
        if (mCtx.subHdrTypeRead)
        {
            get(mCtx.subRecordHeader.dataSize);
            result = (mStream->gcount() == sizeof(mCtx.subRecordHeader.dataSize));
        }
        else
        {
            result = getExact(mCtx.subRecordHeader);
            //assert (mStream->gcount() == sizeof(mCtx.subRecordHeader));
        }

        mCtx.subHdrTypeRead = false;

        // NOTE: Fragile code below! Assumes sub-record data will be read or skipped in full.
        //       It aims to avoid updating mCtx.recordRead each time anything is read.
        mCtx.recordRead += (sizeof(SubRecordHeader) + mCtx.subRecordHeader.dataSize);

        assert(mCtx.subRecordHeader.typeId > MKTAG('A', 'A', 'A', '0') &&
               mCtx.subRecordHeader.typeId < MKTAG('Z', 'Z', '_', '_') && // "ID__"
               "Unlikely sub-record type detected");

        // clamp any overrun (only works if the offending sub-record is the last one)
        if (mCtx.recordRead > mCtx.recordHeader.dataSize)
        {
            mCtx.subRecordHeader.dataSize -= (mCtx.recordRead - mCtx.recordHeader.dataSize);
            mCtx.recordRead = mCtx.recordHeader.dataSize;
        }
    }

    return result;
}

std::uint32_t Reader::getNextSubRecordType()
{
    if (!mCtx.subHdrTypeRead && !mCtx.subHdrCached)
    {
        // return null if there aren't any nore sub-records
        if (mCtx.recordHeader.dataSize - mCtx.recordRead < sizeof(SubRecordHeader))
            return 0;

        // read the sub-record type only
        get(mCtx.subRecordHeader, 4);
        mCtx.subHdrTypeRead = true;
    }

    return mCtx.subRecordHeader.typeId;
}

void Reader::skipRecordData()
{
    assert (mCtx.recordRead <= mCtx.recordHeader.dataSize && "Skipping after reading more than available");
    std::size_t recordRead = mCtx.recordRead;
    if (mCtx.subHdrCached)
    {
        mCtx.subHdrCached = false;
        recordRead -= mCtx.subRecordHeader.dataSize;
    }

    mStream->ignore(mCtx.recordHeader.dataSize - recordRead);
    mCtx.recordRead = mCtx.recordHeader.dataSize; // for getSubRecordHeader()
}

void Reader::cacheSubRecordHeader()
{
    mCtx.subHdrCached = true;
}

[[noreturn]] void Reader::fail(const std::string &msg)
{
    std::stringstream ss;

    ss << "ESM Error: " << msg;
    ss << "\n  File: " << mCtx.filename;
    ss << "\n  Record: " << ESM::printName(mCtx.recordHeader.typeId);
    ss << "\n  Subrecord: " << ESM::printName(mCtx.subRecordHeader.typeId);
    if (mStream.get())
        ss << "\n  Offset: 0x" << std::hex << mStream->tellg();
    throw std::runtime_error(ss.str());
}

}
