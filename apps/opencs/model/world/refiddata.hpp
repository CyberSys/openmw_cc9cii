#ifndef CSM_WOLRD_REFIDDATA_H
#define CSM_WOLRD_REFIDDATA_H

#include <vector>
#include <map>
#include <memory>
#include <cassert>

#include <components/esm3/acti.hpp>
#include <components/esm3/alch.hpp>
#include <components/esm3/appa.hpp>
#include <components/esm3/armo.hpp>
#include <components/esm3/book.hpp>
#include <components/esm3/clot.hpp>
#include <components/esm3/cont.hpp>
#include <components/esm3/crea.hpp>
#include <components/esm3/door.hpp>
#include <components/esm3/ingr.hpp>
#include <components/esm3/levlist.hpp>
#include <components/esm3/ligh.hpp>
#include <components/esm3/lock.hpp>
#include <components/esm3/prob.hpp>
#include <components/esm3/repa.hpp>
#include <components/esm3/stat.hpp>
#include <components/esm3/weap.hpp>
#include <components/esm3/npc_.hpp>
#include <components/esm3/misc.hpp>
#include <components/esm3/reader.hpp>
#include <components/esm/esmwriter.hpp>

#include <components/misc/stringops.hpp>

#include "record.hpp"
#include "universalid.hpp"

namespace ESM
{
    class Reader;
}

namespace CSMWorld
{
    struct RefIdDataContainerBase
    {
        virtual ~RefIdDataContainerBase();

        virtual int getSize() const = 0;

        virtual const RecordBase& getRecord (int index) const = 0;

        virtual RecordBase& getRecord (int index)= 0;

        virtual unsigned int getRecordFlags (int index) const = 0;

        virtual void appendRecord (const std::string& id, bool base) = 0;

        virtual void insertRecord (std::unique_ptr<RecordBase> record) = 0;

        virtual int load (ESM::Reader& reader, bool base) = 0;
        ///< \return index of a loaded record or -1 if no record was loaded

        virtual void erase (int index, int count) = 0;

        virtual std::string getId (int index) const = 0;

        virtual void save (int index, ESM::ESMWriter& writer) const = 0;
    };

    template<typename RecordT>
    struct RefIdDataContainer : public RefIdDataContainerBase
    {
        std::vector<std::unique_ptr<Record<RecordT> > > mContainer;

        int getSize() const override;

        const RecordBase& getRecord (int index) const override;

        RecordBase& getRecord (int index) override;

        unsigned int getRecordFlags (int index) const override;

        void appendRecord (const std::string& id, bool base) override;

        void insertRecord (std::unique_ptr<RecordBase> record) override;

        int load (ESM::Reader& reader, bool base) override;
        ///< \return index of a loaded record or -1 if no record was loaded

        void erase (int index, int count) override;

        std::string getId (int index) const override;

        void save (int index, ESM::ESMWriter& writer) const override;
    };

    template<typename RecordT>
    void RefIdDataContainer<RecordT>::insertRecord(std::unique_ptr<RecordBase> record)
    {
        assert(record != nullptr);
        // convert base pointer to record type pointer
        std::unique_ptr<Record<RecordT>> typedRecord(&dynamic_cast<Record<RecordT>&>(*record));
        record.release();
        mContainer.push_back(std::move(typedRecord));
    }

    template<typename RecordT>
    int RefIdDataContainer<RecordT>::getSize() const
    {
        return static_cast<int> (mContainer.size());
    }

    template<typename RecordT>
    const RecordBase& RefIdDataContainer<RecordT>::getRecord (int index) const
    {
        return *mContainer.at (index);
    }

    template<typename RecordT>
    RecordBase& RefIdDataContainer<RecordT>::getRecord (int index)
    {
        return *mContainer.at (index);
    }

    template<typename RecordT>
    unsigned int RefIdDataContainer<RecordT>::getRecordFlags (int index) const
    {
        return mContainer.at (index)->get().mRecordFlags;
    }

    template<typename RecordT>
    void RefIdDataContainer<RecordT>::appendRecord (const std::string& id, bool base)
    {
        std::unique_ptr<Record<RecordT> > record(new Record<RecordT>);

        record->mState = base ? RecordBase::State_BaseOnly : RecordBase::State_ModifiedOnly;

        record->mBase.mId = id;
        record->mModified.mId = id;
        (base ? record->mBase : record->mModified).blank();

        mContainer.push_back (std::move(record));
    }

    template<typename RecordT>
    int RefIdDataContainer<RecordT>::load (ESM::Reader& reader, bool base)
    {
        RecordT record;
        bool isDeleted = false;

        record.load(static_cast<ESM3::Reader&>(reader), isDeleted);

        int index = 0;
        int numRecords = static_cast<int>(mContainer.size());
        for (; index < numRecords; ++index)
        {
            if (Misc::StringUtils::ciEqual(mContainer[index]->get().mId, record.mId))
            {
                break;
            }
        }

        if (isDeleted)
        {
            if (index == numRecords)
            {
                // deleting a record that does not exist
                // ignore it for now
                /// \todo report the problem to the user
                return -1;
            }

            // Flag the record as Deleted even for a base content file.
            // RefIdData is responsible for its erasure.
            mContainer[index]->mState = RecordBase::State_Deleted;
        }
        else
        {
            if (index == numRecords)
            {
                appendRecord(record.mId, base);
                if (base)
                {
                    mContainer.back()->mBase = record;
                }
                else
                {
                    mContainer.back()->mModified = record;
                }
            }
            else if (!base)
            {
                mContainer[index]->setModified(record);
            }
            else
            {
                // Overwrite
                mContainer[index]->setModified(record);
                mContainer[index]->merge();
            }
        }

        return index;
    }

    template<typename RecordT>
    void RefIdDataContainer<RecordT>::erase (int index, int count)
    {
        if (index<0 || index+count>getSize())
            throw std::runtime_error ("invalid RefIdDataContainer index");

        mContainer.erase (mContainer.begin()+index, mContainer.begin()+index+count);
    }

    template<typename RecordT>
    std::string RefIdDataContainer<RecordT>::getId (int index) const
    {
        return mContainer.at (index)->get().mId;
    }

    template<typename RecordT>
    void RefIdDataContainer<RecordT>::save (int index, ESM::ESMWriter& writer) const
    {
        const Record<RecordT>& record = *mContainer.at(index);

        if (record.isModified() || record.mState == RecordBase::State_Deleted)
        {
            RecordT esmRecord = record.get();
            writer.startRecord(esmRecord.sRecordId, esmRecord.mRecordFlags);
            esmRecord.save(writer, record.mState == RecordBase::State_Deleted);
            writer.endRecord(esmRecord.sRecordId);
        }
    }


    class RefIdData
    {
        public:

            typedef std::pair<int, UniversalId::Type> LocalIndex;

        private:

            RefIdDataContainer<ESM3::Activator> mActivators;
            RefIdDataContainer<ESM3::Potion> mPotions;
            RefIdDataContainer<ESM3::Apparatus> mApparati;
            RefIdDataContainer<ESM3::Armor> mArmors;
            RefIdDataContainer<ESM3::Book> mBooks;
            RefIdDataContainer<ESM3::Clothing> mClothing;
            RefIdDataContainer<ESM3::Container> mContainers;
            RefIdDataContainer<ESM3::Creature> mCreatures;
            RefIdDataContainer<ESM3::Door> mDoors;
            RefIdDataContainer<ESM3::Ingredient> mIngredients;
            RefIdDataContainer<ESM3::CreatureLevList> mCreatureLevelledLists;
            RefIdDataContainer<ESM3::ItemLevList> mItemLevelledLists;
            RefIdDataContainer<ESM3::Light> mLights;
            RefIdDataContainer<ESM3::Lockpick> mLockpicks;
            RefIdDataContainer<ESM3::Miscellaneous> mMiscellaneous;
            RefIdDataContainer<ESM3::NPC> mNpcs;
            RefIdDataContainer<ESM3::Probe> mProbes;
            RefIdDataContainer<ESM3::Repair> mRepairs;
            RefIdDataContainer<ESM3::Static> mStatics;
            RefIdDataContainer<ESM3::Weapon> mWeapons;

            std::map<std::string, LocalIndex> mIndex;

            std::map<UniversalId::Type, RefIdDataContainerBase *> mRecordContainers;

            void erase (const LocalIndex& index, int count);
            ///< Must not spill over into another type.

            std::string getRecordId(const LocalIndex &index) const;

        public:

            RefIdData();

            LocalIndex globalToLocalIndex (int index) const;

            int localToGlobalIndex (const LocalIndex& index) const;

            LocalIndex searchId (const std::string& id) const;

            void erase (int index, int count);

            void insertRecord (std::unique_ptr<RecordBase> record, CSMWorld::UniversalId::Type type,
                const std::string& id);

            const RecordBase& getRecord (const LocalIndex& index) const;

            RecordBase& getRecord (const LocalIndex& index);

            unsigned int getRecordFlags(const std::string& id) const;

            void appendRecord (UniversalId::Type type, const std::string& id, bool base);

            int getAppendIndex (UniversalId::Type type) const;

            void load (ESM::Reader& reader, bool base, UniversalId::Type type);

            int getSize() const;

            std::vector<std::string> getIds (bool listDeleted = true) const;
            ///< Return a sorted collection of all IDs
            ///
            /// \param listDeleted include deleted record in the list

            void save (int index, ESM::ESMWriter& writer) const;

            //RECORD CONTAINERS ACCESS METHODS
            const RefIdDataContainer<ESM3::Book>& getBooks() const;
            const RefIdDataContainer<ESM3::Activator>& getActivators() const;
            const RefIdDataContainer<ESM3::Potion>& getPotions() const;
            const RefIdDataContainer<ESM3::Apparatus>& getApparati() const;
            const RefIdDataContainer<ESM3::Armor>& getArmors() const;
            const RefIdDataContainer<ESM3::Clothing>& getClothing() const;
            const RefIdDataContainer<ESM3::Container>& getContainers() const;
            const RefIdDataContainer<ESM3::Creature>& getCreatures() const;
            const RefIdDataContainer<ESM3::Door>& getDoors() const;
            const RefIdDataContainer<ESM3::Ingredient>& getIngredients() const;
            const RefIdDataContainer<ESM3::CreatureLevList>& getCreatureLevelledLists() const;
            const RefIdDataContainer<ESM3::ItemLevList>& getItemLevelledList() const;
            const RefIdDataContainer<ESM3::Light>& getLights() const;
            const RefIdDataContainer<ESM3::Lockpick>& getLocpicks() const;
            const RefIdDataContainer<ESM3::Miscellaneous>& getMiscellaneous() const;
            const RefIdDataContainer<ESM3::NPC>& getNPCs() const;
            const RefIdDataContainer<ESM3::Weapon >& getWeapons() const;
            const RefIdDataContainer<ESM3::Probe >& getProbes() const;
            const RefIdDataContainer<ESM3::Repair>& getRepairs() const;
            const RefIdDataContainer<ESM3::Static>& getStatics() const;

            void copyTo (int index, RefIdData& target) const;
    };
}

#endif
