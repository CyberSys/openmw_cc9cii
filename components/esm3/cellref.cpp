#include "cellref.hpp"

#include <components/debug/debuglog.hpp>

#include "reader.hpp"
#include "../esm/esmwriter.hpp"

namespace ESM3
{
    int GroundcoverIndex = std::numeric_limits<int>::max();

    void RefNum::load (Reader& reader, bool wide, std::uint32_t tag)
    {
        const ESM3::SubRecordHeader& subHdr = reader.subRecordHeader();
        if (subHdr.typeId != tag)
            reader.fail("Expected subrecord " + ESM::printName(tag)
                        + " but got " + ESM::printName(subHdr.typeId));

        if (wide)
            reader.get(*this, 8);
        else
            reader.get(mIndex);
    }

    void RefNum::save (ESM::ESMWriter& esm, bool wide, const std::string& tag) const
    {
        if (wide)
            esm.writeHNT (tag, *this, 8);
        else
        {
            if (isSet() && !hasContentFile())
                Log(Debug::Error) << "Generated RefNum can not be saved in 32bit format";
            int refNum = (mIndex & 0xffffff) | ((hasContentFile() ? mContentFile : 0xff)<<24);
            esm.writeHNT (tag, refNum, 4);
        }
    }


    void CellRef::load (Reader& reader, bool& isDeleted, bool wideRefNum)
    {
        loadId(reader, wideRefNum);
        loadData(reader, isDeleted);
    }

    // NOTE: assumes sub-record header was read
    void CellRef::loadId (Reader& reader, bool wideRefNum)
    {
        // FIXME: NAM0, if occurs, comes before FRMR but the logic below has it coming in between
        //        FRMR sub-record and its data which is clearly wrong
        //        (should be moved to Cell::getNextRef())
#if 0
        // According to Hrnchamd, this does not belong to the actual ref. Instead, it is a
        // marker indicating that the following refs are part of a "temp refs" section. A temp
        // ref is not being tracked by the moved references system.  Its only purpose is a
        // performance optimization for "immovable" things. We don't need this, and it's
        // problematic anyway, because any item can theoretically be moved by a script.
        if (reader.getNextSubRecordHeader(ESM3::SUB_NAM0))
            reader.skipSubRecordData();
#endif
        blank();

        mRefNum.load (reader, wideRefNum); // get the reference id for FRMR

        reader.getSubRecordHeader(ESM3::SUB_NAME); // assumed that "NAME" follows "FRMR"
        reader.getZString(mRefID);
        if (mRefID.empty())
        {
            Log(Debug::Warning)
                << "Warning: got CellRef with empty RefId in "
                << reader.getFileName() << " 0x" << std::hex << reader.getFileOffset();
        }
    }

    bool CellRef::loadData(Reader& reader, bool& isDeleted)
    {
        isDeleted = false;

        bool isLoaded = false;
        while (!isLoaded && reader.hasMoreSubs() && reader.getSubRecordHeader())
        {
            const ESM3::SubRecordHeader& subHdr = reader.subRecordHeader();
            switch (subHdr.typeId)
            {
                case ESM3::SUB_UNAM: reader.get(mReferenceBlocked); break;
                case ESM3::SUB_XSCL:
                {
                    reader.get(mScale);
                    mScale = std::clamp(mScale, 0.5f, 2.0f);
                    break;
                }
                case ESM3::SUB_ANAM: reader.getZString(mOwner); break;
                case ESM3::SUB_BNAM: reader.getZString(mGlobalVariable); break;
                case ESM3::SUB_XSOL: reader.getZString(mSoul); break;
                case ESM3::SUB_CNAM: reader.getZString(mFaction); break;
                case ESM3::SUB_INDX: reader.get(mFactionRank); break;
                case ESM3::SUB_XCHG: reader.get(mEnchantmentCharge); break;
                case ESM3::SUB_INTV: reader.get(mChargeInt); break;
                case ESM3::SUB_NAM9: reader.get(mGoldValue); break;
                case ESM3::SUB_DODT:
                {
                    reader.get(mDoorDest);
                    mTeleport = true;
                    break;
                }
                case ESM3::SUB_DNAM: reader.getZString(mDestCell); break;
                case ESM3::SUB_FLTV: reader.get(mLockLevel); break;
                case ESM3::SUB_KNAM: reader.getZString(mKey); break;
                case ESM3::SUB_TNAM: reader.getZString(mTrap); break;
                case ESM3::SUB_DATA:
                {
                    if (subHdr.dataSize != sizeof(mPos) || subHdr.dataSize != 24)
                        reader.fail("REFR incorrect data size");
                    reader.get(mPos);
                    break;
                }
                case ESM3::SUB_NAM0:
                {
                    reader.skipSubRecordData();
                    break;
                }
                case ESM3::SUB_DELE:
                {
                    reader.skipSubRecordData();
                    isDeleted = true;
                    break;
                }
                default:
                    reader.cacheSubRecordHeader(); // blech
                    isLoaded = true;
                    break;
            }
        }

        if (mLockLevel == 0 && !mKey.empty())
        {
            mLockLevel = UnbreakableLock;
            mTrap.clear();
        }

        return isLoaded;
    }

    void CellRef::save (ESM::ESMWriter& esm, bool wideRefNum, bool inInventory, bool isDeleted) const
    {
        mRefNum.save (esm, wideRefNum);

        esm.writeHNCString("NAME", mRefID);

        if (isDeleted) {
            esm.writeHNString("DELE", "", 3);
            return;
        }

        if (mScale != 1.0) {
            esm.writeHNT("XSCL", std::clamp(mScale, 0.5f, 2.0f));
        }

        if (!inInventory)
            esm.writeHNOCString("ANAM", mOwner);

        esm.writeHNOCString("BNAM", mGlobalVariable);
        esm.writeHNOCString("XSOL", mSoul);

        if (!inInventory)
        {
            esm.writeHNOCString("CNAM", mFaction);
            if (mFactionRank != -2)
            {
                esm.writeHNT("INDX", mFactionRank);
            }
        }

        if (mEnchantmentCharge != -1)
            esm.writeHNT("XCHG", mEnchantmentCharge);

        if (mChargeInt != -1)
            esm.writeHNT("INTV", mChargeInt);

        if (mGoldValue > 1)
            esm.writeHNT("NAM9", mGoldValue);

        if (!inInventory && mTeleport)
        {
            esm.writeHNT("DODT", mDoorDest);
            esm.writeHNOCString("DNAM", mDestCell);
        }

        if (!inInventory && mLockLevel != 0) {
            esm.writeHNT("FLTV", mLockLevel);
        }

        if (!inInventory)
        {
            esm.writeHNOCString ("KNAM", mKey);
            esm.writeHNOCString ("TNAM", mTrap);
        }

        if (mReferenceBlocked != -1)
            esm.writeHNT("UNAM", mReferenceBlocked);

        if (!inInventory)
            esm.writeHNT("DATA", mPos, 24);
    }

    void CellRef::blank()
    {
        mRefNum.unset();
        mRefID.clear();
        mScale = 1;
        mOwner.clear();
        mGlobalVariable.clear();
        mSoul.clear();
        mFaction.clear();
        mFactionRank = -2;
        mChargeInt = -1;
        mChargeIntRemainder = 0.0f;
        mEnchantmentCharge = -1;
        mGoldValue = 1;
        mDestCell.clear();
        mLockLevel = 0;
        mKey.clear();
        mTrap.clear();
        mReferenceBlocked = -1;
        mTeleport = false;

        for (int i=0; i<3; ++i)
        {
            mDoorDest.pos[i] = 0;
            mDoorDest.rot[i] = 0;
            mPos.pos[i] = 0;
            mPos.rot[i] = 0;
        }
    }
}
