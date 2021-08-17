#include "esmstore.hpp"

#include <algorithm>
#include <set>

#include <boost/filesystem/operations.hpp>

#include <components/debug/debuglog.hpp>
#include <components/loadinglistener/loadinglistener.hpp>
#include <components/esm3/reader.hpp>
#include <components/esm/esmwriter.hpp>
#include <components/misc/algorithm.hpp>

#include "../mwmechanics/spelllist.hpp"

namespace
{
    struct Ref
    {
        ESM3::RefNum mRefNum;
        std::size_t mRefID;

        Ref(ESM3::RefNum refNum, std::size_t refID) : mRefNum(refNum), mRefID(refID) {}
    };

    constexpr std::size_t deletedRefID = std::numeric_limits<std::size_t>::max();

    void readRefs(const ESM3::Cell& cell, std::vector<Ref>& refs, std::vector<std::string>& refIDs, std::vector<ESM3::Reader>& readers)
    {
        for (size_t i = 0; i < cell.mContextList.size(); i++)
        {
            size_t index = cell.mContextList[i].modIndex;
            if (readers.size() <= index)
                readers.resize(index + 1);
            cell.restore(readers[index], i);
            ESM3::CellRef ref;
            ref.mRefNum.unset();
            bool deleted = false;
            while(cell.getNextRef(readers[index], ref, deleted))
            {
                if(deleted)
                    refs.emplace_back(ref.mRefNum, deletedRefID);
                else if (std::find(cell.mMovedRefs.begin(), cell.mMovedRefs.end(), ref.mRefNum) == cell.mMovedRefs.end())
                {
                    refs.emplace_back(ref.mRefNum, refIDs.size());
                    refIDs.push_back(std::move(ref.mRefID));
                }
            }
        }
        for(const auto& [value, deleted] : cell.mLeasedRefs)
        {
            if(deleted)
                refs.emplace_back(value.mRefNum, deletedRefID);
            else
            {
                refs.emplace_back(value.mRefNum, refIDs.size());
                refIDs.push_back(value.mRefID);
            }
        }
    }

    std::vector<ESM3::NPC> getNPCsToReplace(const MWWorld::Store<ESM3::Faction>& factions, const MWWorld::Store<ESM3::Class>& classes, const std::map<std::string, ESM3::NPC>& npcs)
    {
        // Cache first class from store - we will use it if current class is not found
        std::string defaultCls;
        auto it = classes.begin();
        if (it != classes.end())
            defaultCls = it->mId;
        else
            throw std::runtime_error("List of NPC classes is empty!");

        // Validate NPCs for non-existing class and faction.
        // We will replace invalid entries by fixed ones
        std::vector<ESM3::NPC> npcsToReplace;

        for (const auto& npcIter : npcs)
        {
            ESM3::NPC npc = npcIter.second;
            bool changed = false;

            const std::string npcFaction = npc.mFaction;
            if (!npcFaction.empty())
            {
                const ESM3::Faction *fact = factions.search(npcFaction);
                if (!fact)
                {
                    Log(Debug::Verbose) << "NPC '" << npc.mId << "' (" << npc.mName << ") has nonexistent faction '" << npc.mFaction << "', ignoring it.";
                    npc.mFaction.clear();
                    npc.mNpdt.mRank = 0;
                    changed = true;
                }
            }

            std::string npcClass = npc.mClass;
            if (!npcClass.empty())
            {
                const ESM3::Class *cls = classes.search(npcClass);
                if (!cls)
                {
                    Log(Debug::Verbose) << "NPC '" << npc.mId << "' (" << npc.mName << ") has nonexistent class '" << npc.mClass << "', using '" << defaultCls << "' class as replacement.";
                    npc.mClass = defaultCls;
                    changed = true;
                }
            }

            if (changed)
                npcsToReplace.push_back(npc);
        }

        return npcsToReplace;
    }

    // Custom enchanted items can reference scripts that no longer exist, this doesn't necessarily mean the base item no longer exists however.
    // So instead of removing the item altogether, we're only removing the script.
    template<class T>
    void removeMissingScripts(const MWWorld::Store<ESM3::Script>& scripts, std::map<std::string, T>& items)
    {
        for(auto& [id, item] : items)
        {
            if(!item.mScript.empty() && !scripts.search(item.mScript))
            {
                item.mScript.clear();
                Log(Debug::Verbose) << "Item '" << id << "' (" << item.mName << ") has nonexistent script '" << item.mScript << "', ignoring it.";
            }
        }
    }
}

namespace MWWorld
{

static bool isCacheableRecord(int id)
{
    if (id == ESM3::REC_ACTI || id == ESM3::REC_ALCH || id == ESM3::REC_APPA || id == ESM3::REC_ARMO ||
        id == ESM3::REC_BOOK || id == ESM3::REC_CLOT || id == ESM3::REC_CONT || id == ESM3::REC_CREA ||
        id == ESM3::REC_DOOR || id == ESM3::REC_INGR || id == ESM3::REC_LEVC || id == ESM3::REC_LEVI ||
        id == ESM3::REC_LIGH || id == ESM3::REC_LOCK || id == ESM3::REC_MISC || id == ESM3::REC_NPC_ ||
        id == ESM3::REC_PROB || id == ESM3::REC_REPA || id == ESM3::REC_STAT || id == ESM3::REC_WEAP ||
        id == ESM3::REC_BODY)
    {
        return true;
    }
    return false;
}

void ESMStore::load(ESM::Reader &esm, Loading::Listener* listener)
{
    listener->setProgressRange(1000);

    // FIXME: add ESM4 code here

    // NOTE: everything below uses ESM3::Reader
    // FIXME: dynamic_cast is probably safer
    ESM3::Reader& reader = static_cast<ESM3::Reader&>(esm);

    ESM3::Dialogue *dialogue = nullptr;

    // Land texture loading needs to use a separate internal store for each plugin.
    // We set the number of plugins here to avoid continual resizes during loading,
    // and so we can properly verify if valid plugin indices are being passed to the
    // LandTexture Store retrieval methods.
    mLandTextures.resize(esm.getGlobalReaderList()->size()); // FIXME: need to change the logic here - we don't need all content files for land textures

    /// \todo Move this to somewhere else. ESMReader?
    // Cache parent esX files by tracking their indices in the global list of
    //  all files/readers used by the engine. This will greaty accelerate
    //  refnumber mangling, as required for handling moved references.
    const std::vector<ESM::MasterData> &masters = esm.getGameFiles();
    std::vector<ESM::Reader*> *allPlugins = esm.getGlobalReaderList();
    for (size_t j = 0; j < masters.size(); j++) {
        const ESM::MasterData &mast = masters[j];
        std::string fname = mast.name;
        int index = ~0;
        for (unsigned int i = 0; i < reader.getModIndex(); i++) {
            const std::string candidate = allPlugins->at(i)->getFileName();
            std::string fnamecandidate = boost::filesystem::path(candidate).filename().string();
            if (Misc::StringUtils::ciEqual(fname, fnamecandidate)) {
                index = i;
                break;
            }
        }
        if (index == (int)~0) {
            // Tried to load a parent file that has not been loaded yet. This is bad,
            //  the launcher should have taken care of this.
            std::string fstring = "File " + esm.getFileName() + " asks for parent file " + masters[j].name
                + ", but it has not been loaded yet. Please check your load order.";
            reader.fail(fstring);
        }
        reader.addParentFileIndex(index);
    }

    // Loop through all records
    while(esm.hasMoreRecs())
    {
        reader.getRecordHeader();
        std::uint32_t typeId = reader.hdr().typeId;

        // Look up the record type.
        std::map<int, StoreBase *>::iterator it = mStores.find(typeId);

        if (it == mStores.end()) {
            if (typeId == ESM3::REC_INFO) {
                if (dialogue)
                {
                    dialogue->readInfo(reader, reader.getModIndex() != 0);
                }
                else
                {
                    Log(Debug::Error) << "Error: info record without dialog";
                    reader.skipRecordData();
                }
            } else if (typeId == ESM::REC_MGEF) {
                mMagicEffects.load (reader);
            } else if (typeId == ESM::REC_SKIL) {
                mSkills.load (reader);
            }
            else if (typeId==ESM3::REC_FILT || typeId == ESM3::REC_DBGP)
            {
                // ignore project file only records
                reader.skipRecordData();
            }
            else {
                throw std::runtime_error("Unknown record: " + ESM::printName(typeId));
            }
        } else {
            RecordId id = it->second->load(reader);
            if (id.mIsDeleted)
            {
                it->second->eraseStatic(id.mId);
                continue;
            }

            if (typeId == ESM3::REC_DIAL) {
                dialogue = const_cast<ESM3::Dialogue*>(mDialogs.find(id.mId));
            } else {
                dialogue = nullptr;
            }
        }
        listener->setProgress(static_cast<size_t>(reader.getFileOffset() / (float)reader.getFileSize() * 1000));
    }
}

void ESMStore::setUp(bool validateRecords)
{
    mIds.clear();

    std::map<int, StoreBase *>::iterator storeIt = mStores.begin();
    for (; storeIt != mStores.end(); ++storeIt) {
        storeIt->second->setUp();

        if (isCacheableRecord(storeIt->first))
        {
            std::vector<std::string> identifiers;
            storeIt->second->listIdentifier(identifiers);

            for (std::vector<std::string>::const_iterator record = identifiers.begin(); record != identifiers.end(); ++record)
                mIds[*record] = storeIt->first;
        }
    }

    if (mStaticIds.empty())
        mStaticIds = mIds;

    mSkills.setUp();
    mMagicEffects.setUp();
    mAttributes.setUp();
    mDialogs.setUp();

    if (validateRecords)
    {
        validate();
        countRecords();
    }
}

void ESMStore::countRecords()
{
    if(!mRefCount.empty())
        return;
    std::vector<Ref> refs;
    std::vector<std::string> refIDs;
    std::vector<ESM3::Reader> readers;
    for(auto it = mCells.intBegin(); it != mCells.intEnd(); ++it)
        readRefs(*it, refs, refIDs, readers);
    for(auto it = mCells.extBegin(); it != mCells.extEnd(); ++it)
        readRefs(*it, refs, refIDs, readers);
    const auto lessByRefNum = [] (const Ref& l, const Ref& r) { return l.mRefNum < r.mRefNum; };
    std::stable_sort(refs.begin(), refs.end(), lessByRefNum);
    const auto equalByRefNum = [] (const Ref& l, const Ref& r) { return l.mRefNum == r.mRefNum; };
    const auto incrementRefCount = [&] (const Ref& value)
    {
        if (value.mRefID != deletedRefID)
        {
            std::string& refId = refIDs[value.mRefID];
            Misc::StringUtils::lowerCaseInPlace(refId);
            ++mRefCount[std::move(refId)];
        }
    };
    Misc::forEachUnique(refs.rbegin(), refs.rend(), equalByRefNum, incrementRefCount);
}

int ESMStore::getRefCount(const std::string& id) const
{
    const std::string lowerId = Misc::StringUtils::lowerCase(id);
    auto it = mRefCount.find(lowerId);
    if(it == mRefCount.end())
        return 0;
    return it->second;
}

void ESMStore::validate()
{
    std::vector<ESM3::NPC> npcsToReplace = getNPCsToReplace(mFactions, mClasses, mNpcs.mStatic);

    for (const ESM3::NPC &npc : npcsToReplace)
    {
        mNpcs.eraseStatic(npc.mId);
        mNpcs.insertStatic(npc);
    }

    // Validate spell effects for invalid arguments
    std::vector<ESM3::Spell> spellsToReplace;
    for (ESM3::Spell spell : mSpells)
    {
        if (spell.mEffects.mList.empty())
            continue;

        bool changed = false;
        auto iter = spell.mEffects.mList.begin();
        while (iter != spell.mEffects.mList.end())
        {
            const ESM3::MagicEffect* mgef = mMagicEffects.search(iter->mEffectID);
            if (!mgef)
            {
                Log(Debug::Verbose) << "Spell '" << spell.mId << "' has an invalid effect (index " << iter->mEffectID << ") present. Dropping the effect.";
                iter = spell.mEffects.mList.erase(iter);
                changed = true;
                continue;
            }

            if (mgef->mData.mFlags & ESM3::MagicEffect::TargetSkill)
            {
                if (iter->mAttribute != -1)
                {
                    iter->mAttribute = -1;
                    Log(Debug::Verbose) << ESM3::MagicEffect::effectIdToString(iter->mEffectID) <<
                        " effect of spell '" << spell.mId << "' has an attribute argument present. Dropping the argument.";
                    changed = true;
                }
            }
            else if (mgef->mData.mFlags & ESM3::MagicEffect::TargetAttribute)
            {
                if (iter->mSkill != -1)
                {
                    iter->mSkill = -1;
                    Log(Debug::Verbose) << ESM3::MagicEffect::effectIdToString(iter->mEffectID) <<
                        " effect of spell '" << spell.mId << "' has a skill argument present. Dropping the argument.";
                    changed = true;
                }
            }
            else if (iter->mSkill != -1 || iter->mAttribute != -1)
            {
                iter->mSkill = -1;
                iter->mAttribute = -1;
                Log(Debug::Verbose) << ESM3::MagicEffect::effectIdToString(iter->mEffectID) <<
                    " effect of spell '" << spell.mId << "' has argument(s) present. Dropping the argument(s).";
                changed = true;
            }

            ++iter;
        }

        if (changed)
            spellsToReplace.emplace_back(spell);
    }

    for (const ESM3::Spell &spell : spellsToReplace)
    {
        mSpells.eraseStatic(spell.mId);
        mSpells.insertStatic(spell);
    }
}

void ESMStore::validateDynamic()
{
    std::vector<ESM3::NPC> npcsToReplace = getNPCsToReplace(mFactions, mClasses, mNpcs.mDynamic);

    for (const ESM3::NPC &npc : npcsToReplace)
        mNpcs.insert(npc);

    removeMissingScripts(mScripts, mArmors.mDynamic);
    removeMissingScripts(mScripts, mBooks.mDynamic);
    removeMissingScripts(mScripts, mClothes.mDynamic);
    removeMissingScripts(mScripts, mWeapons.mDynamic);

    removeMissingObjects(mCreatureLists);
    removeMissingObjects(mItemLists);
}

// Leveled lists can be modified by scripts. This removes items that no longer exist (presumably because the plugin was removed) from modified lists
template<class T>
void ESMStore::removeMissingObjects(Store<T>& store)
{
    for(auto& entry : store.mDynamic)
    {
        auto first = std::remove_if(entry.second.mList.begin(), entry.second.mList.end(), [&] (const auto& item)
        {
            if(!find(item.mId))
            {
                Log(Debug::Verbose) << "Leveled list '" << entry.first << "' has nonexistent object '" << item.mId << "', ignoring it.";
                return true;
            }
            return false;
        });
        entry.second.mList.erase(first, entry.second.mList.end());
    }
}

    int ESMStore::countSavedGameRecords() const
    {
        return 1 // DYNA (dynamic name counter)
            +mPotions.getDynamicSize()
            +mArmors.getDynamicSize()
            +mBooks.getDynamicSize()
            +mClasses.getDynamicSize()
            +mClothes.getDynamicSize()
            +mEnchants.getDynamicSize()
            +mNpcs.getDynamicSize()
            +mSpells.getDynamicSize()
            +mWeapons.getDynamicSize()
            +mCreatureLists.getDynamicSize()
            +mItemLists.getDynamicSize()
            +mCreatures.getDynamicSize()
            +mContainers.getDynamicSize();
    }

    void ESMStore::write (ESM::ESMWriter& writer, Loading::Listener& progress) const
    {
        writer.startRecord(ESM::REC_DYNA);
        writer.startSubRecord("COUN");
        writer.writeT(mDynamicCount);
        writer.endRecord("COUN");
        writer.endRecord(ESM::REC_DYNA);

        mPotions.write (writer, progress);
        mArmors.write (writer, progress);
        mBooks.write (writer, progress);
        mClasses.write (writer, progress);
        mClothes.write (writer, progress);
        mEnchants.write (writer, progress);
        mSpells.write (writer, progress);
        mWeapons.write (writer, progress);
        mNpcs.write (writer, progress);
        mItemLists.write (writer, progress);
        mCreatureLists.write (writer, progress);
        mCreatures.write (writer, progress);
        mContainers.write (writer, progress);
    }

    bool ESMStore::readRecord (ESM3::Reader& reader, uint32_t type)
    {
        switch (type)
        {
            case ESM3::REC_ALCH:
            case ESM3::REC_ARMO:
            case ESM3::REC_BOOK:
            case ESM3::REC_CLAS:
            case ESM3::REC_CLOT:
            case ESM3::REC_ENCH:
            case ESM3::REC_SPEL:
            case ESM3::REC_WEAP:
            case ESM3::REC_LEVI:
            case ESM3::REC_LEVC:
                mStores[type]->read (reader);
                return true;

            case ESM3::REC_NPC_:
            case ESM3::REC_CREA:
            case ESM3::REC_CONT:
                mStores[type]->read (reader, true);
                return true;

            case ESM::REC_DYNA:
                reader.getSubRecordHeader();
                assert(reader.subRecordHeader().typeId == ESM3::SUB_COUN);
                reader.get(mDynamicCount);
                return true;

            default:

                return false;
        }
    }

    void ESMStore::checkPlayer()
    {
        setUp();

        const ESM3::NPC *player = mNpcs.find ("player");

        if (!mRaces.find (player->mRace) ||
            !mClasses.find (player->mClass))
            throw std::runtime_error ("Invalid player record (race or class unavailable");
    }

    std::pair<std::shared_ptr<MWMechanics::SpellList>, bool> ESMStore::getSpellList(const std::string& originalId) const
    {
        const std::string id = Misc::StringUtils::lowerCase(originalId);
        auto result = mSpellListCache.find(id);
        std::shared_ptr<MWMechanics::SpellList> ptr;
        if (result != mSpellListCache.end())
            ptr = result->second.lock();
        if (!ptr)
        {
            int type = find(id);
            ptr = std::make_shared<MWMechanics::SpellList>(id, type);
            if (result != mSpellListCache.end())
                result->second = ptr;
            else
                mSpellListCache.insert({id, ptr});
            return {ptr, false};
        }
        return {ptr, true};
    }
} // end namespace
