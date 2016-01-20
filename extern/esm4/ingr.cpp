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
#include "ingr.hpp"

#include <cassert>
#include <stdexcept>

#ifdef NDEBUG // FIXME: debuggigng only
#undef NDEBUG
#endif

#include "reader.hpp"
//#include "writer.hpp"

ESM4::Ingredient::Ingredient() : mScript(0)
{
    mEditorId.clear();
    mFullName.clear();
    mModel.clear();
    mIcon.clear();

    mData.value = 0;
    mData.weight = 0.f;
    mEnchantment.value = 0;
    mEnchantment.flags = 0;
}

ESM4::Ingredient::~Ingredient()
{
}

void ESM4::Ingredient::load(ESM4::Reader& reader)
{
    mFormId = reader.hdr().record.id;
    mFlags  = reader.hdr().record.flags;

    while (reader.getSubRecordHeader())
    {
        const ESM4::SubRecordHeader& subHdr = reader.subRecordHeader();
        switch (subHdr.typeId)
        {
            case ESM4::SUB_EDID: // Editor name or the worldspace
            {
                if (!reader.getZString(mEditorId))
                    throw std::runtime_error ("INGR EDID data read error");
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

                if (mFullName.empty())
                {
                    if (!reader.getZString(mFullName))
                        throw std::runtime_error ("INGR FULL data read error");
                }
                else
                {
                    // FIXME: should be part of a struct?
                    std::string scriptEffectName;
                    if (!reader.getZString(scriptEffectName))
                        throw std::runtime_error ("INGR FULL data read error");
                    mScriptEffect.push_back(scriptEffectName);
                }
                break;
            }
            case ESM4::SUB_MODL:
            {
                if (!reader.getZString(mModel))
                    throw std::runtime_error ("INGR MODL data read error");

                //if (reader.esmVersion() == ESM4::VER_094 || reader.esmVersion() == ESM4::VER_170)
                //{
                    // read MODT/MODS here?
                //}
                break;
            }
            case ESM4::SUB_ICON:
            {
                if (!reader.getZString(mIcon))
                    throw std::runtime_error ("INGR ICON data read error");
                break;
            }
            case ESM4::SUB_DATA:
            {
                if (reader.esmVersion() == ESM4::VER_094 || reader.esmVersion() == ESM4::VER_170)
                    reader.get(mData);
                else
                    reader.get(mData.weight);

                break;
            }
            case ESM4::SUB_SCRI:
            {
                reader.get(mScript);
                break;
            }
            case ESM4::SUB_ENIT:
            {
                reader.get(mEnchantment);
                break;
            }
            case ESM4::SUB_MODB:
            case ESM4::SUB_MODT:
            case ESM4::SUB_EFID:
            case ESM4::SUB_EFIT:
            case ESM4::SUB_SCIT:
            case ESM4::SUB_OBND:
            case ESM4::SUB_KSIZ:
            case ESM4::SUB_KWDA:
            case ESM4::SUB_VMAD:
            case ESM4::SUB_YNAM:
            case ESM4::SUB_ZNAM:
            {
                //std::cout << "INGR " << ESM4::printName(subHdr.typeId) << " skipping..." << std::endl;
                reader.skipSubRecordData();
                break;
            }
            default:
                throw std::runtime_error("ESM4::INGR::load - Unknown subrecord " + ESM4::printName(subHdr.typeId));
        }
    }
}

//void ESM4::Ingredient::save(ESM4::Writer& writer) const
//{
//}

//void ESM4::Ingredient::blank()
//{
//}