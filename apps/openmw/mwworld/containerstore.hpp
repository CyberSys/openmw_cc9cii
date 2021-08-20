#ifndef GAME_MWWORLD_CONTAINERSTORE_H
#define GAME_MWWORLD_CONTAINERSTORE_H

#include <iterator>
#include <map>
#include <memory>
#include <utility>

#include <components/esm3/alch.hpp>
#include <components/esm3/appa.hpp>
#include <components/esm3/armo.hpp>
#include <components/esm3/book.hpp>
#include <components/esm3/clot.hpp>
#include <components/esm3/ingr.hpp>
#include <components/esm3/lock.hpp>
#include <components/esm3/ligh.hpp>
#include <components/esm3/misc.hpp>
#include <components/esm3/prob.hpp>
#include <components/esm3/repa.hpp>
#include <components/esm3/weap.hpp>

#include <components/misc/rng.hpp>

#include "ptr.hpp"
#include "cellreflist.hpp"

namespace ESM3
{
    struct InventoryList;
    struct InventoryState;
}

namespace MWClass
{
    class Container;
}

namespace MWWorld
{
    class ContainerStore;

    template<class PtrType>
    class ContainerStoreIteratorBase;

    typedef ContainerStoreIteratorBase<Ptr> ContainerStoreIterator;
    typedef ContainerStoreIteratorBase<ConstPtr> ConstContainerStoreIterator;

    class ResolutionListener
    {
            ContainerStore& mStore;
        public:
            ResolutionListener(ContainerStore& store) : mStore(store) {}
            ~ResolutionListener();
    };

    class ResolutionHandle
    {
            std::shared_ptr<ResolutionListener> mListener;
        public:
            ResolutionHandle(std::shared_ptr<ResolutionListener> listener) : mListener(listener) {}
            ResolutionHandle() = default;
    };
    
    class ContainerStoreListener
    {
        public:
            virtual void itemAdded(const ConstPtr& item, int count) {}
            virtual void itemRemoved(const ConstPtr& item, int count) {}
            virtual ~ContainerStoreListener() = default;
    };

    class ContainerStore
    {
        public:

            static constexpr int Type_Potion = 0x0001;
            static constexpr int Type_Apparatus = 0x0002;
            static constexpr int Type_Armor = 0x0004;
            static constexpr int Type_Book = 0x0008;
            static constexpr int Type_Clothing = 0x0010;
            static constexpr int Type_Ingredient = 0x0020;
            static constexpr int Type_Light = 0x0040;
            static constexpr int Type_Lockpick = 0x0080;
            static constexpr int Type_Miscellaneous = 0x0100;
            static constexpr int Type_Probe = 0x0200;
            static constexpr int Type_Repair = 0x0400;
            static constexpr int Type_Weapon = 0x0800;

            static constexpr int Type_Last = Type_Weapon;

            static constexpr int Type_All = 0xffff;

            static const std::string sGoldId;

        protected:
            ContainerStoreListener* mListener;

            // (item, max charge)
            typedef std::vector<std::pair<ContainerStoreIterator, float> > TRechargingItems;
            TRechargingItems mRechargingItems;

            bool mRechargingItemsUpToDate;

        private:

            MWWorld::CellRefList<ESM3::Potion>            potions;
            MWWorld::CellRefList<ESM3::Apparatus>         appas;
            MWWorld::CellRefList<ESM3::Armor>             armors;
            MWWorld::CellRefList<ESM3::Book>              books;
            MWWorld::CellRefList<ESM3::Clothing>          clothes;
            MWWorld::CellRefList<ESM3::Ingredient>        ingreds;
            MWWorld::CellRefList<ESM3::Light>             lights;
            MWWorld::CellRefList<ESM3::Lockpick>          lockpicks;
            MWWorld::CellRefList<ESM3::Miscellaneous>     miscItems;
            MWWorld::CellRefList<ESM3::Probe>             probes;
            MWWorld::CellRefList<ESM3::Repair>            repairs;
            MWWorld::CellRefList<ESM3::Weapon>            weapons;

            mutable float mCachedWeight;
            mutable bool mWeightUpToDate;

            bool mModified;
            bool mResolved;
            unsigned int mSeed;
            MWWorld::Ptr mPtr;
            std::weak_ptr<ResolutionListener> mResolutionListener;

            ContainerStoreIterator addImp (const Ptr& ptr, int count, bool markModified = true);
            void addInitialItem (const std::string& id, const std::string& owner, int count, Misc::Rng::Seed* seed, bool topLevel=true);
            void addInitialItemImp (const MWWorld::Ptr& ptr, const std::string& owner, int count, Misc::Rng::Seed* seed, bool topLevel=true);

            template<typename T>
            ContainerStoreIterator getState (CellRefList<T>& collection,
                const ESM3::ObjectState& state);

            template<typename T>
            void storeState (const LiveCellRef<T>& ref, ESM3::ObjectState& state) const;

            template<typename T>
            void storeStates (const CellRefList<T>& collection,
                ESM3::InventoryState& inventory, int& index,
                bool equipable = false) const;

            void updateRechargingItems();

            virtual void storeEquipmentState (const MWWorld::LiveCellRefBase& ref, int index, ESM3::InventoryState& inventory) const;

            virtual void readEquipmentState (const MWWorld::ContainerStoreIterator& iter, int index, const ESM3::InventoryState& inventory);

        public:

            ContainerStore();

            virtual ~ContainerStore();

            virtual std::unique_ptr<ContainerStore> clone() { return std::make_unique<ContainerStore>(*this); }

            ConstContainerStoreIterator cbegin (int mask = Type_All) const;
            ConstContainerStoreIterator cend() const;
            ConstContainerStoreIterator begin (int mask = Type_All) const;
            ConstContainerStoreIterator end() const;
            
            ContainerStoreIterator begin (int mask = Type_All);
            ContainerStoreIterator end();

            bool hasVisibleItems() const;

            virtual ContainerStoreIterator add (const Ptr& itemPtr, int count, const Ptr& actorPtr, bool allowAutoEquip = true, bool resolve = true);
            ///< Add the item pointed to by \a ptr to this container. (Stacks automatically if needed)
            ///
            /// \note The item pointed to is not required to exist beyond this function call.
            ///
            /// \attention Do not add items to an existing stack by increasing the count instead of
            /// calling this function!
            ///
            /// @return if stacking happened, return iterator to the item that was stacked against, otherwise iterator to the newly inserted item.

            ContainerStoreIterator add(const std::string& id, int count, const Ptr& actorPtr);
            ///< Utility to construct a ManualRef and call add(ptr, count, actorPtr, true)

            int remove(const std::string& itemId, int count, const Ptr& actor, bool equipReplacement = 0, bool resolve = true);
            ///< Remove \a count item(s) designated by \a itemId from this container.
            ///
            /// @return the number of items actually removed

            virtual int remove(const Ptr& item, int count, const Ptr& actor, bool equipReplacement = 0, bool resolve = true);
            ///< Remove \a count item(s) designated by \a item from this inventory.
            ///
            /// @return the number of items actually removed

            void rechargeItems (float duration);
            ///< Restore charge on enchanted items. Note this should only be done for the player.

            ContainerStoreIterator unstack (const Ptr& ptr, const Ptr& container, int count = 1);
            ///< Unstack an item in this container. The item's count will be set to count, then a new stack will be added with (origCount-count).
            ///
            /// @return an iterator to the new stack, or end() if no new stack was created.

            MWWorld::ContainerStoreIterator restack (const MWWorld::Ptr& item);
            ///< Attempt to re-stack an item in this container.
            /// If a compatible stack is found, the item's count is added to that stack, then the original is deleted.
            /// @return If the item was stacked, return the stack, otherwise return the old (untouched) item.

            int count (const std::string& id) const;
            ///< @return How many items with refID \a id are in this container?

            ContainerStoreListener* getContListener() const;
            void setContListener(ContainerStoreListener* listener);
        protected:
            ContainerStoreIterator addNewStack (const ConstPtr& ptr, int count);
            ///< Add the item to this container (do not try to stack it onto existing items)

            virtual void flagAsModified();

            /// + and - operations that can deal with negative stacks
            /// Note that negativity is infectious
            static int addItems(int count1, int count2);
            static int subtractItems(int count1, int count2);
        public:

            virtual bool stacks (const ConstPtr& ptr1, const ConstPtr& ptr2) const;
            ///< @return true if the two specified objects can stack with each other

            void fill (const ESM3::InventoryList& items, const std::string& owner, Misc::Rng::Seed& seed = Misc::Rng::getSeed());
            ///< Insert items into *this.

            void fillNonRandom (const ESM3::InventoryList& items, const std::string& owner, unsigned int seed);
            ///< Insert items into *this, excluding leveled items

            virtual void clear();
            ///< Empty container.

            float getWeight() const;
            ///< Return total weight of the items contained in *this.

            static int getType (const ConstPtr& ptr);
            ///< This function throws an exception, if ptr does not point to an object, that can be
            /// put into a container.

            Ptr findReplacement(const std::string& id);
            ///< Returns replacement for object with given id. Prefer used items (with low durability left).

            Ptr search (const std::string& id);

            virtual void writeState (ESM3::InventoryState& state) const;

            virtual void readState (const ESM3::InventoryState& state);

            bool isResolved() const;

            void resolve();
            ResolutionHandle resolveTemporarily();
            void unresolve();

            friend class ContainerStoreIteratorBase<Ptr>;
            friend class ContainerStoreIteratorBase<ConstPtr>;
            friend class ResolutionListener;
            friend class MWClass::Container;
    };

    
    template<class PtrType>
    class ContainerStoreIteratorBase
        : public std::iterator<std::forward_iterator_tag, PtrType, std::ptrdiff_t, PtrType *, PtrType&>
    {
        template<class From, class To, class Dummy>
        struct IsConvertible
        {
            static constexpr bool value = true;
        };

        template<class Dummy>
        struct IsConvertible<ConstPtr, Ptr, Dummy>
        {
            static constexpr bool value = false;
        };

        template<class T, class U>
        struct IteratorTrait
        {
            typedef typename MWWorld::CellRefList<T>::List::iterator type;
        };

        template<class T>
        struct IteratorTrait<T, ConstPtr>
        {
            typedef typename MWWorld::CellRefList<T>::List::const_iterator type;
        };

        template<class T>
        struct Iterator : IteratorTrait<T, PtrType>
        {
        };

        template<class T, class Dummy>
        struct ContainerStoreTrait
        {
            typedef ContainerStore* type;
        };
        
        template<class Dummy>
        struct ContainerStoreTrait<ConstPtr, Dummy>
        {
            typedef const ContainerStore* type;
        };

        typedef typename ContainerStoreTrait<PtrType, void>::type ContainerStoreType;

        int mType;
        int mMask;
        ContainerStoreType mContainer;
        mutable PtrType mPtr;

        typename Iterator<ESM3::Potion>::type mPotion;
        typename Iterator<ESM3::Apparatus>::type mApparatus;
        typename Iterator<ESM3::Armor>::type mArmor;
        typename Iterator<ESM3::Book>::type mBook;
        typename Iterator<ESM3::Clothing>::type mClothing;
        typename Iterator<ESM3::Ingredient>::type mIngredient;
        typename Iterator<ESM3::Light>::type mLight;
        typename Iterator<ESM3::Lockpick>::type mLockpick;
        typename Iterator<ESM3::Miscellaneous>::type mMiscellaneous;
        typename Iterator<ESM3::Probe>::type mProbe;
        typename Iterator<ESM3::Repair>::type mRepair;
        typename Iterator<ESM3::Weapon>::type mWeapon;

        ContainerStoreIteratorBase (ContainerStoreType container);
        ///< End-iterator

        ContainerStoreIteratorBase (int mask, ContainerStoreType container);
        ///< Begin-iterator

        // construct iterator using a CellRefList iterator
        ContainerStoreIteratorBase (ContainerStoreType container, typename Iterator<ESM3::Potion>::type);
        ContainerStoreIteratorBase (ContainerStoreType container, typename Iterator<ESM3::Apparatus>::type);
        ContainerStoreIteratorBase (ContainerStoreType container, typename Iterator<ESM3::Armor>::type);
        ContainerStoreIteratorBase (ContainerStoreType container, typename Iterator<ESM3::Book>::type);
        ContainerStoreIteratorBase (ContainerStoreType container, typename Iterator<ESM3::Clothing>::type);
        ContainerStoreIteratorBase (ContainerStoreType container, typename Iterator<ESM3::Ingredient>::type);
        ContainerStoreIteratorBase (ContainerStoreType container, typename Iterator<ESM3::Light>::type);
        ContainerStoreIteratorBase (ContainerStoreType container, typename Iterator<ESM3::Lockpick>::type);
        ContainerStoreIteratorBase (ContainerStoreType container, typename Iterator<ESM3::Miscellaneous>::type);
        ContainerStoreIteratorBase (ContainerStoreType container, typename Iterator<ESM3::Probe>::type);
        ContainerStoreIteratorBase (ContainerStoreType container, typename Iterator<ESM3::Repair>::type);
        ContainerStoreIteratorBase (ContainerStoreType container, typename Iterator<ESM3::Weapon>::type);

        template<class T>
        void copy (const ContainerStoreIteratorBase<T>& src);
        
        void incType ();
        
        void nextType ();

        bool resetIterator ();
        ///< Reset iterator for selected type.
        ///
        /// \return Type not empty?

        bool incIterator ();
        ///< Increment iterator for selected type.
        ///
        /// \return reached the end?

        public:
            template<class T>
            ContainerStoreIteratorBase (const ContainerStoreIteratorBase<T>& other)
            {
                char CANNOT_CONVERT_CONST_ITERATOR_TO_ITERATOR[IsConvertible<T, PtrType, void>::value ? 1 : -1];
                ((void)CANNOT_CONVERT_CONST_ITERATOR_TO_ITERATOR);
                copy (other);
            }

            template<class T>
            bool isEqual(const ContainerStoreIteratorBase<T>& other) const;

            PtrType *operator->() const;
            PtrType operator*() const;

            ContainerStoreIteratorBase& operator++ ();
            ContainerStoreIteratorBase operator++ (int);
            ContainerStoreIteratorBase& operator= (const ContainerStoreIteratorBase& rhs);
            ContainerStoreIteratorBase (const ContainerStoreIteratorBase& rhs) = default;

            int getType() const;
            const ContainerStore *getContainerStore() const;

            friend class ContainerStore;
            friend class ContainerStoreIteratorBase<Ptr>;
            friend class ContainerStoreIteratorBase<ConstPtr>;
    };

    template<class T, class U>
    bool operator== (const ContainerStoreIteratorBase<T>& left, const ContainerStoreIteratorBase<U>& right);
    template<class T, class U>
    bool operator!= (const ContainerStoreIteratorBase<T>& left, const ContainerStoreIteratorBase<U>& right);
}
#endif
