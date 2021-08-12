#ifndef ESM3_PROJECTILESTATE_H
#define ESM3_PROJECTILESTATE_H

#include <string>

#include <osg/Quat>
#include <osg/Vec3f>

#include "effectlist.hpp"

#include "../esm/util.hpp"

namespace ESM3
{
    // format 0, savegames only

    struct BaseProjectileState
    {
        std::string mId;

        Vector3 mPosition;
        Quaternion mOrientation;

        int mActorId;

        void load (Reader& esm);
        void save (ESM::ESMWriter& esm) const;
    };

    struct MagicBoltState : public BaseProjectileState
    {
        std::string mSpellId;
        float mSpeed;

        void load (Reader& esm);
        void save (ESM::ESMWriter& esm) const;
    };

    struct ProjectileState : public BaseProjectileState
    {
        std::string mBowId;
        Vector3 mVelocity;
        float mAttackStrength;

        void load (Reader& esm);
        void save (ESM::ESMWriter& esm) const;
    };
}

#endif
