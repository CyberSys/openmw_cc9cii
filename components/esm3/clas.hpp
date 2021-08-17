#ifndef ESM3_CLAS_H
#define ESM3_CLAS_H

#include <string>
#include <cstdint>

namespace ESM
{
    class ESMWriter;
}

namespace ESM3
{

class Reader;

/*
 * Character class definitions
 */

// These flags tells us which items should be auto-calculated for this
// class
struct Class
{
    static unsigned int sRecordId;
    /// Return a string descriptor for this record type. Currently used for debugging / error logs only.
    static std::string getRecordType() { return "Class"; }

    enum AutoCalc
    {
        Weapon      = 0x00001,
        Armor       = 0x00002,
        Clothing    = 0x00004,
        Books       = 0x00008,
        Ingredient  = 0x00010,
        Lockpick    = 0x00020,
        Probe       = 0x00040,
        Lights      = 0x00080,
        Apparatus   = 0x00100,
        Repair      = 0x00200,
        Misc        = 0x00400,
        Spells      = 0x00800,
        MagicItems  = 0x01000,
        Potions     = 0x02000,
        Training    = 0x04000,
        Spellmaking = 0x08000,
        Enchanting  = 0x10000,
        RepairItem  = 0x20000
    };

    enum Specialization
    {
        Combat  = 0,
        Magic   = 1,
        Stealth = 2
    };

    static const Specialization sSpecializationIds[3];
    static const char *sGmstSpecializationIds[3];

#pragma pack(push, 1)
    // TODO: These used to be signed int - there are existing code that relies on that
    struct CLDTstruct
    {
        std::uint32_t mAttribute[2];   // Attributes that get class bonus
        std::uint32_t mSpecialization; // 0 = Combat, 1 = Magic, 2 = Stealth
        std::uint32_t mSkills[5][2];   // Minor and major skills.
        std::uint32_t mIsPlayable;     // 0x0001 - Playable class

        // I have no idea how to autocalculate these items...
        std::uint32_t mCalc;

        std::uint32_t& getSkill (int index, bool major);
        ///< Throws an exception for invalid values of \a index.

        std::uint32_t getSkill (int index, bool major) const;
        ///< Throws an exception for invalid values of \a index.
    }; // 60 bytes
#pragma pack(pop)

    std::string mId, mName, mDescription;
    CLDTstruct mData;

    void load(Reader& reader, bool& isDeleted);
    void save(ESM::ESMWriter& esm, bool isDeleted = false) const;

    void blank();
     ///< Set record to default state (does not touch the ID/index).

};
}
#endif
