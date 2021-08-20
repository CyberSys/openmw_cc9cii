#ifndef OPENMW_MWWORLD_ESMSTORE_H
#define OPENMW_MWWORLD_ESMSTORE_H

#include <memory>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

#include <components/esm3/records.hpp>
#include "store.hpp"

namespace Loading
{
    class Listener;
}

namespace MWMechanics
{
    class SpellList;
}

namespace MWWorld
{
    class ESMStore
    {
        Store<ESM3::Activator>       mActivators;
        Store<ESM3::Potion>          mPotions;
        Store<ESM3::Apparatus>       mAppas;
        Store<ESM3::Armor>           mArmors;
        Store<ESM3::BodyPart>        mBodyParts;
        Store<ESM3::Book>            mBooks;
        Store<ESM3::BirthSign>       mBirthSigns;
        Store<ESM3::Class>           mClasses;
        Store<ESM3::Clothing>        mClothes;
        Store<ESM3::Container>       mContainers;
        Store<ESM3::Creature>        mCreatures;
        Store<ESM3::Dialogue>        mDialogs;
        Store<ESM3::Door>            mDoors;
        Store<ESM3::Enchantment>     mEnchants;
        Store<ESM3::Faction>         mFactions;
        Store<ESM3::Global>          mGlobals;
        Store<ESM3::Ingredient>      mIngreds;
        Store<ESM3::CreatureLevList> mCreatureLists;
        Store<ESM3::ItemLevList>     mItemLists;
        Store<ESM3::Light>           mLights;
        Store<ESM3::Lockpick>        mLockpicks;
        Store<ESM3::Miscellaneous>   mMiscItems;
        Store<ESM3::NPC>             mNpcs;
        Store<ESM3::Probe>           mProbes;
        Store<ESM3::Race>            mRaces;
        Store<ESM3::Region>          mRegions;
        Store<ESM3::Repair>          mRepairs;
        Store<ESM3::SoundGenerator>  mSoundGens;
        Store<ESM3::Sound>           mSounds;
        Store<ESM3::Spell>           mSpells;
        Store<ESM3::StartScript>     mStartScripts;
        Store<ESM3::Static>          mStatics;
        Store<ESM3::Weapon>          mWeapons;

        Store<ESM3::GameSetting>     mGameSettings;
        Store<ESM3::Script>          mScripts;

        // Lists that need special rules
        Store<ESM3::Cell>        mCells;
        Store<ESM3::Land>        mLands;
        Store<ESM3::LandTexture> mLandTextures;
        Store<ESM3::Pathgrid>    mPathgrids;

        Store<ESM3::MagicEffect> mMagicEffects;
        Store<ESM3::Skill>       mSkills;

        // Special entry which is hardcoded and not loaded from an ESM
        Store<ESM::Attribute>   mAttributes;

        // Lookup of all IDs. Makes looking up references faster. Just
        // maps the id name to the record type.
        std::map<std::string, int> mIds;
        std::map<std::string, int> mStaticIds;

        std::unordered_map<std::string, int> mRefCount;

        std::map<int, StoreBase *> mStores;

        unsigned int mDynamicCount;

        mutable std::map<std::string, std::weak_ptr<MWMechanics::SpellList> > mSpellListCache;

        /// Validate entries in store after setup
        void validate();

        void countRecords();

        template<class T>
        void removeMissingObjects(Store<T>& store);
    public:
        /// \todo replace with SharedIterator<StoreBase>
        typedef std::map<int, StoreBase *>::const_iterator iterator;

        iterator begin() const {
            return mStores.begin();
        }

        iterator end() const {
            return mStores.end();
        }

        /// Look up the given ID in 'all'. Returns 0 if not found.
        /// \note id must be in lower case.
        int find(const std::string &id) const
        {
            std::map<std::string, int>::const_iterator it = mIds.find(id);
            if (it == mIds.end()) {
                return 0;
            }
            return it->second;
        }
        int findStatic(const std::string &id) const
        {
            std::map<std::string, int>::const_iterator it = mStaticIds.find(id);
            if (it == mStaticIds.end()) {
                return 0;
            }
            return it->second;
        }

        ESMStore()
          : mDynamicCount(0)
        {
            mStores[ESM3::REC_ACTI] = &mActivators;
            mStores[ESM3::REC_ALCH] = &mPotions;
            mStores[ESM3::REC_APPA] = &mAppas;
            mStores[ESM3::REC_ARMO] = &mArmors;
            mStores[ESM3::REC_BODY] = &mBodyParts;
            mStores[ESM3::REC_BOOK] = &mBooks;
            mStores[ESM3::REC_BSGN] = &mBirthSigns;
            mStores[ESM3::REC_CELL] = &mCells;
            mStores[ESM3::REC_CLAS] = &mClasses;
            mStores[ESM3::REC_CLOT] = &mClothes;
            mStores[ESM3::REC_CONT] = &mContainers;
            mStores[ESM3::REC_CREA] = &mCreatures;
            mStores[ESM3::REC_DIAL] = &mDialogs;
            mStores[ESM3::REC_DOOR] = &mDoors;
            mStores[ESM3::REC_ENCH] = &mEnchants;
            mStores[ESM3::REC_FACT] = &mFactions;
            mStores[ESM3::REC_GLOB] = &mGlobals;
            mStores[ESM3::REC_GMST] = &mGameSettings;
            mStores[ESM3::REC_INGR] = &mIngreds;
            mStores[ESM3::REC_LAND] = &mLands;
            mStores[ESM3::REC_LEVC] = &mCreatureLists;
            mStores[ESM3::REC_LEVI] = &mItemLists;
            mStores[ESM3::REC_LIGH] = &mLights;
            mStores[ESM3::REC_LOCK] = &mLockpicks;
            mStores[ESM3::REC_LTEX] = &mLandTextures;
            mStores[ESM3::REC_MISC] = &mMiscItems;
            mStores[ESM3::REC_NPC_] = &mNpcs;
            mStores[ESM3::REC_PGRD] = &mPathgrids;
            mStores[ESM3::REC_PROB] = &mProbes;
            mStores[ESM3::REC_RACE] = &mRaces;
            mStores[ESM3::REC_REGN] = &mRegions;
            mStores[ESM3::REC_REPA] = &mRepairs;
            mStores[ESM3::REC_SCPT] = &mScripts;
            mStores[ESM3::REC_SNDG] = &mSoundGens;
            mStores[ESM3::REC_SOUN] = &mSounds;
            mStores[ESM3::REC_SPEL] = &mSpells;
            mStores[ESM3::REC_SSCR] = &mStartScripts;
            mStores[ESM3::REC_STAT] = &mStatics;
            mStores[ESM3::REC_WEAP] = &mWeapons;

            mPathgrids.setCells(mCells);
        }

        void clearDynamic ()
        {
            for (std::map<int, StoreBase *>::iterator it = mStores.begin(); it != mStores.end(); ++it)
                it->second->clearDynamic();

            movePlayerRecord();
        }

        void movePlayerRecord ()
        {
            auto player = mNpcs.find("player");
            mNpcs.insert(*player);
        }

        /// Validate entries in store after loading a save
        void validateDynamic();

        void load(ESM::Reader &esm, Loading::Listener* listener);

        template <class T>
        const Store<T> &get() const {
            throw std::runtime_error("Storage for this type not exist");
        }

        /// Insert a custom record (i.e. with a generated ID that will not clash will pre-existing records)
        template <class T>
        const T *insert(const T &x)
        {
            const std::string id = "$dynamic" + std::to_string(mDynamicCount++);

            Store<T> &store = const_cast<Store<T> &>(get<T>());
            if (store.search(id) != nullptr)
            {
                const std::string msg = "Try to override existing record '" + id + "'";
                throw std::runtime_error(msg);
            }
            T record = x;

            record.mId = id;

            T *ptr = store.insert(record);
            for (iterator it = mStores.begin(); it != mStores.end(); ++it) {
                if (it->second == &store) {
                    mIds[ptr->mId] = it->first;
                }
            }
            return ptr;
        }

        /// Insert a record with set ID, and allow it to override a pre-existing static record.
        template <class T>
        const T *overrideRecord(const T &x) {
            Store<T> &store = const_cast<Store<T> &>(get<T>());

            T *ptr = store.insert(x);
            for (iterator it = mStores.begin(); it != mStores.end(); ++it) {
                if (it->second == &store) {
                    mIds[ptr->mId] = it->first;
                }
            }
            return ptr;
        }

        template <class T>
        const T *insertStatic(const T &x)
        {
            const std::string id = "$dynamic" + std::to_string(mDynamicCount++);

            Store<T> &store = const_cast<Store<T> &>(get<T>());
            if (store.search(id) != nullptr)
            {
                const std::string msg = "Try to override existing record '" + id + "'";
                throw std::runtime_error(msg);
            }
            T record = x;

            T *ptr = store.insertStatic(record);
            for (iterator it = mStores.begin(); it != mStores.end(); ++it) {
                if (it->second == &store) {
                    mIds[ptr->mId] = it->first;
                }
            }
            return ptr;
        }

        // This method must be called once, after loading all master/plugin files. This can only be done
        //  from the outside, so it must be public.
        void setUp(bool validateRecords = false);

        int countSavedGameRecords() const;

        void write (ESM::ESMWriter& writer, Loading::Listener& progress) const;

        bool readRecord (ESM3::Reader& reader, uint32_t type);
        ///< \return Known type?

        // To be called when we are done with dynamic record loading
        void checkPlayer();

        /// @return The number of instances defined in the base files. Excludes changes from the save file.
        int getRefCount(const std::string& id) const;

        /// Actors with the same ID share spells, abilities, etc.
        /// @return The shared spell list to use for this actor and whether or not it has already been initialized.
        std::pair<std::shared_ptr<MWMechanics::SpellList>, bool> getSpellList(const std::string& id) const;
    };

    template <>
    inline const ESM3::Cell *ESMStore::insert<ESM3::Cell>(const ESM3::Cell &cell) {
        return mCells.insert(cell);
    }

    template <>
    inline const ESM3::NPC *ESMStore::insert<ESM3::NPC>(const ESM3::NPC &npc)
    {
        const std::string id = "$dynamic" + std::to_string(mDynamicCount++);

        if (Misc::StringUtils::ciEqual(npc.mId, "player"))
        {
            return mNpcs.insert(npc);
        }
        else if (mNpcs.search(id) != nullptr)
        {
            const std::string msg = "Try to override existing record '" + id + "'";
            throw std::runtime_error(msg);
        }
        ESM3::NPC record = npc;

        record.mId = id;

        ESM3::NPC *ptr = mNpcs.insert(record);
        mIds[ptr->mId] = ESM3::REC_NPC_;
        return ptr;
    }

    template <>
    inline const Store<ESM3::Activator> &ESMStore::get<ESM3::Activator>() const {
        return mActivators;
    }

    template <>
    inline const Store<ESM3::Potion> &ESMStore::get<ESM3::Potion>() const {
        return mPotions;
    }

    template <>
    inline const Store<ESM3::Apparatus> &ESMStore::get<ESM3::Apparatus>() const {
        return mAppas;
    }

    template <>
    inline const Store<ESM3::Armor> &ESMStore::get<ESM3::Armor>() const {
        return mArmors;
    }

    template <>
    inline const Store<ESM3::BodyPart> &ESMStore::get<ESM3::BodyPart>() const {
        return mBodyParts;
    }

    template <>
    inline const Store<ESM3::Book> &ESMStore::get<ESM3::Book>() const {
        return mBooks;
    }

    template <>
    inline const Store<ESM3::BirthSign> &ESMStore::get<ESM3::BirthSign>() const {
        return mBirthSigns;
    }

    template <>
    inline const Store<ESM3::Class> &ESMStore::get<ESM3::Class>() const {
        return mClasses;
    }

    template <>
    inline const Store<ESM3::Clothing> &ESMStore::get<ESM3::Clothing>() const {
        return mClothes;
    }

    template <>
    inline const Store<ESM3::Container> &ESMStore::get<ESM3::Container>() const {
        return mContainers;
    }

    template <>
    inline const Store<ESM3::Creature> &ESMStore::get<ESM3::Creature>() const {
        return mCreatures;
    }

    template <>
    inline const Store<ESM3::Dialogue> &ESMStore::get<ESM3::Dialogue>() const {
        return mDialogs;
    }

    template <>
    inline const Store<ESM3::Door> &ESMStore::get<ESM3::Door>() const {
        return mDoors;
    }

    template <>
    inline const Store<ESM3::Enchantment> &ESMStore::get<ESM3::Enchantment>() const {
        return mEnchants;
    }

    template <>
    inline const Store<ESM3::Faction> &ESMStore::get<ESM3::Faction>() const {
        return mFactions;
    }

    template <>
    inline const Store<ESM3::Global> &ESMStore::get<ESM3::Global>() const {
        return mGlobals;
    }

    template <>
    inline const Store<ESM3::Ingredient> &ESMStore::get<ESM3::Ingredient>() const {
        return mIngreds;
    }

    template <>
    inline const Store<ESM3::CreatureLevList> &ESMStore::get<ESM3::CreatureLevList>() const {
        return mCreatureLists;
    }

    template <>
    inline const Store<ESM3::ItemLevList> &ESMStore::get<ESM3::ItemLevList>() const {
        return mItemLists;
    }

    template <>
    inline const Store<ESM3::Light> &ESMStore::get<ESM3::Light>() const {
        return mLights;
    }

    template <>
    inline const Store<ESM3::Lockpick> &ESMStore::get<ESM3::Lockpick>() const {
        return mLockpicks;
    }

    template <>
    inline const Store<ESM3::Miscellaneous> &ESMStore::get<ESM3::Miscellaneous>() const {
        return mMiscItems;
    }

    template <>
    inline const Store<ESM3::NPC> &ESMStore::get<ESM3::NPC>() const {
        return mNpcs;
    }

    template <>
    inline const Store<ESM3::Probe> &ESMStore::get<ESM3::Probe>() const {
        return mProbes;
    }

    template <>
    inline const Store<ESM3::Race> &ESMStore::get<ESM3::Race>() const {
        return mRaces;
    }

    template <>
    inline const Store<ESM3::Region> &ESMStore::get<ESM3::Region>() const {
        return mRegions;
    }

    template <>
    inline const Store<ESM3::Repair> &ESMStore::get<ESM3::Repair>() const {
        return mRepairs;
    }

    template <>
    inline const Store<ESM3::SoundGenerator> &ESMStore::get<ESM3::SoundGenerator>() const {
        return mSoundGens;
    }

    template <>
    inline const Store<ESM3::Sound> &ESMStore::get<ESM3::Sound>() const {
        return mSounds;
    }

    template <>
    inline const Store<ESM3::Spell> &ESMStore::get<ESM3::Spell>() const {
        return mSpells;
    }

    template <>
    inline const Store<ESM3::StartScript> &ESMStore::get<ESM3::StartScript>() const {
        return mStartScripts;
    }

    template <>
    inline const Store<ESM3::Static> &ESMStore::get<ESM3::Static>() const {
        return mStatics;
    }

    template <>
    inline const Store<ESM3::Weapon> &ESMStore::get<ESM3::Weapon>() const {
        return mWeapons;
    }

    template <>
    inline const Store<ESM3::GameSetting> &ESMStore::get<ESM3::GameSetting>() const {
        return mGameSettings;
    }

    template <>
    inline const Store<ESM3::Script> &ESMStore::get<ESM3::Script>() const {
        return mScripts;
    }

    template <>
    inline const Store<ESM3::Cell> &ESMStore::get<ESM3::Cell>() const {
        return mCells;
    }

    template <>
    inline const Store<ESM3::Land> &ESMStore::get<ESM3::Land>() const {
        return mLands;
    }

    template <>
    inline const Store<ESM3::LandTexture> &ESMStore::get<ESM3::LandTexture>() const {
        return mLandTextures;
    }

    template <>
    inline const Store<ESM3::Pathgrid> &ESMStore::get<ESM3::Pathgrid>() const {
        return mPathgrids;
    }

    template <>
    inline const Store<ESM3::MagicEffect> &ESMStore::get<ESM3::MagicEffect>() const {
        return mMagicEffects;
    }

    template <>
    inline const Store<ESM3::Skill> &ESMStore::get<ESM3::Skill>() const {
        return mSkills;
    }

    template <>
    inline const Store<ESM::Attribute> &ESMStore::get<ESM::Attribute>() const {
        return mAttributes;
    }
}

#endif
