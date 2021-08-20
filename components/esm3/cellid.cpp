#include "cellid.hpp"

#include "common.hpp"
#include "reader.hpp"
#include "../esm/esmwriter.hpp"

const std::string ESM3::CellId::sDefaultWorldspace = "sys::default";

void ESM3::CellId::load (Reader& reader)
{
    if (reader.getNextSubRecordHeader(ESM3::SUB_SPAC))
        reader.getString(mWorldspace); // NOTE: string not null terminated

    if (reader.getNextSubRecordHeader(ESM3::SUB_CIDX))
    {
        reader.get(mIndex, 8);
        mPaged = true;
    }
    else
        mPaged = false;
}

void ESM3::CellId::save (ESM::ESMWriter& esm) const
{
    esm.writeHNString ("SPAC", mWorldspace);

    if (mPaged)
        esm.writeHNT ("CIDX", mIndex, 8);
}

bool ESM3::operator== (const CellId& left, const CellId& right)
{
    return left.mWorldspace==right.mWorldspace && left.mPaged==right.mPaged &&
        (!left.mPaged || (left.mIndex.mX==right.mIndex.mX && left.mIndex.mY==right.mIndex.mY));
}

bool ESM3::operator!= (const CellId& left, const CellId& right)
{
    return !(left==right);
}

bool ESM3::operator < (const CellId& left, const CellId& right)
{
    if (left.mPaged < right.mPaged)
        return true;
    if (left.mPaged > right.mPaged)
        return false;

    if (left.mPaged)
    {
        if (left.mIndex.mX < right.mIndex.mX)
            return true;
        if (left.mIndex.mX > right.mIndex.mX)
            return false;

        if (left.mIndex.mY < right.mIndex.mY)
            return true;
        if (left.mIndex.mY > right.mIndex.mY)
            return false;
    }

    return left.mWorldspace < right.mWorldspace;
}
