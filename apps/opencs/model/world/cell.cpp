#include "cell.hpp"

#include <sstream>

void CSMWorld::Cell::load (ESM::Reader& reader, bool& isDeleted)
{
    ESM3::Cell::load (static_cast<ESM3::Reader&>(reader), isDeleted, false);

    mId = mName;
    if (isExterior())
    {
        std::ostringstream stream;
        stream << "#" << mData.mX << " " << mData.mY;
        mId = stream.str();
    }
}
