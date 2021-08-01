#include "reader.hpp"

#ifdef NDEBUG // FIXME: debugging only
#undef NDEBUG
#endif

#include <cassert>
#include <stdexcept>

namespace ESM3
{

Reader::Reader(Files::IStreamPtr esmStream, const std::string& filename)
{
    open(esmStream, filename);
}

// FIXME: only required by ESM3::Land
Reader::Reader() : mEncoder(nullptr), mFileSize(0), mGlobalReaderList(nullptr)
{
    clearCtx();
}

void Reader::openRaw(Files::IStreamPtr _esm, const std::string& name)
{
    close();
    mStream = _esm;

    mCtx.fileRead = 0;
    mCtx.filePos = 0; // FIXME: for temp testing only
    mCtx.filename = name;
    mStream->seekg(0, mStream->end);
    mFileSize = mStream->tellg();
    mStream->seekg(0, mStream->beg);
}

void Reader::open(Files::IStreamPtr _esm, const std::string &name)
{
    openRaw(_esm, name);

    getRecordHeader();
    if (mCtx.recordHeader.typeId == ESM3::REC_TES3)
    {
        mHeader.load(*this);
    }
    else
        fail("Not a valid Morrowind file");
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

    // Copy the data
    mCtx = rc;

    // Make sure we seek to the right place
    mStream->seekg(mCtx.filePos);
}

void Reader::clearCtx()
{
   mCtx.filename.clear();
   mCtx.modIndex = 0;
   mCtx.fileRead = 0;
   mCtx.fileRead = 0;
   mCtx.recordRead = 0;
   mCtx.index = 0;
   mCtx.parentFileIndices.clear();
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
    bool result = false;

    assert(mCtx.recordRead <= mCtx.recordHeader.dataSize && "Read more from the stream than the record size.");
    /*if (mCtx.recordRead > mCtx.recordHeader.dataSize)
    {
        // NOTE: only works if the offending sub-record is the last one

        // try to correct any overshoot, seek to the end of the expected data
        // this will only work if mCtx.subRecordHeader.dataSize was fully read or skipped
        // (i.e. it will only correct mCtx.subRecordHeader.dataSize being incorrect)
        std::uint32_t overshoot = (std::uint32_t)mCtx.recordRead - mCtx.recordHeader.dataSize;

        std::size_t pos = mStream->tellg();
        mStream->seekg(pos - overshoot);

        return false;
    }
    else */if (mCtx.recordHeader.dataSize - mCtx.recordRead >= sizeof(SubRecordHeader))
    {
        result = getExact(mCtx.subRecordHeader);
        assert (mStream->gcount() == sizeof(mCtx.subRecordHeader));

        // NOTE: Fragile code below! Assumes sub-record data will be read or skipped in full.
        //       It aims to avoid updating mCtx.recordRead each time anything is read.
        mCtx.recordRead += (sizeof(SubRecordHeader) + mCtx.subRecordHeader.dataSize);

        assert(mCtx.subRecordHeader.typeId > MKTAG('A', 'A', 'A', '0') &&
               mCtx.subRecordHeader.typeId < MKTAG('Z', 'Z', 'Z', '_') &&
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

void Reader::skipRecordData()
{
    assert (mCtx.recordRead <= mCtx.recordHeader.dataSize && "Skipping after reading more than available");
    mStream->ignore(mCtx.recordHeader.dataSize - mCtx.recordRead);
    mCtx.recordRead = mCtx.recordHeader.dataSize; // for getSubRecordHeader()
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
