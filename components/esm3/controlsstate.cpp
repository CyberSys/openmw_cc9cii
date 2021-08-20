#include "controlsstate.hpp"

#include "reader.hpp"
#include "../esm/esmwriter.hpp"

namespace ESM3
{
    ControlsState::ControlsState()
        : mViewSwitchDisabled(false),
          mControlsDisabled(false),
          mJumpingDisabled(false),
          mLookingDisabled(false),
          mVanityModeDisabled(false),
          mWeaponDrawingDisabled(false),
          mSpellDrawingDisabled(false)
    {
    }

    // NOTE: equivalent to "player flags" in SUB_PNAM of REC_PCDT
    // (called from StateManager::loadGame() via InputManager::readRecord())
    void ControlsState::load(Reader& esm)
    {
        while (esm.getSubRecordHeader())
        {
            const ESM3::SubRecordHeader& subHdr = esm.subRecordHeader();
            switch (subHdr.typeId)
            {
                case ESM3::SUB_CFLG:
                {
                    int flags;
                    esm.get(flags);

                    mViewSwitchDisabled =    flags & ViewSwitchDisabled;
                    mControlsDisabled =      flags & ControlsDisabled;
                    mJumpingDisabled =       flags & JumpingDisabled;
                    mLookingDisabled =       flags & LookingDisabled;
                    mVanityModeDisabled =    flags & VanityModeDisabled;
                    mWeaponDrawingDisabled = flags & WeaponDrawingDisabled;
                    mSpellDrawingDisabled =  flags & SpellDrawingDisabled;
                    break;
                }
                default:
                    esm.skipSubRecordData();
                    break;
            }
        }
    }

    void ControlsState::save(ESM::ESMWriter& esm) const
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
}
