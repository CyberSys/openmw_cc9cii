#include "controlsstate.hpp"

#include "reader.hpp"
#include "../esm/esmwriter.hpp"

ESM3::ControlsState::ControlsState()
    : mViewSwitchDisabled(false),
      mControlsDisabled(false),
      mJumpingDisabled(false),
      mLookingDisabled(false),
      mVanityModeDisabled(false),
      mWeaponDrawingDisabled(false),
      mSpellDrawingDisabled(false)
{
}

void ESM3::ControlsState::load(Reader& esm)
{
#if 0
    int flags;
    esm.getHNT(flags, "CFLG");

    mViewSwitchDisabled = flags & ViewSwitchDisabled;
    mControlsDisabled = flags & ControlsDisabled;
    mJumpingDisabled = flags & JumpingDisabled;
    mLookingDisabled = flags & LookingDisabled;
    mVanityModeDisabled = flags & VanityModeDisabled;
    mWeaponDrawingDisabled = flags & WeaponDrawingDisabled;
    mSpellDrawingDisabled = flags & SpellDrawingDisabled;
#endif
}

void ESM3::ControlsState::save(ESM::ESMWriter& esm) const
{
    int flags = 0;
    if (mViewSwitchDisabled) flags |= ViewSwitchDisabled;
    if (mControlsDisabled) flags |= ControlsDisabled;
    if (mJumpingDisabled) flags |= JumpingDisabled;
    if (mLookingDisabled) flags |= LookingDisabled;
    if (mVanityModeDisabled) flags |= VanityModeDisabled;
    if (mWeaponDrawingDisabled) flags |= WeaponDrawingDisabled;
    if (mSpellDrawingDisabled) flags |= SpellDrawingDisabled;

    esm.writeHNT("CFLG", flags);
}
