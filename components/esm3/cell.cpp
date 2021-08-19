#include "cell.hpp"

#include <string>
#include <limits>
#include <list>

#include <boost/concept_check.hpp>

#include <components/debug/debuglog.hpp>
#include <components/misc/stringops.hpp>

#include "common.hpp"
#include "reader.hpp"
#include "../esm/esmwriter.hpp"
#include "cellid.hpp"

namespace
{
    ///< Translate 8bit/24bit code (stored in refNum.mIndex) into a proper refNum
    void adjustRefNum (ESM3::RefNum& refNum, ESM3::Reader& reader)
    {
        std::uint32_t local = (refNum.mIndex & 0xff000000) >> 24;

        // If we have an index value that does not make sense, assume that it was an addition
        // by the present plugin (but a faulty one)
        if (local && local <= reader.getParentFileIndices().size())
        {
            // If the most significant 8 bits are used, then this reference already exists.
            // In this case, do not spawn a new reference, but overwrite the old one.
            refNum.mIndex &= 0x00ffffff; // delete old plugin ID
            refNum.mContentFile = reader.getParentFileIndices()[local-1];
        }
        else
        {
            // This is an addition by the present plugin. Set the corresponding plugin index.
            refNum.mContentFile = reader.getModIndex();
        }
    }
}

namespace ESM3
{
    unsigned int Cell::sRecordId = REC_CELL;

    // Some overloaded compare operators.
    bool operator== (const MovedCellRef& ref, const RefNum& refNum)
    {
        return ref.mRefNum == refNum;
    }

    bool operator== (const CellRef& ref, const RefNum& refNum)
    {
        return ref.mRefNum == refNum;
    }

    void Cell::load(Reader& reader, bool& isDeleted, bool saveContext)
    {
        loadNameAndData(reader, isDeleted);
        loadCell(reader, saveContext);
    }

    void Cell::loadNameAndData(Reader& reader, bool& isDeleted)
    {
        isDeleted = false;

        blank();

        bool hasData = false;
        bool isLoaded = false;
        while (!isLoaded && reader.getNextSubRecordType() != 0)
        {
            const ESM3::SubRecordHeader& subHdr = reader.subRecordHeader();
            switch (subHdr.typeId)
            {
                case ESM3::SUB_NAME:
                    reader.getSubRecordHeader();
                    reader.getZString(mName);
                    break;
                case ESM3::SUB_DATA:
                {
                    reader.getSubRecordHeader();
                    if (subHdr.dataSize != sizeof(mData) || subHdr.dataSize != 12)
                        reader.fail("CELL incorrect data size");
                    reader.get(mData);
                    hasData = true;
                    break;
                }
                case ESM3::SUB_DELE:
                {
                    reader.getSubRecordHeader();
                    reader.skipSubRecordData();
                    isDeleted = true;
                    break;
                }
                default:
                    isLoaded = true;
                    break;
            }
        }

        if (!hasData)
            reader.fail("Missing DATA subrecord");

        mCellId.mPaged = !(mData.mFlags & Interior);

        if (mCellId.mPaged)
        {
            mCellId.mWorldspace = ESM3::CellId::sDefaultWorldspace;
            mCellId.mIndex.mX = mData.mX;
            mCellId.mIndex.mY = mData.mY;
        }
        else
        {
            mCellId.mWorldspace = Misc::StringUtils::lowerCase (mName);
            mCellId.mIndex.mX = 0;
            mCellId.mIndex.mY = 0;
        }
    }

    void Cell::loadCell(Reader& reader, bool saveContext)
    {
        bool overriding = !mName.empty();
        bool isLoaded = false;
        mHasAmbi = false;
        while (!isLoaded && reader.getSubRecordHeader())
        {
            const ESM3::SubRecordHeader& subHdr = reader.subRecordHeader();
            switch (subHdr.typeId)
            {
                case ESM3::SUB_INTV:
                {
                    int32_t waterl;
                    reader.get(waterl);
                    mWater = static_cast<float>(waterl);
                    mWaterInt = true;
                    break;
                }
                case ESM3::SUB_WHGT:
                {
                    float waterLevel;
                    reader.get(waterLevel);
                    mWaterInt = false;
                    if(!std::isfinite(waterLevel))
                    {
                        if(!overriding)
                            mWater = std::numeric_limits<float>::max();
                        Log(Debug::Warning)
                            << "Warning: Encountered invalid water level in cell "
                            << mName << " defined in " << reader.getFileName();
                    }
                    else
                        mWater = waterLevel;
                    break;
                }
                case ESM3::SUB_AMBI:
                {
                    if (subHdr.dataSize != sizeof(mAmbi) || subHdr.dataSize != 16)
                        reader.fail("AMBI incorrect data size");
                    reader.get(mAmbi);
                    mHasAmbi = true;
                    break;
                }
                case ESM3::SUB_RGNN: reader.getZString(mRegion); break;
                case ESM3::SUB_NAM5: reader.get(mMapColor); break;
                case ESM3::SUB_NAM0: reader.get(mRefNumCounter); break;
                default:
                    reader.cacheSubRecordHeader(); // urgh
                    isLoaded = true;
                    break;
            }
        }

        if (saveContext)
        {
            mContextList.push_back(reader.getContext());
            reader.skipRecordData();
        }
        // FIXME: below is for testing only
        //else if (reader.hasMoreSubs() && reader.subRecordHeader().typeId != ESM3::SUB_FRMR)
            //std::cout << "sub-rec " << ESM::printName(reader.subRecordHeader().typeId) << std::endl;
    }

    // called from MWWorld::Store<ESM3::Cell>::load()
    void Cell::postLoad(Reader& reader)
    {
        // Save position of the cell references and move on
        mContextList.push_back(reader.getContext());
        reader.skipRecordData();
    }

    void Cell::save(ESM::ESMWriter& esm, bool isDeleted) const
    {
        esm.writeHNCString("NAME", mName);
        esm.writeHNT("DATA", mData, 12);

        if (isDeleted)
        {
            esm.writeHNString("DELE", "", 3);
            return;
        }

        if (mData.mFlags & Interior)
        {
            if (mWaterInt) {
                int water =
                    (mWater >= 0) ? (int) (mWater + 0.5) : (int) (mWater - 0.5);
                esm.writeHNT("INTV", water);
            } else {
                esm.writeHNT("WHGT", mWater);
            }

            if (mData.mFlags & QuasiEx)
                esm.writeHNOCString("RGNN", mRegion);
            else
            {
                // Try to avoid saving ambient lighting information when it's unnecessary.
                // This is to fix black lighting in resaved cell records that lack this information.
                if (mHasAmbi)
                    esm.writeHNT("AMBI", mAmbi, 16);
            }
        }
        else
        {
            esm.writeHNOCString("RGNN", mRegion);
            if (mMapColor != 0)
                esm.writeHNT("NAM5", mMapColor);
        }
    }

    void Cell::saveTempMarker(ESM::ESMWriter& esm, int tempCount) const
    {
        if (tempCount != 0)
            esm.writeHNT("NAM0", tempCount);
    }

    void Cell::restore(Reader& reader, int iCtx) const
    {
        reader.restoreContext(mContextList.at (iCtx));
    }

    std::string Cell::getDescription() const
    {
        if (mData.mFlags & Interior)
            return mName;

        std::string cellGrid = "(" + std::to_string(mData.mX) + ", " + std::to_string(mData.mY) + ")";
        if (!mName.empty())
            return mName + ' ' + cellGrid;
        // FIXME: should use sDefaultCellname GMST instead, but it's not available in this scope
        std::string region = !mRegion.empty() ? mRegion : "Wilderness";

        return region + ' ' + cellGrid;
    }

    bool Cell::getNextRef(Reader& reader, CellRef& cellRef, bool& isDeleted)
    {
        isDeleted = false;

        // TODO: Try and document reference numbering, I don't think this has been done anywhere else.
        if (!reader.hasMoreSubs())
            return false;

        // MVRF are FRMR are present in pairs. MVRF indicates that following FRMR describes moved CellRef.
        // This function has to skip all moved CellRefs therefore read all such pairs to ignored values.
        while (reader.getSubRecordHeader())
        {
            const ESM3::SubRecordHeader& subHdr = reader.subRecordHeader();

            bool skipDeleted = false;
            switch (subHdr.typeId)
            {
                case ESM3::SUB_MVRF:
                {
                    MovedCellRef movedCellRef;
                    reader.get(movedCellRef.mRefNum.mIndex);
                    // assumed that "CNDT" follows "MVRF"
                    reader.getSubRecordHeader(ESM3::SUB_CNDT);
                    reader.get(movedCellRef.mTarget);
                    skipDeleted = true;

                    if (reader.getNextSubRecordHeader(ESM3::SUB_CNDT))
                        reader.get(movedCellRef.mTarget);
                    CellRef skippedCellRef;
                    if (reader.getNextSubRecordType() != ESM3::SUB_FRMR)
                        return false;
                    bool skippedDeleted;
                    skippedCellRef.load(reader, skippedDeleted);
                    break;
                }
                case ESM3::SUB_FRMR:
                {
                    if (skipDeleted)
                    {
                        reader.skipSubRecordData();
                        skipDeleted = false;
                        break;
                    }

                    cellRef.load(reader, isDeleted);

                    // TODO: should count the number of temp refs and validate the number

                    // Identify references belonging to a parent file and adapt the ID accordingly.
                    adjustRefNum(cellRef.mRefNum, reader);
                    return true;
                }
                default:
                    std::cout << "unexpected sub-rec " << ESM::printName(subHdr.typeId) << std::endl;
                    //return false;
            }
        }

        return false;
    }

    bool Cell::getNextRef(Reader& reader, CellRef& cellRef, bool& isDeleted, MovedCellRef& movedCellRef, bool& moved)
    {
        isDeleted = false;
        moved = false;

        if (!reader.hasMoreSubs())
            return false;

        while (reader.getSubRecordHeader())
        {
            const ESM3::SubRecordHeader& subHdr = reader.subRecordHeader();
            switch (subHdr.typeId)
            {
                case ESM3::SUB_MVRF:
                {
                    moved = true;
                    getNextMVRF(reader, movedCellRef);
                    break;
                }
                case ESM3::SUB_FRMR:
                {
                    cellRef.load(reader, isDeleted);
                    adjustRefNum(cellRef.mRefNum, reader);

                    return true;
                }
                default:
                    std::cout << "unexpected sub-rec " << ESM::printName(subHdr.typeId) << std::endl;
                    //return false;
            }
        }

        return false;
    }

    // NOTE: assumes the sub-record header was just read
    bool Cell::getNextMVRF(Reader& reader, MovedCellRef &mref)
    {
        reader.get(mref.mRefNum.mIndex);
        // assumed that "CNDT" follows "MVRF"
        reader.getSubRecordHeader(ESM3::SUB_CNDT);
        reader.get(mref.mTarget);

        adjustRefNum (mref.mRefNum, reader);

        return true;
    }

    void Cell::blank()
    {
        mName.clear();
        mRegion.clear();
        mWater = 0;
        mWaterInt = false;
        mMapColor = 0;
        mRefNumCounter = -1;

        mData.mFlags = 0;
        mData.mX = 0;
        mData.mY = 0;

        mHasAmbi = true;
        mAmbi.mAmbient = 0;
        mAmbi.mSunlight = 0;
        mAmbi.mFog = 0;
        mAmbi.mFogDensity = 0;
    }

    const CellId& Cell::getCellId() const
    {
        return mCellId;
    }
}
