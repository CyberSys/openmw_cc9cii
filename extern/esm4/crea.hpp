/*
  Copyright (C) 2016 cc9cii

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  cc9cii cc9c@iinet.net.au

*/
#ifndef ESM4_CREA_H
#define ESM4_CREA_H

#include <vector>

#include "common.hpp"

namespace ESM4
{
    class Reader;
    class Writer;

    struct Creature
    {
#pragma pack(push, 1)
        struct Data
        {
            std::uint8_t  unknown;
            std::uint8_t  combat;
            std::uint8_t  magic;
            std::uint8_t  stealth;
            std::uint16_t soul;
            std::uint16_t health;
            std::uint16_t unknown2;
            std::uint16_t damage;
            AttributeValues attribs;
        };
#pragma pack(pop)

        FormId mFormId;       // from the header
        std::uint32_t mFlags; // from the header, see enum type RecordFlag for details

        std::string mEditorId;
        std::string mFullName;
        std::string mModel;

        FormId mDeathItem;
        std::vector<FormId> mSpell;
        FormId mScript;

        AIData mAIData;
        std::vector<FormId> mAIPackages;
        ActorBaseConfig mBaseConfig;
        ActorFaction mFaction;
        Data   mData;
        FormId mCombatStyle;
        FormId mSoundBase;
        FormId mSound;
        std::uint8_t mSoundChance;
        float mBaseScale;
        float mTurningSpeed;
        float mFootWeight;
        std::string mBloodSpray;
        std::string mBloodDecal;

        float mBoundRadius;
        std::vector<std::string> mNif; // NIF filenames, get directory from mModel
        std::vector<std::string> mKf;

        std::vector<InventoryItem> mInventory;

        Creature();
        ~Creature();

        void load(ESM4::Reader& reader);
        //void save(ESM4::Writer& reader) const;

        //void blank();
    };
}

#endif // ESM4_CREA_H