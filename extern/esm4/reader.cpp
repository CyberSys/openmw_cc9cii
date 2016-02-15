/*
  Copyright (C) 2015, 2016 cc9cii

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  cc9cii cc9c@iinet.net.au

*/
#include "reader.hpp"

#include <cassert>
#include <stdexcept>
#include <unordered_map>

#include <iostream> // FIXME: debugging only
#ifdef NDEBUG // FIXME: debugging only
#undef NDEBUG
#endif

#include <zlib.h>

#include <components/misc/stringops.hpp>

#include <components/esm/esm4reader.hpp>

#include "formid.hpp"

ESM4::Reader::Reader() : mModIndex(0), mObserver(nullptr), mEndOfRecord(0), mCellGridValid(false),
                         mRecHeaderSize(sizeof(ESM4::RecordHeader))
{
    mInBuf.reset();
    mDataBuf.reset();
    mStream.setNull();
    mSavedStream.setNull();
}

ESM4::Reader::~Reader()
{
}

std::size_t ESM4::Reader::openTes4File(Ogre::DataStreamPtr stream, const std::string& name)
{
    mStream = stream;
    return mStream->size();
}

void ESM4::Reader::setRecHeaderSize(const std::size_t size)
{
    mRecHeaderSize = size;
}

void ESM4::Reader::registerForUpdates(ESM4::ReaderObserver *observer)
{
    mObserver = observer;
}

bool ESM4::Reader::getRecordHeader()
{
    // FIXME: this seems hacky
    if (/*mStream->eof() && */!mSavedStream.isNull())
    {
        mStream = mSavedStream;
        mSavedStream.setNull();
    }

    // keep track of data left to read from the file
    mObserver->update(mRecHeaderSize);

    return (mStream->read(&mRecordHeader, mRecHeaderSize) == mRecHeaderSize
            && (mEndOfRecord = mStream->tell() + mRecordHeader.record.dataSize)); // for keeping track of sub records
}

bool ESM4::Reader::getSubRecordHeader()
{
    return (mStream->tell() < mEndOfRecord) && get(mSubRecordHeader);
}

void ESM4::Reader::updateModIndicies(const std::vector<std::string>& files)
{
    if (files.size() >= 0xff)
        throw std::runtime_error("ESM4::Reader::updateModIndicies too many files"); // 0xff is reserved

    // build a lookup map
    std::unordered_map<std::string, size_t> fileIndex;
    for (size_t i = 0; i < files.size(); ++i) // ATTENTION: assumes current file is not included
        fileIndex[Misc::StringUtils::lowerCase(files[i])] = i;

    mHeader.mModIndicies.resize(mHeader.mMaster.size());
    for (unsigned int i = 0; i < mHeader.mMaster.size(); ++i)
    {
        // locate the position of the dependency in already loaded files
        std::unordered_map<std::string, size_t>::const_iterator it
            = fileIndex.find(Misc::StringUtils::lowerCase(mHeader.mMaster[i].name));

        if (it != fileIndex.end())
            mHeader.mModIndicies[i] = (std::uint32_t)((it->second << 24) & 0xff000000);
        else
            throw std::runtime_error("ESM4::Reader::updateModIndicies required dependency file not loaded");
//#if 0
        std::cout << mHeader.mMaster[i].name << ", " << ESM4::formIdToString(mHeader.mModIndicies[i]) << std::endl;
//#endif
    }

    if (!mHeader.mModIndicies.empty() &&  mHeader.mModIndicies[0] != 0)
        throw std::runtime_error("ESM4::Reader::updateModIndicies base modIndex is not zero");
}

void ESM4::Reader::saveGroupStatus()
{
#if 0
    std::string padding = ""; // FIXME: debugging only
    padding.insert(0, mGroupStack.size()*2, ' ');
    std::cout << padding << "Starting record group "
              << ESM4::printLabel(mRecordHeader.group.label, mRecordHeader.group.type) << std::endl;
#endif
    if (mRecordHeader.group.groupSize == (std::uint32_t)mRecHeaderSize)
    {
#if 0
        std::cout << padding << "Igorning record group " // FIXME: debugging only
            << ESM4::printLabel(mRecordHeader.group.label, mRecordHeader.group.type)
            << " (empty)" << std::endl;
#endif
        if (!mGroupStack.empty()) // top group may be empty (e.g. HAIR in Skyrim)
        {
            // don't put on the stack, checkGroupStatus() may not get called before recursing into this method
            mGroupStack.back().second -= mRecordHeader.group.groupSize;
            checkGroupStatus();
        }
        return; // DLCMehrunesRazor - Unofficial Patch.esp is at EOF after one of these empty groups...
    }

    // push group
    mGroupStack.push_back(std::make_pair(mRecordHeader.group,
                mRecordHeader.group.groupSize - (std::uint32_t)mRecHeaderSize));
}

const ESM4::CellGrid& ESM4::Reader::currCellGrid() const
{
    // Maybe should throw an exception instead?
    assert(mCellGridValid && "Attempt to use an invalid cell grid");

    return mCurrCellGrid;
}

void ESM4::Reader::checkGroupStatus()
{
    // pop finished groups
    while (!mGroupStack.empty() && mGroupStack.back().second == 0)
    {
        ESM4::GroupTypeHeader grp = mGroupStack.back().first; // FIXME: debugging only
        uint32_t groupSize = mGroupStack.back().first.groupSize;
        mGroupStack.pop_back();
#if 0
        std::string padding = ""; // FIXME: debugging only
        padding.insert(0, mGroupStack.size()*2, ' ');
        std::cout << padding << "Finished record group " << ESM4::printLabel(grp.label, grp.type) << std::endl;
#endif
        // Check if the previous group was the final one
        if (mGroupStack.empty())
            return;

        assert (mGroupStack.back().second >= groupSize && "Read more records than available");
//#if 0
        if (mGroupStack.back().second < groupSize) // FIXME: debugging only
            std::cerr << ESM4::printLabel(mGroupStack.back().first.label, mGroupStack.back().first.type)
                      << " read more records than available" << std::endl;
//#endif
        mGroupStack.back().second -= groupSize;
    }
}

const ESM4::GroupTypeHeader& ESM4::Reader::grp(std::size_t pos) const
{
    assert(pos <= mGroupStack.size()-1 && "ESM4::Reader::grp - exceeded stack depth");

    return (*(mGroupStack.end()-pos-1)).first;
}

void ESM4::Reader::getRecordData()
{
    std::uint32_t bufSize = 0;

    if ((mRecordHeader.record.flags & ESM4::Rec_Compressed) != 0)
    {
        mInBuf.reset(new unsigned char[mRecordHeader.record.dataSize-(int)sizeof(bufSize)]);
        mStream->read(&bufSize, sizeof(bufSize));
        mStream->read(mInBuf.get(), mRecordHeader.record.dataSize-(int)sizeof(bufSize));
        mDataBuf.reset(new unsigned char[bufSize]);

        int ret;
        z_stream strm;
        strm.zalloc = Z_NULL;
        strm.zfree  = Z_NULL;
        strm.opaque = Z_NULL;
        strm.avail_in = bufSize;
        strm.next_in = mInBuf.get();
        ret = inflateInit(&strm);
        if (ret != Z_OK)
            throw std::runtime_error("ESM4::Reader::getRecordData - inflateInit failed");

        strm.avail_out = bufSize;
        strm.next_out = mDataBuf.get();
        ret = inflate(&strm, Z_NO_FLUSH);
        assert(ret != Z_STREAM_ERROR && "ESM4::Reader::getRecordData - inflate - state clobbered");
        switch (ret)
        {
        case Z_NEED_DICT:
            ret = Z_DATA_ERROR; /* and fall through */
        case Z_DATA_ERROR:
        case Z_MEM_ERROR:
            inflateEnd(&strm);
            throw std::runtime_error("ESM4::Reader::getRecordData - inflate failed");
        }
        assert(ret == Z_OK || ret == Z_STREAM_END);

    // For debugging only
#if 0
        std::ostringstream ss;
        for (unsigned int i = 0; i < bufSize; ++i)
        {
            if (mDataBuf[i] > 64 && mDataBuf[i] < 91)
                ss << (char)(mDataBuf[i]) << " ";
            else
                ss << std::setfill('0') << std::setw(2) << std::hex << (int)(mDataBuf[i]);
            if ((i & 0x000f) == 0xf)
                ss << "\n";
            else if (i < bufSize-1)
                ss << " ";
        }
        std::cout << ss.str() << std::endl;
#endif
        inflateEnd(&strm);

        mSavedStream = mStream;
        mStream = Ogre::DataStreamPtr(new Ogre::MemoryDataStream(mDataBuf.get(), bufSize, false, true));
    }

    // keep track of data left to read from the current group
    assert (!mGroupStack.empty() && "Read data for a record without a group");
    mGroupStack.back().second -= (std::uint32_t)mRecHeaderSize + mRecordHeader.record.dataSize;

    // keep track of data left to read from the file
    mObserver->update(mRecordHeader.record.dataSize);

    //std::cout << "data size 0x" << std::hex << mRecordHeader.record.dataSize << std::endl; // FIXME
}

// FIXME: how to without using a temp buffer?
bool ESM4::Reader::getZString(std::string& str)
{
    std::uint16_t size = mSubRecordHeader.dataSize; // assumed size from the header is correct

    boost::scoped_array<char> buf(new char[size]);
    if (mStream->read(buf.get(), size) == (size_t)size)
    {
        if (buf[size-1] != 0)
            std::cerr << "ESM4::Reader - string is not terminated with a zero" << std::endl;

        str.assign(buf.get(), size-1); // don't copy null terminator
        //assert((size_t)size-1 == str.size() && "ESM4::Reader - string size mismatch");
        return true;
    }
    else
    {
        str.clear();
        return false; // FIXME: throw instead?
    }
}

void ESM4::Reader::skipGroup()
{
#if 0
    std::string padding = ""; // FIXME: debugging only
    padding.insert(0, mGroupStack.size()*2, ' ');
    std::cout << padding << "Skipping record group "
              << ESM4::printLabel(mRecordHeader.group.label, mRecordHeader.group.type) << std::endl;
#endif
    // Note: subtract the size of header already read before skipping
    mStream->skip(mRecordHeader.group.groupSize - (std::uint32_t)mRecHeaderSize);

    // keep track of data left to read from the file
    mObserver->update((std::size_t)mRecordHeader.group.groupSize - mRecHeaderSize);
}

void ESM4::Reader::skipRecordData()
{
    mStream->skip(mRecordHeader.record.dataSize);

    // keep track of data left to read from the current group
    assert (!mGroupStack.empty() && "Skipping a record without a group");
    mGroupStack.back().second -= (std::uint32_t)mRecHeaderSize + mRecordHeader.record.dataSize;

    // keep track of data left to read from the file
    mObserver->update(mRecordHeader.record.dataSize);
}

void ESM4::Reader::skipSubRecordData()
{
    mStream->skip(mSubRecordHeader.dataSize);
}

void ESM4::Reader::skipSubRecordData(std::uint32_t size)
{
    mStream->skip(size);
}

// ModIndex adjusted formId according to master file dependencies
// (see http://www.uesp.net/wiki/Tes4Mod:FormID_Fixup)
// NOTE: need to update modindex to mModIndicies.size() before saving
void ESM4::Reader::adjustFormId(FormId& id)
{
    if (mHeader.mModIndicies.empty())
        return;

    unsigned int index = (id >> 24) & 0xff;

    if (index < mHeader.mModIndicies.size())
        id = mHeader.mModIndicies[index] | (id & 0x00ffffff);
    else
        id =  mModIndex | (id & 0x00ffffff);
}

bool ESM4::Reader::getFormId(FormId& id)
{
    if (!get(id))
        return false;

    adjustFormId(id);
    return true;
}

void ESM4::Reader::adjustGRUPFormId()
{
    adjustFormId(mRecordHeader.group.label.value);
}
