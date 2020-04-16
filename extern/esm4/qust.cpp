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
#include "qust.hpp"

#include <stdexcept>
//#include <iostream> // FIXME: for debugging only

#include "reader.hpp"
//#include "writer.hpp"

ESM4::Quest::Quest() : mFormId(0), mFlags(0), mQuestScript(0), mData(0)
{
    mEditorId.clear();
    mQuestName.clear();
    mFileName.clear();
}

ESM4::Quest::~Quest()
{
}

void ESM4::Quest::load(ESM4::Reader& reader)
{
    mFormId = reader.hdr().record.id;
    reader.adjustFormId(mFormId);
    mFlags  = reader.hdr().record.flags;

    while (reader.getSubRecordHeader())
    {
        const ESM4::SubRecordHeader& subHdr = reader.subRecordHeader();
        switch (subHdr.typeId)
        {
            case ESM4::SUB_EDID: reader.getZString(mEditorId);  break;
            case ESM4::SUB_FULL: reader.getZString(mQuestName); break;
            case ESM4::SUB_ICON: reader.getZString(mFileName);  break;
            case ESM4::SUB_DATA:
            {
                if (subHdr.dataSize != sizeof(mData))
                    reader.skipSubRecordData(); // FIXME: FO3
                else
                    reader.get(mData); // TES4

                break;
            }
            case ESM4::SUB_SCRI: reader.get(mQuestScript); break;
            case ESM4::SUB_CTDA:
            case ESM4::SUB_INDX:
            case ESM4::SUB_QSDT:
            case ESM4::SUB_CNAM:
            case ESM4::SUB_SCHR:
            case ESM4::SUB_SCDA:
            case ESM4::SUB_SCTX:
            case ESM4::SUB_SCRO:
            case ESM4::SUB_QSTA:
            case ESM4::SUB_NNAM: // FO3
            case ESM4::SUB_QOBJ: // FO3
            case ESM4::SUB_NAM0: // FO3
            case ESM4::SUB_ANAM: // TES5
            case ESM4::SUB_DNAM: // TES5
            case ESM4::SUB_ENAM: // TES5
            case ESM4::SUB_FNAM: // TES5
            case ESM4::SUB_NEXT: // TES5
            case ESM4::SUB_ALCA: // TES5
            case ESM4::SUB_ALCL: // TES5
            case ESM4::SUB_ALCO: // TES5
            case ESM4::SUB_ALDN: // TES5
            case ESM4::SUB_ALEA: // TES5
            case ESM4::SUB_ALED: // TES5
            case ESM4::SUB_ALEQ: // TES5
            case ESM4::SUB_ALFA: // TES5
            case ESM4::SUB_ALFC: // TES5
            case ESM4::SUB_ALFD: // TES5
            case ESM4::SUB_ALFE: // TES5
            case ESM4::SUB_ALFI: // TES5
            case ESM4::SUB_ALFL: // TES5
            case ESM4::SUB_ALFR: // TES5
            case ESM4::SUB_ALID: // TES5
            case ESM4::SUB_ALLS: // TES5
            case ESM4::SUB_ALNA: // TES5
            case ESM4::SUB_ALNT: // TES5
            case ESM4::SUB_ALPC: // TES5
            case ESM4::SUB_ALRT: // TES5
            case ESM4::SUB_ALSP: // TES5
            case ESM4::SUB_ALST: // TES5
            case ESM4::SUB_ALUA: // TES5
            case ESM4::SUB_CIS2: // TES5
            case ESM4::SUB_CNTO: // TES5
            case ESM4::SUB_COCT: // TES5
            case ESM4::SUB_ECOR: // TES5
            case ESM4::SUB_FLTR: // TES5
            case ESM4::SUB_KNAM: // TES5
            case ESM4::SUB_KSIZ: // TES5
            case ESM4::SUB_KWDA: // TES5
            case ESM4::SUB_QNAM: // TES5
            case ESM4::SUB_QTGL: // TES5
            case ESM4::SUB_SPOR: // TES5
            case ESM4::SUB_VMAD: // TES5
            case ESM4::SUB_VTCK: // TES5
            {
                //std::cout << "QUST " << ESM4::printName(subHdr.typeId) << " skipping..."
                          //<< subHdr.dataSize << std::endl;
                reader.skipSubRecordData();
                break;
            }
            default:
                throw std::runtime_error("ESM4::QUST::load - Unknown subrecord " + ESM4::printName(subHdr.typeId));
        }
    }
    //std::cout << "QUST " << mEditorId << ": " << mQuestName << " @ " << mFileName << std::endl;
}

//void ESM4::Quest::save(ESM4::Writer& writer) const
//{
//}

//void ESM4::Quest::blank()
//{
//}