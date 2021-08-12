#include "objectstate.hpp"

#include <stdexcept>
#include <sstream>
#include <typeinfo>

#include "reader.hpp"
#include "../esm/esmwriter.hpp"

// called from CreatureState::load, ContainerState::load(),
//             InventoryState::load() and NpcState::load()
void ESM3::ObjectState::load (Reader& esm)
{

    mVersion = esm.getFormat();
#if 0
    bool isDeleted;
    mRef.loadData(esm, isDeleted);

    mHasLocals = 0;
    esm.getHNOT (mHasLocals, "HLOC");

    if (mHasLocals)
        mLocals.load (esm);

    mLuaScripts.load(esm);

    mEnabled = 1;
    esm.getHNOT (mEnabled, "ENAB");

    mCount = 1;
    esm.getHNOT (mCount, "COUN");

    mPosition = mRef.mPos;
    esm.getHNOT (mPosition, "POS_", 24);

    if (esm.isNextSub("LROT"))
        esm.skipHSub(); // local rotation, no longer used

    mFlags = 0;
    esm.getHNOT (mFlags, "FLAG");

    // obsolete
    int unused;
    esm.getHNOT(unused, "LTIM");

    mAnimationState.load(esm);

    // NOTE: mAnimationState.load() will have attempted to read the sub-record header
    //assert(esm.subRecordHeader().typeId == ESM3::SUB_HCUS && "ObjectState: unexpected sub record found");

    // FIXME: assuming "false" as default would make more sense, but also break compatibility with older save files
    mHasCustomState = true;
    esm.getHNOT (mHasCustomState, "HCUS");
#endif
}

void ESM3::ObjectState::save (ESM::ESMWriter& esm, bool inInventory) const
{
    mRef.save (esm, true, inInventory);

    if (mHasLocals)
    {
        esm.writeHNT ("HLOC", mHasLocals);
        mLocals.save (esm);
    }

    mLuaScripts.save(esm);

    if (!mEnabled && !inInventory)
        esm.writeHNT ("ENAB", mEnabled);

    if (mCount!=1)
        esm.writeHNT ("COUN", mCount);

    if (!inInventory && mPosition != mRef.mPos)
        esm.writeHNT ("POS_", mPosition, 24);

    if (mFlags != 0)
        esm.writeHNT ("FLAG", mFlags);

    mAnimationState.save(esm);

    if (!mHasCustomState)
        esm.writeHNT ("HCUS", false);
}

void ESM3::ObjectState::blank()
{
    mRef.blank();
    mHasLocals = 0;
    mEnabled = false;
    mCount = 1;
    for (int i=0;i<3;++i)
    {
        mPosition.pos[i] = 0;
        mPosition.rot[i] = 0;
    }
    mFlags = 0;
    mHasCustomState = true;
}

const ESM3::NpcState& ESM3::ObjectState::asNpcState() const
{
    std::stringstream error;
    error << "bad cast " << typeid(this).name() << " to NpcState";
    throw std::logic_error(error.str());
}

ESM3::NpcState& ESM3::ObjectState::asNpcState()
{
    std::stringstream error;
    error << "bad cast " << typeid(this).name() << " to NpcState";
    throw std::logic_error(error.str());
}

const ESM3::CreatureState& ESM3::ObjectState::asCreatureState() const
{
    std::stringstream error;
    error << "bad cast " << typeid(this).name() << " to CreatureState";
    throw std::logic_error(error.str());
}

ESM3::CreatureState& ESM3::ObjectState::asCreatureState()
{
    std::stringstream error;
    error << "bad cast " << typeid(this).name() << " to CreatureState";
    throw std::logic_error(error.str());
}

const ESM3::ContainerState& ESM3::ObjectState::asContainerState() const
{
    std::stringstream error;
    error << "bad cast " << typeid(this).name() << " to ContainerState";
    throw std::logic_error(error.str());
}

ESM3::ContainerState& ESM3::ObjectState::asContainerState()
{
    std::stringstream error;
    error << "bad cast " << typeid(this).name() << " to ContainerState";
    throw std::logic_error(error.str());
}

const ESM3::DoorState& ESM3::ObjectState::asDoorState() const
{
    std::stringstream error;
    error << "bad cast " << typeid(this).name() << " to DoorState";
    throw std::logic_error(error.str());
}

ESM3::DoorState& ESM3::ObjectState::asDoorState()
{
    std::stringstream error;
    error << "bad cast " << typeid(this).name() << " to DoorState";
    throw std::logic_error(error.str());
}

const ESM3::CreatureLevListState& ESM3::ObjectState::asCreatureLevListState() const
{
    std::stringstream error;
    error << "bad cast " << typeid(this).name() << " to CreatureLevListState";
    throw std::logic_error(error.str());
}

ESM3::CreatureLevListState& ESM3::ObjectState::asCreatureLevListState()
{
    std::stringstream error;
    error << "bad cast " << typeid(this).name() << " to CreatureLevListState";
    throw std::logic_error(error.str());
}

ESM3::ObjectState::~ObjectState() {}
