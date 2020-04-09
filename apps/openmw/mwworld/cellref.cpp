#include "cellref.hpp"

//#include <iostream> // FIXME: for debugging only

#include <extern/esm4/formid.hpp>
#include <extern/esm4/refr.hpp>
#include <extern/esm4/acre.hpp>
#include <extern/esm4/achr.hpp>
#include <extern/esm4/cell.hpp>

#include <components/esm/objectstate.hpp>

#include "../mwbase/environment.hpp"
#include "../mwbase/world.hpp"
//#include "../mwbase/windowmanager.hpp"

#include "../mwworld/esmstore.hpp"

namespace MWWorld
{

    const ESM::RefNum& CellRef::getRefNum() const
    {
        return mCellRef.mRefNum;
    }

    bool CellRef::hasContentFile() const
    {
        return mCellRef.mRefNum.hasContentFile();
    }

    void CellRef::unsetRefNum()
    {
        mCellRef.mRefNum.unset();
    }

    std::string CellRef::getRefId() const
    {
        return mCellRef.mRefID;
    }

    bool CellRef::getTeleport() const
    {
        return mCellRef.mTeleport;
    }

    ESM::Position CellRef::getDoorDest() const
    {
        return mCellRef.mDoorDest;
    }

    std::string CellRef::getDestCell() const
    {
        return mCellRef.mDestCell;
    }

    float CellRef::getScale() const
    {
        return mCellRef.mScale;
    }

    void CellRef::setScale(float scale)
    {
        if (scale != mCellRef.mScale)
        {
            mChanged = true;
            mCellRef.mScale = scale;
        }
    }

    ESM::Position CellRef::getPosition() const
    {
        return mCellRef.mPos;
    }

    void CellRef::setPosition(const ESM::Position &position)
    {
        mChanged = true;
        mCellRef.mPos = position;
    }

    float CellRef::getEnchantmentCharge() const
    {
        return mCellRef.mEnchantmentCharge;
    }

    void CellRef::setEnchantmentCharge(float charge)
    {
        if (charge != mCellRef.mEnchantmentCharge)
        {
            mChanged = true;
            mCellRef.mEnchantmentCharge = charge;
        }
    }

    int CellRef::getCharge() const
    {
        return mCellRef.mChargeInt;
    }

    void CellRef::setCharge(int charge)
    {
        if (charge != mCellRef.mChargeInt)
        {
            mChanged = true;
            mCellRef.mChargeInt = charge;
        }
    }

    float CellRef::getChargeFloat() const
    {
        return mCellRef.mChargeFloat;
    }

    void CellRef::setChargeFloat(float charge)
    {
        if (charge != mCellRef.mChargeFloat)
        {
            mChanged = true;
            mCellRef.mChargeFloat = charge;
        }
    }

    std::string CellRef::getOwner() const
    {
        return mCellRef.mOwner;
    }

    std::string CellRef::getGlobalVariable() const
    {
        return mCellRef.mGlobalVariable;
    }

    void CellRef::resetGlobalVariable()
    {
        if (!mCellRef.mGlobalVariable.empty())
        {
            mChanged = true;
            mCellRef.mGlobalVariable.erase();
        }
    }

    void CellRef::setFactionRank(int factionRank)
    {
        if (factionRank != mCellRef.mFactionRank)
        {
            mChanged = true;
            mCellRef.mFactionRank = factionRank;
        }
    }

    int CellRef::getFactionRank() const
    {
        return mCellRef.mFactionRank;
    }

    void CellRef::setOwner(const std::string &owner)
    {
        if (owner != mCellRef.mOwner)
        {
            mChanged = true;
            mCellRef.mOwner = owner;
        }
    }

    std::string CellRef::getSoul() const
    {
        return mCellRef.mSoul;
    }

    void CellRef::setSoul(const std::string &soul)
    {
        if (soul != mCellRef.mSoul)
        {
            mChanged = true;
            mCellRef.mSoul = soul;
        }
    }

    std::string CellRef::getFaction() const
    {
        return mCellRef.mFaction;
    }

    void CellRef::setFaction(const std::string &faction)
    {
        if (faction != mCellRef.mFaction)
        {
            mChanged = true;
            mCellRef.mFaction = faction;
        }
    }

    int CellRef::getLockLevel() const
    {
        return mCellRef.mLockLevel;
    }

    void CellRef::setLockLevel(int lockLevel)
    {
        if (lockLevel != mCellRef.mLockLevel)
        {
            mChanged = true;
            mCellRef.mLockLevel = lockLevel;
        }
    }

    std::string CellRef::getKey() const
    {
        return mCellRef.mKey;
    }

    std::string CellRef::getTrap() const
    {
        return mCellRef.mTrap;
    }

    void CellRef::setTrap(const std::string& trap)
    {
        if (trap != mCellRef.mTrap)
        {
            mChanged = true;
            mCellRef.mTrap = trap;
        }
    }

    int CellRef::getGoldValue() const
    {
        return mCellRef.mGoldValue;
    }

    void CellRef::setGoldValue(int value)
    {
        if (value != mCellRef.mGoldValue)
        {
            mChanged = true;
            mCellRef.mGoldValue = value;
        }
    }

    void CellRef::writeState(ESM::ObjectState &state) const
    {
        state.mRef = mCellRef;
    }

    bool CellRef::hasChanged() const
    {
        return mChanged;
    }

    // This may not be required?  see LiveCellRef constructor
    CellRef::CellRef (const ESM4::Reference& ref)
    {
#if 0
        if (ref.mEditorId.empty())
        {
            mCellRef.mRefID = ESM4::formIdToString(ref.mFormId);
            std::cout << "CellRef:: empty editor id, using " << mCellRef.mRefID << std::endl;
        }
        else
            mCellRef.mRefID = ref.mEditorId; // almost all references have this
#endif
        mFormId = ref.mFormId;
        mDestDoorId = 0;

        mCellRef.mRefID = ESM4::formIdToString(ref.mBaseObj);

        mCellRef.mPos.pos[0] = ref.mPosition.pos.x;
        mCellRef.mPos.pos[1] = ref.mPosition.pos.y;
        mCellRef.mPos.pos[2] = ref.mPosition.pos.z;
        mCellRef.mPos.rot[0] = ref.mPosition.rot.x;
        mCellRef.mPos.rot[1] = ref.mPosition.rot.y;
        mCellRef.mPos.rot[2] = ref.mPosition.rot.z;

        mCellRef.mScale = ref.mScale;

        if (ref.mDoor.destDoor != 0)
        {
            mCellRef.mTeleport = true;
            mCellRef.mDoorDest.pos[0] = ref.mDoor.destPos.pos.x;
            mCellRef.mDoorDest.pos[1] = ref.mDoor.destPos.pos.y;
            mCellRef.mDoorDest.pos[2] = ref.mDoor.destPos.pos.z;
            mCellRef.mDoorDest.rot[0] = ref.mDoor.destPos.rot.x;
            mCellRef.mDoorDest.rot[1] = ref.mDoor.destPos.rot.y;
            mCellRef.mDoorDest.rot[2] = ref.mDoor.destPos.rot.z;

            mDestDoorId = ref.mDoor.destDoor;
// while loading doors from CellStore, MWBase::World is not yet fully constructed
#if 0
            const MWWorld::ESMStore& store = MWBase::Environment::get().getWorld()->getStore();
            const ESM4::FormId cellId = store.getDoorCellId(ref.mDoor.destDoor);
            const MWWorld::ForeignCell *cell = store.get<MWWorld::ForeignCell>().find(cellId);
            if (cell && !cell->mCell->mFullName.empty())
                mCellRef.mDestCell = cell->mCell->mFullName;
            else if (cell && !cell->mCell->mEditorId.empty())
                mCellRef.mDestCell = cell->mCell->mEditorId;
            else
                mCellRef.mDestCell = "";
#endif
        }
        else
            mCellRef.mDestCell = "";

        mChanged = false;
    }

    CellRef::CellRef (const ESM4::ActorCreature& ref)
    {
        mFormId = ref.mFormId;

        mCellRef.mRefID = ESM4::formIdToString(ref.mBaseObj);

        mCellRef.mPos.pos[0] = ref.mPosition.pos.x;
        mCellRef.mPos.pos[1] = ref.mPosition.pos.y;
        mCellRef.mPos.pos[2] = ref.mPosition.pos.z;
        mCellRef.mPos.rot[0] = ref.mPosition.rot.x;
        mCellRef.mPos.rot[1] = ref.mPosition.rot.y;
        mCellRef.mPos.rot[2] = ref.mPosition.rot.z;

        mCellRef.mScale = ref.mScale;

        mChanged = false;
    }

    CellRef::CellRef (const ESM4::ActorCharacter& ref)
    {
        mFormId = ref.mFormId;

        mCellRef.mRefID = ESM4::formIdToString(ref.mBaseObj);

        mCellRef.mPos.pos[0] = ref.mPosition.pos.x;
        mCellRef.mPos.pos[1] = ref.mPosition.pos.y;
        mCellRef.mPos.pos[2] = ref.mPosition.pos.z;
        mCellRef.mPos.rot[0] = ref.mPosition.rot.x;
        mCellRef.mPos.rot[1] = ref.mPosition.rot.y;
        mCellRef.mPos.rot[2] = ref.mPosition.rot.z;

        mCellRef.mScale = ref.mScale;

        mChanged = false;
    }

    ESM4::FormId CellRef::getFormId () const
    {
        return mFormId;
    }

    ESM4::FormId CellRef::getDestDoorId () const
    {
        return mDestDoorId;
    }

}
