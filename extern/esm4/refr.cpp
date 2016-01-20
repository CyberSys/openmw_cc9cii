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
#include "refr.hpp"

#include <cassert>
#include <stdexcept>

#include <iostream> // FIXME: debug only
#ifdef NDEBUG // FIXME: debuggigng only
#undef NDEBUG
#endif

#include "reader.hpp"
//#include "writer.hpp"

ESM4::Reference::Reference() : mBaseObj(0), mScale(1.f)
{
    mEditorId.clear();
    mFullName.clear();
}

ESM4::Reference::~Reference()
{
}

void ESM4::Reference::load(ESM4::Reader& reader)
{
    mFormId = reader.hdr().record.id;
    mFlags  = reader.hdr().record.flags;

    while (reader.getSubRecordHeader())
    {
        const ESM4::SubRecordHeader& subHdr = reader.subRecordHeader();
        switch (subHdr.typeId)
        {
            case ESM4::SUB_EDID:
            {
                if (!reader.getZString(mEditorId))
                    throw std::runtime_error ("REFR EDID data read error");
#if 0
                std::string padding = "";
                padding.insert(0, reader.stackSize()*2, ' ');
                std::cout << padding << "REFR Editor ID: " << mEditorId << std::endl;
#endif
                break;
            }
            case ESM4::SUB_FULL:
            {
                // NOTE: checking flags does not work, Skyrim.esm does not set the localized flag
                //
                // A possible hack is to look for SUB_FULL subrecord size of 4 to indicate that
                // a lookup is required.  This obviously does not work for a string size of 3,
                // but the chance of having that is assumed to be low.
                if ((reader.hdr().record.flags & Rec_Localized) != 0 || subHdr.dataSize == 4)
                {
                    reader.skipSubRecordData(); // FIXME: process the subrecord rather than skip
                    mFullName = "FIXME";
                    break;
                }

                if (!reader.getZString(mFullName))
                    throw std::runtime_error ("REFR FULL data read error");
#if 0
                std::string padding = "";
                padding.insert(0, reader.stackSize()*2, ' ');
                std::cout << padding << "REFR Full Name: " << mFullName << std::endl;
#endif
                break;
            }
            case ESM4::SUB_NAME:
            {
                reader.get(mBaseObj);
                break;
            }
            case ESM4::SUB_DATA:
            {
                reader.get(mPosition);
                break;
            }
            case ESM4::SUB_XSCL:
            {
                reader.get(mScale);
                break;
            }
            case ESM4::SUB_XTEL:
            case ESM4::SUB_XESP:
            case ESM4::SUB_XOWN:
            case ESM4::SUB_XLOC:
            case ESM4::SUB_XACT:
            case ESM4::SUB_XRNK:
            case ESM4::SUB_XMRK:
            case ESM4::SUB_FNAM:
            case ESM4::SUB_XGLB:
            case ESM4::SUB_XTRG:
            case ESM4::SUB_XSED:
            case ESM4::SUB_XLOD:
            case ESM4::SUB_XPCI:
            case ESM4::SUB_XLCM:
            case ESM4::SUB_XRTM:
            case ESM4::SUB_XCNT:
            case ESM4::SUB_TNAM:
            case ESM4::SUB_ONAM:
            case ESM4::SUB_VMAD:
            case ESM4::SUB_XPRM:
            case ESM4::SUB_INAM:
            case ESM4::SUB_LNAM:
            case ESM4::SUB_PDTO:
            case ESM4::SUB_SCHR:
            case ESM4::SUB_SCTX:
            case ESM4::SUB_XALP:
            case ESM4::SUB_XAPD:
            case ESM4::SUB_XAPR:
            case ESM4::SUB_XCVL:
            case ESM4::SUB_XCZA:
            case ESM4::SUB_XCZC:
            case ESM4::SUB_XEMI:
            case ESM4::SUB_XEZN:
            case ESM4::SUB_XFVC:
            case ESM4::SUB_XHTW:
            case ESM4::SUB_XIS2:
            case ESM4::SUB_XLCN:
            case ESM4::SUB_XLIB:
            case ESM4::SUB_XLIG:
            case ESM4::SUB_XLKR:
            case ESM4::SUB_XLRM:
            case ESM4::SUB_XLRT:
            case ESM4::SUB_XLTW:
            case ESM4::SUB_XMBO:
            case ESM4::SUB_XMBP:
            case ESM4::SUB_XMBR:
            case ESM4::SUB_XNDP:
            case ESM4::SUB_XOCP:
            case ESM4::SUB_XPOD:
            case ESM4::SUB_XPPA:
            case ESM4::SUB_XPRD:
            case ESM4::SUB_XPWR:
            case ESM4::SUB_XRDS:
            case ESM4::SUB_XRGB:
            case ESM4::SUB_XRGD:
            case ESM4::SUB_XRMR:
            case ESM4::SUB_XSPC:
            case ESM4::SUB_XTNM:
            case ESM4::SUB_XTRI:
            case ESM4::SUB_XWCN:
            case ESM4::SUB_XWCU:
            {
                //std::cout << "REFR " << ESM4::printName(subHdr.typeId) << " skipping..." << std::endl;
                reader.skipSubRecordData();
                break;
            }
            default:
                throw std::runtime_error("ESM4::REFR::load - Unknown subrecord " + ESM4::printName(subHdr.typeId));
        }
    }
}

//void ESM4::Reference::save(ESM4::Writer& writer) const
//{
//}

void ESM4::Reference::blank()
{
}