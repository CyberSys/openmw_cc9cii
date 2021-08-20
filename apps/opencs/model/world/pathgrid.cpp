#include "cell.hpp"
#include "idcollection.hpp"
#include "pathgrid.hpp"

#include <sstream>

void CSMWorld::Pathgrid::load (ESM::Reader& reader, bool& isDeleted, const IdCollection<Cell>& cells)
{
    load (static_cast<ESM3::Reader&>(reader), isDeleted);

    // correct ID
    if (!mId.empty() && mId[0]!='#' && cells.searchId (mId)==-1)
    {
        std::ostringstream stream;
        stream << "#" << mData.mX << " " << mData.mY;
        mId = stream.str();
    }
}

void CSMWorld::Pathgrid::load (ESM::Reader& reader, bool& isDeleted)
{
    ESM3::Pathgrid::load (static_cast<ESM3::Reader&>(reader), isDeleted);

    mId = mCell;
    if (mCell.empty())
    {
        std::ostringstream stream;
        stream << "#" << mData.mX << " " << mData.mY;
        mId = stream.str();
    }
}
