#ifndef OPENMW_ESM_UTIL_H
#define OPENMW_ESM_UTIL_H

#include <OgreVector3.h>
#include <OgreQuaternion.h>

#include "esmreader.hpp"
#include "esmwriter.hpp"
#include "loadsscr.hpp"
#include "loadrace.hpp"
#include "loadgmst.hpp"

namespace ESM
{

// format 0, savegames only

struct Quaternion
{
    float mValues[4];

    Quaternion() {}
    Quaternion (Ogre::Quaternion q)
    {
        mValues[0] = q.w;
        mValues[1] = q.x;
        mValues[2] = q.y;
        mValues[3] = q.z;
    }

    operator Ogre::Quaternion () const
    {
        return Ogre::Quaternion(mValues[0], mValues[1], mValues[2], mValues[3]);
    }
};

struct Vector3
{
    float mValues[3];

    Vector3() {}
    Vector3 (Ogre::Vector3 v)
    {
        mValues[0] = v.x;
        mValues[1] = v.y;
        mValues[2] = v.z;
    }

    operator Ogre::Vector3 () const
    {
        return Ogre::Vector3(&mValues[0]);
    }
};

bool readDeleSubRecord(ESMReader &esm);
void writeDeleSubRecord(ESMWriter &esm);

template <class RecordT>
bool isRecordDeleted(const RecordT &record)
{
    return record.mIsDeleted;
}

// The following records can't be deleted (for now)
template <>
bool isRecordDeleted<StartScript>(const StartScript &script);

template <>
bool isRecordDeleted<Race>(const Race &race);

template <>
bool isRecordDeleted<GameSetting>(const GameSetting &gmst);

}

#endif
