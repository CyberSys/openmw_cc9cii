#include "projectilestate.hpp"

#include "reader.hpp"
#include "../esm/esmwriter.hpp"

namespace ESM3
{
    void BaseProjectileState::load(Reader& esm)
    {
        esm.getSubRecordHeader(ESM3::SUB_ID__);
        esm.getString(mId); // FIXME: check if string null terminated
        esm.getSubRecordHeader(ESM3::SUB_VEC3);
        esm.get(mPosition);
        esm.getSubRecordHeader(ESM3::SUB_QUAT);
        esm.get(mOrientation);
        esm.getSubRecordHeader(ESM3::SUB_ACTO);
        esm.get(mActorId);
    }

    void BaseProjectileState::save(ESM::ESMWriter& esm) const
    {
        esm.writeHNString ("ID__", mId);
        esm.writeHNT ("VEC3", mPosition);
        esm.writeHNT ("QUAT", mOrientation);
        esm.writeHNT ("ACTO", mActorId);
    }

    void MagicBoltState::load(Reader& esm)
    {
        BaseProjectileState::load(esm);

        esm.getSubRecordHeader(ESM3::SUB_SPEL);
        esm.getString(mSpellId); // FIXME: check string null terminated
        if (esm.getNextSubRecordHeader(ESM3::SUB_SRCN))
            esm.skipSubRecordData();
        ESM3::EffectList().load(esm); // for backwards compatibility
        esm.getSubRecordHeader(ESM3::SUB_SPED);
        esm.get(mSpeed);

        if (esm.getNextSubRecordHeader(ESM3::SUB_STCK))
            esm.skipSubRecordData();
        if (esm.getNextSubRecordHeader(ESM3::SUB_SOUN))
            esm.skipSubRecordData();
    }

    void MagicBoltState::save(ESM::ESMWriter& esm) const
    {
        BaseProjectileState::save(esm);

        esm.writeHNString ("SPEL", mSpellId);
        esm.writeHNT ("SPED", mSpeed);
    }

    void ProjectileState::load(Reader& esm)
    {
        BaseProjectileState::load(esm);

        esm.getSubRecordHeader(ESM3::SUB_BOW_);
        esm.getString(mBowId); // FIXME: check if string null terminated
        esm.getSubRecordHeader(ESM3::SUB_VEL_);
        esm.get(mVelocity);

        mAttackStrength = 1.f;
        if (esm.getNextSubRecordType() == ESM3::SUB_STR_ && esm.getSubRecordHeader())
            esm.get(mAttackStrength);
    }

    void ProjectileState::save(ESM::ESMWriter& esm) const
    {
        BaseProjectileState::save(esm);

        esm.writeHNString ("BOW_", mBowId);
        esm.writeHNT ("VEL_", mVelocity);
        esm.writeHNT ("STR_", mAttackStrength);
    }
}
