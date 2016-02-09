/*
  Copyright (C) 2016 cc9cii

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
#include "acre.hpp"

#include <cassert>
#include <stdexcept>

#ifdef NDEBUG // FIXME: debuggigng only
#undef NDEBUG
#endif

#include "reader.hpp"
//#include "writer.hpp"

ESM4::ActorCreature::ActorCreature() : mFormId(0), mFlags(0), mDisabled(false), mBaseObj(0), mScale(1.f),
                                       mOwner(0), mGlobal(0), mFactionRank(0)
{
    mEditorId.clear();

    mEsp.parent = 0;
    mEsp.flags = 0;
}

ESM4::ActorCreature::~ActorCreature()
{
}

void ESM4::ActorCreature::load(ESM4::Reader& reader)
{
    mFormId = reader.adjustFormId(reader.hdr().record.id); // FIXME: use master adjusted?
    mFlags  = reader.hdr().record.flags;

    while (reader.getSubRecordHeader())
    {
        const ESM4::SubRecordHeader& subHdr = reader.subRecordHeader();
        switch (subHdr.typeId)
        {
            case ESM4::SUB_EDID: reader.getZString(mEditorId); break;
            case ESM4::SUB_NAME: reader.getFormId(mBaseObj);   break;
            case ESM4::SUB_DATA: reader.get(mPosition);        break;
            case ESM4::SUB_XSCL: reader.get(mScale);           break;
            case ESM4::SUB_XESP:
            {
                reader.get(mEsp);
                reader.adjustFormId(mEsp.parent);
                break;
            }
            case ESM4::SUB_XOWN: reader.getFormId(mOwner);     break;
            case ESM4::SUB_XGLB: reader.get(mGlobal);          break; // FIXME: formId?
            case ESM4::SUB_XRNK: reader.get(mFactionRank);     break;
            case ESM4::SUB_XRGD: // ragdoll
            {
                //std::cout << "ACRE " << ESM4::printName(subHdr.typeId) << " skipping..." << std::endl;
                reader.skipSubRecordData();
                break;
            }
            default:
                throw std::runtime_error("ESM4::ACRE::load - Unknown subrecord " + ESM4::printName(subHdr.typeId));
        }
    }
}

//void ESM4::ActorCreature::save(ESM4::Writer& writer) const
//{
//}

//void ESM4::ActorCreature::blank()
//{
//}