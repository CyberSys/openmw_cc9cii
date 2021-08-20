#include "inventorystate.hpp"

#include <components/misc/stringops.hpp>

#include "reader.hpp"
#include "../esm/esmwriter.hpp"

void ESM3::InventoryState::load (Reader& esm)
{
    // obsolete
    int index = 0;
    while (esm.getNextSubRecordHeader(ESM3::SUB_IOBJ))
    {
        int unused; // no longer used
        esm.get(unused);

        ObjectState state;

        if (esm.getNextSubRecordHeader(ESM3::SUB_SLOT))
        {
            int slot;
            esm.get(slot);
            mEquipmentSlots[index] = slot;
        }

        esm.getSubRecordHeader();
        state.mRef.loadId(esm, true);
        state.load(esm);

        if (state.mCount == 0)
            continue;

        mItems.push_back(state);

        ++index;
     }

    int itemsCount = 0;
    if (esm.getNextSubRecordHeader(ESM3::SUB_ICNT))
        esm.get(itemsCount);
    for (int i = 0; i < itemsCount; i++)
    {
        ObjectState state;

        esm.getSubRecordHeader(); // FRMR
        state.mRef.loadId(esm, true);
        state.load(esm);

        if (state.mCount == 0)
            continue;

        mItems.push_back(state);
    }

    //Next item is Levelled item
    while (esm.getNextSubRecordHeader(ESM3::SUB_LEVM))
    {
        //Get its name
        std::string id;
        esm.getZString(id);
        int count;
        std::string parentGroup = "";

        //Then get its count
        esm.getSubRecordHeader(ESM3::SUB_COUN);
        esm.get(count);

        //Old save formats don't have information about parent group; check for that
        if (esm.getNextSubRecordHeader(ESM3::SUB_LGRP))
        {
            //Newest saves contain parent group
            esm.getZString(parentGroup);
        }

        mLevelledItemMap[std::make_pair(id, parentGroup)] = count;
    }

    while (esm.getNextSubRecordHeader(ESM3::SUB_MAGI))
    {
        std::string id;
        esm.getZString(id);

        std::vector<std::pair<float, float> > params;
        while (esm.getNextSubRecordHeader(ESM3::SUB_RAND))
        {
            float rand, multiplier;
            esm.get(rand);

            esm.getSubRecordHeader(ESM3::SUB_MULT);
            esm.get(multiplier);

            params.emplace_back(rand, multiplier);
        }
        mPermanentMagicEffectMagnitudes[id] = params;
    }

    while (esm.getNextSubRecordHeader(ESM3::SUB_EQUI))
    {
        int equipIndex;
        esm.get(equipIndex);
        int slot;
        esm.get(slot);
        mEquipmentSlots[equipIndex] = slot;
    }

    if (esm.getNextSubRecordHeader(ESM3::SUB_EQIP))
    {
        int slotsCount = 0;
        esm.get(slotsCount);
        for (int i = 0; i < slotsCount; i++)
        {
            int equipIndex;
            esm.get(equipIndex);
            int slot;
            esm.get(slot);
            mEquipmentSlots[equipIndex] = slot;
        }
    }

    mSelectedEnchantItem = -1;
    if (esm.getNextSubRecordHeader(ESM3::SUB_SELE))
        esm.get(mSelectedEnchantItem);

    // Old saves had restocking levelled items in a special map
    // This turns items from that map into negative quantities
    for(const auto& entry : mLevelledItemMap)
    {
        const std::string& id = entry.first.first;
        const int count = entry.second;
        for(auto& item : mItems)
        {
            if(item.mCount == count && Misc::StringUtils::ciEqual(id, item.mRef.mRefID))
                item.mCount = -count;
        }
    }
}

void ESM3::InventoryState::save (ESM::ESMWriter& esm) const
{
    int itemsCount = static_cast<int>(mItems.size());
    if (itemsCount > 0)
    {
        esm.writeHNT ("ICNT", itemsCount);
        for (const ObjectState& state : mItems)
        {
            state.save (esm, true);
        }
    }

    for (std::map<std::pair<std::string, std::string>, int>::const_iterator it = mLevelledItemMap.begin(); it != mLevelledItemMap.end(); ++it)
    {
        esm.writeHNString ("LEVM", it->first.first);
        esm.writeHNT ("COUN", it->second);
        esm.writeHNString("LGRP", it->first.second);
    }

    for (TEffectMagnitudes::const_iterator it = mPermanentMagicEffectMagnitudes.begin(); it != mPermanentMagicEffectMagnitudes.end(); ++it)
    {
        esm.writeHNString("MAGI", it->first);

        const std::vector<std::pair<float, float> >& params = it->second;
        for (std::vector<std::pair<float, float> >::const_iterator pIt = params.begin(); pIt != params.end(); ++pIt)
        {
            esm.writeHNT ("RAND", pIt->first);
            esm.writeHNT ("MULT", pIt->second);
        }
    }

    int slotsCount = static_cast<int>(mEquipmentSlots.size());
    if (slotsCount > 0)
    {
        esm.startSubRecord("EQIP");
        esm.writeT(slotsCount);
        for (std::map<int, int>::const_iterator it = mEquipmentSlots.begin(); it != mEquipmentSlots.end(); ++it)
        {
            esm.writeT(it->first);
            esm.writeT(it->second);
        }
        esm.endRecord("EQIP");
    }

    if (mSelectedEnchantItem != -1)
        esm.writeHNT ("SELE", mSelectedEnchantItem);
}
