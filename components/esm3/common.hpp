#ifndef ESM3_COMMON_H
#define ESM3_COMMON_H

#include <cstdint>
#include <string>

// From ScummVM's endianness.h but for little endian
#ifndef MKTAG
#define MKTAG(a0,a1,a2,a3) ((std::uint32_t)((a0) | ((a1) << 8) | ((a2) << 16) | ((a3) << 24)))
#endif

namespace ESM3
{
    // Based on http://www.uesp.net/wiki
    enum RecordTypes
    {
      //REC_AACT = MKTAG('A','A','C','T'), // Action
      //REC_ACHR = MKTAG('A','C','H','R'), // Actor Reference
        REC_ACTI = MKTAG('A','C','T','I'), // Activator
      //REC_ADDN = MKTAG('A','D','D','N'), // Addon Node
        REC_ALCH = MKTAG('A','L','C','H'), // Potion
      //REC_AMMO = MKTAG('A','M','M','O'), // Ammo
      //REC_ANIO = MKTAG('A','N','I','O'), // Animated Object
        REC_APPA = MKTAG('A','P','P','A'), // Apparatus
      //REC_ARMA = MKTAG('A','R','M','A'), // Armature (Model)
        REC_ARMO = MKTAG('A','R','M','O'), // Armor
      //REC_ARTO = MKTAG('A','R','T','O'), // Art Object
      //REC_ASPC = MKTAG('A','S','P','C'), // Acoustic Space
      //REC_ASTP = MKTAG('A','S','T','P'), // Association Type
      //REC_AVIF = MKTAG('A','V','I','F'), // Actor Values/Perk Tree Graphics
        REC_BSGN = MKTAG('B','S','G','N'), // Birthsign
        REC_BOOK = MKTAG('B','O','O','K'), // Book
      //REC_BPTD = MKTAG('B','P','T','D'), // Body Part Data
      //REC_CAMS = MKTAG('C','A','M','S'), // Camera Shot
        REC_CELL = MKTAG('C','E','L','L'), // Cell
        REC_CLAS = MKTAG('C','L','A','S'), // Class
      //REC_CLFM = MKTAG('C','L','F','M'), // Color
      //REC_CLMT = MKTAG('C','L','M','T'), // Climate
        REC_CLOT = MKTAG('C','L','O','T'), // Clothing
      //REC_COBJ = MKTAG('C','O','B','J'), // Constructible Object (recipes)
      //REC_COLL = MKTAG('C','O','L','L'), // Collision Layer
        REC_CONT = MKTAG('C','O','N','T'), // Container
      //REC_CPTH = MKTAG('C','P','T','H'), // Camera Path
        REC_CREA = MKTAG('C','R','E','A'), // Creature
      //REC_CSTY = MKTAG('C','S','T','Y'), // Combat Style
      //REC_DEBR = MKTAG('D','E','B','R'), // Debris
        REC_DIAL = MKTAG('D','I','A','L'), // Dialog Topic
      //REC_DLBR = MKTAG('D','L','B','R'), // Dialog Branch
      //REC_DLVW = MKTAG('D','L','V','W'), // Dialog View
      //REC_DOBJ = MKTAG('D','O','B','J'), // Default Object Manager
        REC_DOOR = MKTAG('D','O','O','R'), // Door
      //REC_DUAL = MKTAG('D','U','A','L'), // Dual Cast Data (possibly unused)
      //REC_ECZN = MKTAG('E','C','Z','N'), // Encounter Zone
      //REC_EFSH = MKTAG('E','F','S','H'), // Effect Shader
        REC_ENCH = MKTAG('E','N','C','H'), // Enchantment
      //REC_EQUP = MKTAG('E','Q','U','P'), // Equip Slot (flag-type values)
      //REC_EXPL = MKTAG('E','X','P','L'), // Explosion
      //REC_EYES = MKTAG('E','Y','E','S'), // Eyes
        REC_FACT = MKTAG('F','A','C','T'), // Faction
      //REC_FLOR = MKTAG('F','L','O','R'), // Flora
      //REC_FLST = MKTAG('F','L','S','T'), // Form List (non-levelled list)
      //REC_FSTP = MKTAG('F','S','T','P'), // Footstep
      //REC_FSTS = MKTAG('F','S','T','S'), // Footstep Set
      //REC_FURN = MKTAG('F','U','R','N'), // Furniture
        REC_GLOB = MKTAG('G','L','O','B'), // Global Variable
        REC_GMST = MKTAG('G','M','S','T'), // Game Setting
      //REC_GRAS = MKTAG('G','R','A','S'), // Grass
      //REC_GRUP = MKTAG('G','R','U','P'), // Form Group
      //REC_HAIR = MKTAG('H','A','I','R'), // Hair
      //REC_HAZD = MKTAG('H','A','Z','D'), // Hazard
      //REC_HDPT = MKTAG('H','D','P','T'), // Head Part
      //REC_IDLE = MKTAG('I','D','L','E'), // Idle Animation
      //REC_IDLM = MKTAG('I','D','L','M'), // Idle Marker
      //REC_IMAD = MKTAG('I','M','A','D'), // Image Space Modifier
      //REC_IMGS = MKTAG('I','M','G','S'), // Image Space
        REC_INFO = MKTAG('I','N','F','O'), // Dialog Topic Info
        REC_INGR = MKTAG('I','N','G','R'), // Ingredient
      //REC_IPCT = MKTAG('I','P','C','T'), // Impact Data
      //REC_IPDS = MKTAG('I','P','D','S'), // Impact Data Set
      //REC_KEYM = MKTAG('K','E','Y','M'), // Key
      //REC_KYWD = MKTAG('K','Y','W','D'), // Keyword
        REC_LAND = MKTAG('L','A','N','D'), // Land
      //REC_LCRT = MKTAG('L','C','R','T'), // Location Reference Type
      //REC_LCTN = MKTAG('L','C','T','N'), // Location
      //REC_LGTM = MKTAG('L','G','T','M'), // Lighting Template
        REC_LIGH = MKTAG('L','I','G','H'), // Light
        REC_LOCK = MKTAG('L','O','C','K'), // Lock
      //REC_LSCR = MKTAG('L','S','C','R'), // Load Screen
        REC_LTEX = MKTAG('L','T','E','X'), // Land Texture
      //REC_LVLC = MKTAG('L','V','L','C'), // Leveled Creature
      //REC_LVLI = MKTAG('L','V','L','I'), // Leveled Item
        REC_LEVC = MKTAG('L','E','V','C'), // Leveled Creature
        REC_LEVI = MKTAG('L','E','V','I'), // Leveled Item
      //REC_LVLN = MKTAG('L','V','L','N'), // Leveled Actor
      //REC_LVSP = MKTAG('L','V','S','P'), // Leveled Spell
      //REC_MATO = MKTAG('M','A','T','O'), // Material Object
      //REC_MATT = MKTAG('M','A','T','T'), // Material Type
      //REC_MESG = MKTAG('M','E','S','G'), // Message
        REC_MGEF = MKTAG('M','G','E','F'), // Magic Effect
        REC_MISC = MKTAG('M','I','S','C'), // Misc. Object
      //REC_MOVT = MKTAG('M','O','V','T'), // Movement Type
      //REC_MSTT = MKTAG('M','S','T','T'), // Movable Static
      //REC_MUSC = MKTAG('M','U','S','C'), // Music Type
      //REC_MUST = MKTAG('M','U','S','T'), // Music Track
      //REC_NAVI = MKTAG('N','A','V','I'), // Navigation (master data)
      //REC_NAVM = MKTAG('N','A','V','M'), // Nav Mesh
      //REC_NOTE = MKTAG('N','O','T','E'), // Note
        REC_NPC_ = MKTAG('N','P','C','_'), // Actor (NPC, Creature)
      //REC_OTFT = MKTAG('O','T','F','T'), // Outfit
      //REC_PACK = MKTAG('P','A','C','K'), // AI Package
      //REC_PERK = MKTAG('P','E','R','K'), // Perk
      //REC_PGRE = MKTAG('P','G','R','E'), // Placed grenade
      //REC_PHZD = MKTAG('P','H','Z','D'), // Placed hazard
        REC_PROB = MKTAG('P','R','O','B'), // Probe
      //REC_PROJ = MKTAG('P','R','O','J'), // Projectile
      //REC_QUST = MKTAG('Q','U','S','T'), // Quest
        REC_RACE = MKTAG('R','A','C','E'), // Race / Creature type
        REC_REPA = MKTAG('R','E','P','A'), // Repair
      //REC_REFR = MKTAG('R','E','F','R'), // Object Reference
        REC_REGN = MKTAG('R','E','G','N'), // Region (Audio/Weather)
      //REC_RELA = MKTAG('R','E','L','A'), // Relationship
      //REC_REVB = MKTAG('R','E','V','B'), // Reverb Parameters
      //REC_RFCT = MKTAG('R','F','C','T'), // Visual Effect
      //REC_SBSP = MKTAG('S','B','S','P'), // Subspace (TES4 only?)
      //REC_SCEN = MKTAG('S','C','E','N'), // Scene
        REC_SCPT = MKTAG('S','C','P','T'), // Script
      //REC_SCRL = MKTAG('S','C','R','L'), // Scroll
      //REC_SGST = MKTAG('S','G','S','T'), // Sigil Stone
      //REC_SHOU = MKTAG('S','H','O','U'), // Shout
      //REC_SLGM = MKTAG('S','L','G','M'), // Soul Gem
      //REC_SMBN = MKTAG('S','M','B','N'), // Story Manager Branch Node
      //REC_SMEN = MKTAG('S','M','E','N'), // Story Manager Event Node
      //REC_SMQN = MKTAG('S','M','Q','N'), // Story Manager Quest Node
      //REC_SNCT = MKTAG('S','N','C','T'), // Sound Category
      //REC_SNDR = MKTAG('S','N','D','R'), // Sound Reference
      //REC_SOPM = MKTAG('S','O','P','M'), // Sound Output Model
        REC_SOUN = MKTAG('S','O','U','N'), // Sound
        REC_SPEL = MKTAG('S','P','E','L'), // Spell
      //REC_SPGD = MKTAG('S','P','G','D'), // Shader Particle Geometry
        REC_STAT = MKTAG('S','T','A','T'), // Static
      //REC_TACT = MKTAG('T','A','C','T'), // Talking Activator
      //REC_TERM = MKTAG('T','E','R','M'), // Terminal
        REC_TES3 = MKTAG('T','E','S','3'), // Plugin info
      //REC_TES4 = MKTAG('T','E','S','4'), // Plugin info
      //REC_TREE = MKTAG('T','R','E','E'), // Tree
      //REC_TXST = MKTAG('T','X','S','T'), // Texture Set
      //REC_VTYP = MKTAG('V','T','Y','P'), // Voice Type
      //REC_WATR = MKTAG('W','A','T','R'), // Water Type
        REC_WEAP = MKTAG('W','E','A','P'), // Weapon
      //REC_WOOP = MKTAG('W','O','O','P'), // Word Of Power
      //REC_WRLD = MKTAG('W','R','L','D'), // World Space
      //REC_WTHR = MKTAG('W','T','H','R'), // Weather
      //REC_ACRE = MKTAG('A','C','R','E'), // Placed Creature (TES4 only?)
        REC_PGRD = MKTAG('P','G','R','D'), // Pathgrid
      //REC_ROAD = MKTAG('R','O','A','D'), // Road (TES4 only?)
      //REC_IMOD = MKTAG('I','M','O','D'), // Item Mod
      //REC_PWAT = MKTAG('P','W','A','T'), // Placeable Water
      //REC_SCOL = MKTAG('S','C','O','L'), // Static Collection
      //REC_CCRD = MKTAG('C','C','R','D'), // Caravan Card
      //REC_CMNY = MKTAG('C','M','N','Y'), // Caravan Money
      //REC_ALOC = MKTAG('A','L','O','C'), // Audio Location Controller
      //REC_MSET = MKTAG('M','S','E','T'), // Media Set
        REC_BODY = MKTAG('B','O','D','Y'), // Body Part
        REC_SNDG = MKTAG('S','N','D','G'), // Sound Generator
        REC_SSCR = MKTAG('S','S','C','R'), // Start Script
        REC_SKIL = MKTAG('S','K','I','L'), // Skill
        REC_FILT = MKTAG('F','I','L','T'), // Filter
        REC_DBGP = MKTAG('D','B','G','P')  // Debug Profile
    };

    enum SubRecordTypes
    {
        SUB_HEDR = MKTAG('H','E','D','R'),
        SUB_CNAM = MKTAG('C','N','A','M'),
        SUB_FORM = MKTAG('F','O','R','M'),
        SUB_MAST = MKTAG('M','A','S','T'),
        SUB_DATA = MKTAG('D','A','T','A'),
        SUB_GMDT = MKTAG('G','M','D','T'), // TES3

        SUB_AIDT = MKTAG('A','I','D','T'),
        SUB_AI_A = MKTAG('A','I','_','A'),
        SUB_AI_E = MKTAG('A','I','_','E'),
        SUB_AI_F = MKTAG('A','I','_','F'),
        SUB_AI_T = MKTAG('A','I','_','T'),
        SUB_AI_W = MKTAG('A','I','_','W'),
        SUB_AMBI = MKTAG('A','M','B','I'),
        SUB_ANAM = MKTAG('A','N','A','M'),
        SUB_ASND = MKTAG('A','S','N','D'),
        SUB_AVFX = MKTAG('A','V','F','X'),
        SUB_BNAM = MKTAG('B','N','A','M'),
        SUB_BSND = MKTAG('B','S','N','D'),
        SUB_BVFX = MKTAG('B','V','F','X'),
        SUB_CIDX = MKTAG('C','I','D','X'),
        SUB_CNDT = MKTAG('C','N','D','T'),
        SUB_CSND = MKTAG('C','S','N','D'),
        SUB_CTDT = MKTAG('C','T','D','T'),
        SUB_CVFX = MKTAG('C','V','F','X'),
        SUB_DELE = MKTAG('D','E','L','E'),
        SUB_DESC = MKTAG('D','E','S','C'),
        SUB_DNAM = MKTAG('D','N','A','M'),
        SUB_DODT = MKTAG('D','O','D','T'),
        SUB_ENAM = MKTAG('E','N','A','M'),
        SUB_FILT = MKTAG('F','I','L','T'),
        SUB_FLAG = MKTAG('F','L','A','G'),
        SUB_FLTV = MKTAG('F','L','T','V'),
        SUB_FNAM = MKTAG('F','N','A','M'),
        SUB_HSND = MKTAG('H','S','N','D'),
        SUB_HVFX = MKTAG('H','V','F','X'),
        SUB_INAM = MKTAG('I','N','A','M'),
        SUB_INDX = MKTAG('I','N','D','X'),
        SUB_INTV = MKTAG('I','N','T','V'),
        SUB_ITEX = MKTAG('I','T','E','X'),
        SUB_KNAM = MKTAG('K','N','A','M'),
        SUB_LOCA = MKTAG('L','O','C','A'),
        SUB_MODL = MKTAG('M','O','D','L'),
        SUB_NAM0 = MKTAG('N','A','M','0'),
        SUB_NAM5 = MKTAG('N','A','M','5'),
        SUB_NAM9 = MKTAG('N','A','M','9'),
        SUB_NAME = MKTAG('N','A','M','E'),
        SUB_NNAM = MKTAG('N','N','A','M'),
        SUB_NPCO = MKTAG('N','P','C','O'),
        SUB_NPCS = MKTAG('N','P','C','S'),
        SUB_NPDT = MKTAG('N','P','D','T'),
        SUB_ONAM = MKTAG('O','N','A','M'),
        SUB_PGRC = MKTAG('P','G','R','C'),
        SUB_PGRP = MKTAG('P','G','R','P'),
        SUB_PNAM = MKTAG('P','N','A','M'),
        SUB_PTEX = MKTAG('P','T','E','X'),
        SUB_QSTF = MKTAG('Q','S','T','F'),
        SUB_QSTN = MKTAG('Q','S','T','N'),
        SUB_QSTR = MKTAG('Q','S','T','R'),
        SUB_RGNN = MKTAG('R','G','N','N'),
        SUB_RNAM = MKTAG('R','N','A','M'),
        SUB_SCHD = MKTAG('S','C','H','D'),
        SUB_SCRD = MKTAG('S','C','R','D'),
        SUB_SCRI = MKTAG('S','C','R','I'),
        SUB_SCRP = MKTAG('S','C','R','P'),
        SUB_SCRS = MKTAG('S','C','R','S'),
        SUB_SCTX = MKTAG('S','C','T','X'),
        SUB_SCVR = MKTAG('S','C','V','R'),
        SUB_SKDT = MKTAG('S','K','D','T'),
        SUB_SNAM = MKTAG('S','N','A','M'),
        SUB_SPAC = MKTAG('S','P','A','C'),
        SUB_STRV = MKTAG('S','T','R','V'),
        SUB_STTV = MKTAG('S','T','T','V'),
        SUB_TEXT = MKTAG('T','E','X','T'),
        SUB_TNAM = MKTAG('T','N','A','M'),
        SUB_UNAM = MKTAG('U','N','A','M'),
        SUB_VCLR = MKTAG('V','C','L','R'),
        SUB_VHGT = MKTAG('V','H','G','T'),
        SUB_VNML = MKTAG('V','N','M','L'),
        SUB_VTEX = MKTAG('V','T','E','X'),
        SUB_WHGT = MKTAG('W','H','G','T'),
        SUB_WNAM = MKTAG('W','N','A','M'),
        SUB_XCHG = MKTAG('X','C','H','G'),
        SUB_XSCL = MKTAG('X','S','C','L'),
        SUB_XSOL = MKTAG('X','S','O','L'),
        SUB_WEAT = MKTAG('W','E','A','T'), // Region
        SUB_SCDT = MKTAG('S','C','D','T'), // Script
        SUB_MEDT = MKTAG('M','E','D','T'), // Magic Effect
        SUB_RADT = MKTAG('R','A','D','T'), // Race
        SUB_BYDT = MKTAG('B','Y','D','T'), // Body
        SUB_CLDT = MKTAG('C','L','D','T'), // Class
        SUB_ALDT = MKTAG('A','L','D','T'), // Potion
        SUB_AADT = MKTAG('A','A','D','T'), // Apparatus
        SUB_AODT = MKTAG('A','O','D','T'), // Armor
        SUB_BKDT = MKTAG('B','K','D','T'), // Book
        SUB_IRDT = MKTAG('I','R','D','T'), // Ingredient
        SUB_LHDT = MKTAG('L','H','D','T'), // Light
        SUB_LKDT = MKTAG('L','K','D','T'), // Lock
        SUB_PBDT = MKTAG('P','B','D','T'), // Probe
        SUB_RIDT = MKTAG('R','I','D','T'), // Repair
        SUB_WPDT = MKTAG('W','P','D','T'), // Weapon
        SUB_MCDT = MKTAG('M','C','D','T'), // Misc
        SUB_SPDT = MKTAG('S','P','D','T'), // Spell
        SUB_FADT = MKTAG('F','A','D','T'), // Faction
        SUB_ENDT = MKTAG('E','N','D','T'), // Enchant

        SUB_MVRF = MKTAG('M','V','R','F'), // Moved Reference
        SUB_FRMR = MKTAG('F','R','M','R')  // Form Reference
    };

    enum RecordFlag
    {
        Rec_Deleted    = 0x00000020, // Deleted
        Rec_Persistent = 0x00000400, // Persistent reference
        Rec_Disabled   = 0x00000800, // Initially disabled
        Rec_Danger     = 0x00002000  // Blocked
    };

#pragma pack(push, 1)
    struct RecordHeader
    {
        std::uint32_t typeId;
        std::uint32_t dataSize;
        std::uint32_t unknown;
        std::uint32_t flags;
    };

    struct SubRecordHeader
    {
        std::uint32_t typeId;
        std::uint32_t dataSize;
    };
#pragma pack(pop)
}

#endif // ESM3_COMMON_H
