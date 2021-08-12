#ifndef OPENMW_ESSIMPORT_CONTEXT_H
#define OPENMW_ESSIMPORT_CONTEXT_H

#include <map>

#include <components/esm3/npc_.hpp>
#include <components/esm3/player.hpp>
#include <components/esm3/dialoguestate.hpp>
#include <components/esm3/globalmap.hpp>
#include <components/esm3/crea.hpp>
#include <components/esm3/controlsstate.hpp>

#include "importnpcc.hpp"
#include "importcrec.hpp"
#include "importcntc.hpp"
#include "importplayer.hpp"
#include "importsplm.h"

namespace ESSImport
{
    struct Context
    {
        // set from the TES3 header
        std::string mPlayerCellName;

        ESM3::Player mPlayer;
        ESM3::NPC mPlayerBase;
        std::string mCustomPlayerClassName;

        ESM3::DialogueState mDialogueState;

        ESM3::ControlsState mControlsState;

        // cells which should show an explored overlay on the global map
        std::set<std::pair<int, int> > mExploredCells;

        ESM3::GlobalMap mGlobalMapState;

        int mDay, mMonth, mYear;
        float mHour;

        // key <refIndex, refId>
        std::map<std::pair<int, std::string>, CREC> mCreatureChanges;
        std::map<std::pair<int, std::string>, NPCC> mNpcChanges;
        std::map<std::pair<int, std::string>, CNTC> mContainerChanges;

        std::map<std::pair<int, std::string>, int> mActorIdMap;
        int mNextActorId;

        std::map<std::string, ESM3::Creature> mCreatures;
        std::map<std::string, ESM3::NPC> mNpcs;

        std::vector<SPLM::ActiveSpell> mActiveSpells;

        Context()
            : mDay(0)
            , mMonth(0)
            , mYear(0)
            , mHour(0.f)
            , mNextActorId(0)
        {
            ESM3::CellId playerCellId;
            playerCellId.mPaged = true;
            playerCellId.mIndex.mX = playerCellId.mIndex.mY = 0;
            mPlayer.mCellId = playerCellId;
            mPlayer.mLastKnownExteriorPosition[0]
                = mPlayer.mLastKnownExteriorPosition[1]
                = mPlayer.mLastKnownExteriorPosition[2]
                = 0.0f;
            mPlayer.mHasMark = 0;
            mPlayer.mCurrentCrimeId = -1; // TODO
            mPlayer.mPaidCrimeId = -1;
            mPlayer.mObject.blank();
            mPlayer.mObject.mEnabled = true;
            mPlayer.mObject.mRef.mRefID = "player"; // REFR.mRefID would be PlayerSaveGame
            mPlayer.mObject.mCreatureStats.mActorId = generateActorId();

            mGlobalMapState.mBounds.mMinX = 0;
            mGlobalMapState.mBounds.mMaxX = 0;
            mGlobalMapState.mBounds.mMinY = 0;
            mGlobalMapState.mBounds.mMaxY = 0;

            mPlayerBase.blank();
        }

        int generateActorId()
        {
            return mNextActorId++;
        }
    };

}

#endif
