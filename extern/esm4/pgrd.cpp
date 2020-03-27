/*
  Copyright (C) 2020 cc9cii

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

  Much of the information on the data structures are based on the information
  from Tes4Mod:Mod_File_Format and Tes5Mod:File_Formats but also refined by
  trial & error.  See http://en.uesp.net/wiki for details.

*/
#include "pgrd.hpp"

#include <stdexcept>
#include <iostream> // FIXME: for debugging only
#include <iomanip>  // FIXME: for debugging only

#include "formid.hpp" // FIXME: for workaround
#include "reader.hpp"
//#include "writer.hpp"

ESM4::Pathgrid::Pathgrid() : mFormId(0), mFlags(0), mData(0)
{
    mEditorId.clear();

    mNodes.clear();
    mLinks.clear();
    mForeign.clear();
    mObjects.clear();
}

ESM4::Pathgrid::~Pathgrid()
{
}

void ESM4::Pathgrid::load(ESM4::Reader& reader)
{
    mFormId = reader.hdr().record.id;
    reader.adjustFormId(mFormId);
    mFlags  = reader.hdr().record.flags;

    mEditorId = formIdToString(mFormId); // FIXME: quick workaround to use existing code

    while (reader.getSubRecordHeader())
    {
        const ESM4::SubRecordHeader& subHdr = reader.subRecordHeader();
        switch (subHdr.typeId)
        {
            case ESM4::SUB_DATA: reader.get(mData); break;
            case ESM4::SUB_PGRP:
            {
                std::size_t numNodes = subHdr.dataSize / sizeof(PGRP);
                if (numNodes != mData)
                    throw std::runtime_error("ESM4::PGRD::load numNodes mismatch");

                mNodes.resize(numNodes);
                for (std::size_t i = 0; i < numNodes; ++i)
                {
                    reader.get(mNodes.at(i));
#if 0
                    //if (mFormId == 0x000C8A6B)
                    if (mFormId == 0x0001C2C8)
                    {
                        std::bitset<16> out(mNodes.at(i).unknown2);
                        std::cout << i << "," << std::to_string(mNodes.at(i).numLinks)
                            << "," << mNodes.at(i).unknown1
                            << "," << out << std::endl;
                    }
#endif
                }

                break;
            }
            case ESM4::SUB_PGRR:
            {
                static PGRR link;

                for (std::size_t i = 0; i < mData; ++i)
                {
                    for (std::size_t j = 0; j < mNodes[i].numLinks; ++j)
                    {
                        link.startNode = std::int16_t(i);

                        reader.get(link.endNode);
                        if (link.endNode == -1)
                            continue;

                        // ICMarketDistrictTheBestDefenseBasement doesn't have a PGRR
                        // CELL formId 00049E2A
                        // PGRD formId 000304B7
                        //if (mFormId == 0x000304b7)
                            //std::cout << link.startNode << "," << link.endNode << std::endl;
                        mLinks.push_back(link);
                    }
                }

                break;
            }
            case ESM4::SUB_PGRI:
            {
                std::size_t numForeign = subHdr.dataSize / sizeof(PGRI);
                mForeign.resize(numForeign);
                for (std::size_t i = 0; i < numForeign; ++i)
                {
                    reader.get(mForeign.at(i));
                    mForeign.at(i).localNode &= 0xffff; // some have junk high bits (maybe flags?)
                }

                break;
            }
            case ESM4::SUB_PGRL:
            {
                static PGRL objLink;
                reader.get(objLink.object);
                //                                        object             linkedNode
                std::size_t numNodes = (subHdr.dataSize - sizeof(int32_t)) / sizeof(int32_t);

                objLink.linkedNodes.resize(numNodes);
                for (std::size_t i = 0; i < numNodes; ++i)
                    reader.get(objLink.linkedNodes.at(i));

                mObjects.push_back(objLink);

                break;
            }
            case ESM4::SUB_PGAG:
            {
#if 0
                unsigned char mDataBuf[256/*bufSize*/];
                reader.get(&mDataBuf[0], subHdr.dataSize);

                std::ostringstream ss;
                ss << ESM4::printName(subHdr.typeId) << ":size " << subHdr.dataSize << "\n";
                for (unsigned int i = 0; i < subHdr.dataSize; ++i)
                {
                    //if (mDataBuf[i] > 64 && mDataBuf[i] < 91)
                        //ss << (char)(mDataBuf[i]) << " ";
                    //else
                        ss << std::setfill('0') << std::setw(2) << std::hex << (int)(mDataBuf[i]);
                    if ((i & 0x000f) == 0xf)
                        ss << "\n";
                    else if (i < 256/*bufSize*/-1)
                        ss << " ";
                }
                std::cout << ss.str() << std::endl;
#else
                //std::cout << "PGRD " << ESM4::printName(subHdr.typeId) << " skipping..."
                        //<< subHdr.dataSize << std::endl;
                reader.skipSubRecordData();
#endif
                break;
            }
            default:
                throw std::runtime_error("ESM4::PGRD::load - Unknown subrecord " + ESM4::printName(subHdr.typeId));
        }
    }
    //if (mFormId == 0x000304b7)
        //std::cout << "PGRR" << std::endl;
}

//void ESM4::Pathgrid::save(ESM4::Writer& writer) const
//{
//}

//void ESM4::Pathgrid::blank()
//{
//}
