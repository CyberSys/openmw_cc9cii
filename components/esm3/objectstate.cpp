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

    bool isDeleted;
    mRef.loadData(esm, isDeleted);

    mHasLocals = 0;
    mEnabled = 1;
    mCount = 1;
    mPosition = mRef.mPos;
    mFlags = 0;
    // FIXME: assuming "false" as default would make more sense, but also break compatibility with older save files
    mHasCustomState = true;

    while (esm.getSubRecordHeader())
    {
        const ESM3::SubRecordHeader& subHdr = esm.subRecordHeader();
        switch (subHdr.typeId)
        {
            case ESM3::SUB_HLOC:
            {
                esm.get(mHasLocals);

                if (mHasLocals)
                    mLocals.load (esm);
                break;
            }
            case ESM3::SUB_LUAS:
            {
                esm.cacheSubRecordHeader();
                mLuaScripts.load(esm);
                break;
            }
            case ESM3::SUB_ANIS:
            {
                esm.cacheSubRecordHeader();
                mAnimationState.load(esm);
                break;
            }
            case ESM3::SUB_ENAB: esm.get(mEnabled); break;
            case ESM3::SUB_COUN: esm.get(mCount); break;
            case ESM3::SUB_FLAG: esm.get(mFlags); break;
            case ESM3::SUB_HCUS: esm.get(mHasCustomState); break;
            case ESM3::SUB_POS_:
            {
                if (subHdr.dataSize != 24 && sizeof(mPosition) != 24)
                    esm.fail("ObjectState: Position data incorrect size");
                esm.get(mPosition);
                break;
            }
            case ESM3::SUB_LROT: // local rotation, no longer used
            case ESM3::SUB_LTIM: // obsolete, unused
            {
                esm.skipSubRecordData();
                break;
            }
            default:
                esm.cacheSubRecordHeader();
                return;
        }
    }
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
