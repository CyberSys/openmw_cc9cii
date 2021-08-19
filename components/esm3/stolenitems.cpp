#include "stolenitems.hpp"

#include "reader.hpp"
#include "../esm/esmwriter.hpp"

namespace ESM3
{
    // NOTE: equivalent to REC_STLN except SUB_NAME, SUB_FNAM and SUB_ONAM are in lowercase
    // NOTE: OpenMW extension: allows "stacking" of stolen items via SUB_COUN
    // (called from StateManager::loadGame() via MechanicsManager::readRecord())
    void StolenItems::load(Reader& esm)
    {
        std::string itemid;
        std::map<std::pair<std::string, bool>, int> ownerMap;

        while (esm.getSubRecordHeader())
        {
            const ESM3::SubRecordHeader& subHdr = esm.subRecordHeader();
            switch (subHdr.typeId)
            {
                case ESM3::SUB_NAME:
                {
                    esm.getString(itemid); // NOTE: string not null terminated
                    break;
                }
                case ESM3::SUB_FNAM:
                case ESM3::SUB_ONAM:
                {
                    bool isFaction = (subHdr.typeId == ESM3::SUB_FNAM);
                    std::string owner;
                    esm.getString(owner); // NOTE: string not null terminated
                    esm.getSubRecordHeader(ESM3::SUB_COUN);
                    int count;
                    esm.get(count);
                    ownerMap.insert(std::make_pair(std::make_pair(owner, isFaction), count));
                    break;
                }
                default:
                    esm.skipSubRecordData();
                    break;
            }
        }
    }

    void StolenItems::write(ESM::ESMWriter& esm) const
    {
        for (StolenItemsMap::const_iterator it = mStolenItems.begin(); it != mStolenItems.end(); ++it)
        {
            esm.writeHNString("NAME", it->first);
            for (std::map<std::pair<std::string, bool>, int>::const_iterator ownerIt = it->second.begin();
                 ownerIt != it->second.end(); ++ownerIt)
            {
                if (ownerIt->first.second)
                    esm.writeHNString("FNAM", ownerIt->first.first);
                else
                    esm.writeHNString("ONAM", ownerIt->first.first);
                esm.writeHNT("COUN", ownerIt->second);
            }
        }
    }
}
