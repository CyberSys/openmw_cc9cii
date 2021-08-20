#ifndef CSM_WOLRD_IDCOLLECTION_H
#define CSM_WOLRD_IDCOLLECTION_H

#include <components/esm3/reader.hpp>

#include "collection.hpp"
#include "land.hpp"

namespace CSMWorld
{
    /// \brief Single type collection of top level records
    template<typename ESXRecordT, typename IdAccessorT = IdAccessor<ESXRecordT> >
    class IdCollection : public Collection<ESXRecordT, IdAccessorT>
    {
            virtual void loadRecord (ESXRecordT& record, ESM::Reader& reader, bool& isDeleted);

        public:

            /// \return Index of loaded record (-1 if no record was loaded)
            int load (ESM::Reader& reader, bool base);

            /// \param index Index at which the record can be found.
            /// Special values: -2 index unknown, -1 record does not exist yet and therefore
            /// does not have an index
            ///
            /// \return index
            int load (const ESXRecordT& record, bool base, int index = -2);

            bool tryDelete (const std::string& id);
            ///< Try deleting \a id. If the id does not exist or can't be deleted the call is ignored.
            ///
            /// \return Has the ID been deleted?
    };

    template<typename ESXRecordT, typename IdAccessorT>
    void IdCollection<ESXRecordT, IdAccessorT>::loadRecord (ESXRecordT& record,
                                                            ESM::Reader& reader,
                                                            bool& isDeleted)
    {
        record.load (static_cast<ESM3::Reader&>(reader), isDeleted);
    }

    template<>
    inline void IdCollection<Land, IdAccessor<Land> >::loadRecord (Land& record,
        ESM::Reader& reader, bool& isDeleted)
    {
        record.load (static_cast<ESM3::Reader&>(reader), isDeleted);

        // Load all land data for now. A future optimisation may only load non-base data
        // if a suitable mechanism for avoiding race conditions can be established.
        int flags = ESM3::Land::DATA_VHGT | ESM3::Land::DATA_VNML |
                    ESM3::Land::DATA_VCLR | ESM3::Land::DATA_VTEX;
        record.loadData (flags);

        // Prevent data from being reloaded.
        record.mContext.filename.clear();
    }

    template<typename ESXRecordT, typename IdAccessorT>
    int IdCollection<ESXRecordT, IdAccessorT>::load (ESM::Reader& reader, bool base)
    {
        ESXRecordT record;
        bool isDeleted = false;

        loadRecord (record, reader, isDeleted);

        std::string id = IdAccessorT().getId (record);
        int index = this->searchId (id);

        if (isDeleted)
        {
            if (index==-1)
            {
                // deleting a record that does not exist
                // ignore it for now
                /// \todo report the problem to the user
                return -1;
            }

            if (base)
            {
                this->removeRows (index, 1);
                return -1;
            }

            std::unique_ptr<Record<ESXRecordT> > baseRecord(new Record<ESXRecordT>(this->getRecord(index)));
            baseRecord->mState = RecordBase::State_Deleted;
            this->setRecord(index, std::move(baseRecord));
            return index;
        }

        return load (record, base, index);
    }

    template<typename ESXRecordT, typename IdAccessorT>
    int IdCollection<ESXRecordT, IdAccessorT>::load (const ESXRecordT& record, bool base,
        int index)
    {
        if (index==-2) // index unknown
            index = this->searchId (IdAccessorT().getId (record));

        if (index==-1)
        {
            // new record
            std::unique_ptr<Record<ESXRecordT> > record2(new Record<ESXRecordT>);
            record2->mState = base ? RecordBase::State_BaseOnly : RecordBase::State_ModifiedOnly;
            (base ? record2->mBase : record2->mModified) = record;

            index = this->getSize();
            this->appendRecord(std::move(record2));
        }
        else
        {
            // old record
            std::unique_ptr<Record<ESXRecordT> > record2(
                    new Record<ESXRecordT>(Collection<ESXRecordT, IdAccessorT>::getRecord(index)));

            if (base)
                record2->mBase = record;
            else
                record2->setModified(record);

            this->setRecord(index, std::move(record2));
        }

        return index;
    }

    template<typename ESXRecordT, typename IdAccessorT>
    bool IdCollection<ESXRecordT, IdAccessorT>::tryDelete (const std::string& id)
    {
        int index = this->searchId (id);

        if (index==-1)
            return false;

        const Record<ESXRecordT>& record = Collection<ESXRecordT, IdAccessorT>::getRecord (index);

        if (record.isDeleted())
            return false;

        if (record.mState==RecordBase::State_ModifiedOnly)
        {
            Collection<ESXRecordT, IdAccessorT>::removeRows (index, 1);
        }
        else
        {
            std::unique_ptr<Record<ESXRecordT> > record2(
                    new Record<ESXRecordT>(Collection<ESXRecordT, IdAccessorT>::getRecord(index)));
            record2->mState = RecordBase::State_Deleted;
            this->setRecord(index, std::move(record2));
        }

        return true;
    }
}

#endif
