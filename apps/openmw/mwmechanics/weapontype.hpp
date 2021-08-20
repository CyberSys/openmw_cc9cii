#ifndef GAME_MWMECHANICS_WEAPONTYPE_H
#define GAME_MWMECHANICS_WEAPONTYPE_H

#include "../mwworld/inventorystore.hpp"

namespace MWMechanics
{
    static std::map<int, ESM3::WeaponType> sWeaponTypeList =
    {
        {
            ESM3::Weapon::None,
            {
                /* short group */ "",
                /* long group  */ "",
                /*  sound ID   */ "",
                /* attach bone */ "",
                /* sheath bone */ "",
                /* usage skill */ ESM3::Skill::HandToHand,
                /* weapon class*/ ESM3::WeaponType::Melee,
                /*  ammo type  */ ESM3::Weapon::None,
                /*    flags    */ 0
            }
        },
        {
            ESM3::Weapon::PickProbe,
            {
                /* short group */ "1h",
                /* long group  */ "pickprobe",
                /*  sound ID   */ "",
                /* attach bone */ "",
                /* sheath bone */ "",
                /* usage skill */ ESM3::Skill::Security,
                /* weapon class*/ ESM3::WeaponType::Melee,
                /*  ammo type  */ ESM3::Weapon::None,
                /*    flags    */ 0
            }
        },
        {
            ESM3::Weapon::Spell,
            {
                /* short group */ "spell",
                /* long group  */ "spellcast",
                /*  sound ID   */ "",
                /* attach bone */ "",
                /* sheath bone */ "",
                /* usage skill */ ESM3::Skill::HandToHand,
                /* weapon class*/ ESM3::WeaponType::Melee,
                /*  ammo type  */ ESM3::Weapon::None,
                /*    flags    */ ESM3::WeaponType::TwoHanded
            }
        },
        {
            ESM3::Weapon::HandToHand,
            {
                /* short group */ "hh",
                /* long group  */ "handtohand",
                /*  sound ID   */ "",
                /* attach bone */ "",
                /* sheath bone */ "",
                /* usage skill */ ESM3::Skill::HandToHand,
                /* weapon class*/ ESM3::WeaponType::Melee,
                /*  ammo type  */ ESM3::Weapon::None,
                /*    flags    */ ESM3::WeaponType::TwoHanded
            }
        },
        {
            ESM3::Weapon::ShortBladeOneHand,
            {
                /* short group */ "1s",
                /* long group  */ "shortbladeonehand",
                /*  sound ID   */ "Item Weapon Shortblade",
                /* attach bone */ "Weapon Bone",
                /* sheath bone */ "Bip01 ShortBladeOneHand",
                /* usage skill */ ESM3::Skill::ShortBlade,
                /* weapon class*/ ESM3::WeaponType::Melee,
                /*  ammo type  */ ESM3::Weapon::None,
                /*    flags    */ ESM3::WeaponType::HasHealth
            }
        },
        {
            ESM3::Weapon::LongBladeOneHand,
            {
                /* short group */ "1h",
                /* long group  */ "weapononehand",
                /*  sound ID   */ "Item Weapon Longblade",
                /* attach bone */ "Weapon Bone",
                /* sheath bone */ "Bip01 LongBladeOneHand",
                /* usage skill */ ESM3::Skill::LongBlade,
                /* weapon class*/ ESM3::WeaponType::Melee,
                /*  ammo type  */ ESM3::Weapon::None,
                /*    flags    */ ESM3::WeaponType::HasHealth
            }
        },
        {
            ESM3::Weapon::BluntOneHand,
            {
                /* short group */ "1b",
                /* long group  */ "bluntonehand",
                /*  sound ID   */ "Item Weapon Blunt",
                /* attach bone */ "Weapon Bone",
                /* sheath bone */ "Bip01 BluntOneHand",
                /* usage skill */ ESM3::Skill::BluntWeapon,
                /* weapon class*/ ESM3::WeaponType::Melee,
                /*  ammo type  */ ESM3::Weapon::None,
                /*    flags    */ ESM3::WeaponType::HasHealth
            }
        },
        {
            ESM3::Weapon::AxeOneHand,
            {
                /* short group */ "1b",
                /* long group  */ "bluntonehand",
                /*  sound ID   */ "Item Weapon Blunt",
                /* attach bone */ "Weapon Bone",
                /* sheath bone */ "Bip01 LongBladeOneHand",
                /* usage skill */ ESM3::Skill::Axe,
                /* weapon class*/ ESM3::WeaponType::Melee,
                /*  ammo type  */ ESM3::Weapon::None,
                /*    flags    */ ESM3::WeaponType::HasHealth
            }
        },
        {
            ESM3::Weapon::LongBladeTwoHand,
            {
                /* short group */ "2c",
                /* long group  */ "weapontwohand",
                /*  sound ID   */ "Item Weapon Longblade",
                /* attach bone */ "Weapon Bone",
                /* sheath bone */ "Bip01 LongBladeTwoClose",
                /* usage skill */ ESM3::Skill::LongBlade,
                /* weapon class*/ ESM3::WeaponType::Melee,
                /*  ammo type  */ ESM3::Weapon::None,
                /*    flags    */ ESM3::WeaponType::HasHealth|ESM3::WeaponType::TwoHanded
            }
        },
        {
            ESM3::Weapon::AxeTwoHand,
            {
                /* short group */ "2b",
                /* long group  */ "blunttwohand",
                /*  sound ID   */ "Item Weapon Blunt",
                /* attach bone */ "Weapon Bone",
                /* sheath bone */ "Bip01 AxeTwoClose",
                /* usage skill */ ESM3::Skill::Axe,
                /* weapon class*/ ESM3::WeaponType::Melee,
                /*  ammo type  */ ESM3::Weapon::None,
                /*    flags    */ ESM3::WeaponType::HasHealth|ESM3::WeaponType::TwoHanded
            }
        },
        {
            ESM3::Weapon::BluntTwoClose,
            {
                /* short group */ "2b",
                /* long group  */ "blunttwohand",
                /*  sound ID   */ "Item Weapon Blunt",
                /* attach bone */ "Weapon Bone",
                /* sheath bone */ "Bip01 BluntTwoClose",
                /* usage skill */ ESM3::Skill::BluntWeapon,
                /* weapon class*/ ESM3::WeaponType::Melee,
                /*  ammo type  */ ESM3::Weapon::None,
                /*    flags    */ ESM3::WeaponType::HasHealth|ESM3::WeaponType::TwoHanded
            }
        },
        {
            ESM3::Weapon::BluntTwoWide,
            {
                /* short group */ "2w",
                /* long group  */ "weapontwowide",
                /*  sound ID   */ "Item Weapon Blunt",
                /* attach bone */ "Weapon Bone",
                /* sheath bone */ "Bip01 BluntTwoWide",
                /* usage skill */ ESM3::Skill::BluntWeapon,
                /* weapon class*/ ESM3::WeaponType::Melee,
                /*  ammo type  */ ESM3::Weapon::None,
                /*    flags    */ ESM3::WeaponType::HasHealth|ESM3::WeaponType::TwoHanded
            }
        },
        {
            ESM3::Weapon::SpearTwoWide,
            {
                /* short group */ "2w",
                /* long group  */ "weapontwowide",
                /*  sound ID   */ "Item Weapon Spear",
                /* attach bone */ "Weapon Bone",
                /* sheath bone */ "Bip01 SpearTwoWide",
                /* usage skill */ ESM3::Skill::Spear,
                /* weapon class*/ ESM3::WeaponType::Melee,
                /*  ammo type  */ ESM3::Weapon::None,
                /*    flags    */ ESM3::WeaponType::HasHealth|ESM3::WeaponType::TwoHanded
            }
        },
        {
            ESM3::Weapon::MarksmanBow,
            {
                /* short group */ "bow",
                /* long group  */ "bowandarrow",
                /*  sound ID   */ "Item Weapon Bow",
                /* attach bone */ "Weapon Bone Left",
                /* sheath bone */ "Bip01 MarksmanBow",
                /* usage skill */ ESM3::Skill::Marksman,
                /* weapon class*/ ESM3::WeaponType::Ranged,
                /*  ammo type  */ ESM3::Weapon::Arrow,
                /*    flags    */ ESM3::WeaponType::HasHealth|ESM3::WeaponType::TwoHanded
            }
        },
        {
            ESM3::Weapon::MarksmanCrossbow,
            {
                /* short group */ "crossbow",
                /* long group  */ "crossbow",
                /*  sound ID   */ "Item Weapon Crossbow",
                /* attach bone */ "Weapon Bone",
                /* sheath bone */ "Bip01 MarksmanCrossbow",
                /* usage skill */ ESM3::Skill::Marksman,
                /* weapon class*/ ESM3::WeaponType::Ranged,
                /*  ammo type  */ ESM3::Weapon::Bolt,
                /*    flags    */ ESM3::WeaponType::HasHealth|ESM3::WeaponType::TwoHanded
            }
        },
        {
            ESM3::Weapon::MarksmanThrown,
            {
                /* short group */ "1t",
                /* long group  */ "throwweapon",
                /*  sound ID   */ "Item Weapon Blunt",
                /* attach bone */ "Weapon Bone",
                /* sheath bone */ "Bip01 MarksmanThrown",
                /* usage skill */ ESM3::Skill::Marksman,
                /* weapon class*/ ESM3::WeaponType::Thrown,
                /*  ammo type  */ ESM3::Weapon::None,
                /*    flags    */ 0
            }
        },
        {
            ESM3::Weapon::Arrow,
            {
                /* short group */ "",
                /* long group  */ "",
                /*  sound ID   */ "Item Ammo",
                /* attach bone */ "Bip01 Arrow",
                /* sheath bone */ "",
                /* usage skill */ ESM3::Skill::Marksman,
                /* weapon class*/ ESM3::WeaponType::Ammo,
                /*  ammo type  */ ESM3::Weapon::None,
                /*    flags    */ 0
            }
        },
        {
            ESM3::Weapon::Bolt,
            {
                /* short group */ "",
                /* long group  */ "",
                /*  sound ID   */ "Item Ammo",
                /* attach bone */ "ArrowBone",
                /* sheath bone */ "",
                /* usage skill */ ESM3::Skill::Marksman,
                /* weapon class*/ ESM3::WeaponType::Ammo,
                /*  ammo type  */ ESM3::Weapon::None,
                /*    flags    */ 0
            }
        }
    };

    MWWorld::ContainerStoreIterator getActiveWeapon(const MWWorld::Ptr& actor, int *weaptype);

    const ESM3::WeaponType* getWeaponType(const int weaponType);
}

#endif
