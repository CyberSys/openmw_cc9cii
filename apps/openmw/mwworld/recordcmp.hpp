#ifndef OPENMW_MWWORLD_RECORDCMP_H
#define OPENMW_MWWORLD_RECORDCMP_H

#include <components/esm3/records.hpp>

#include <components/misc/stringops.hpp>

namespace MWWorld
{
    struct RecordCmp
    {
        template <class T>
        bool operator()(const T &x, const T& y) const {
            return x.mId < y.mId;
        }
    };

    template <>
    inline bool RecordCmp::operator()<ESM3::Dialogue>(const ESM3::Dialogue &x, const ESM3::Dialogue &y) const {
        return Misc::StringUtils::ciLess(x.mId, y.mId);
    }

    template <>
    inline bool RecordCmp::operator()<ESM3::Cell>(const ESM3::Cell &x, const ESM3::Cell &y) const {
        return Misc::StringUtils::ciLess(x.mName, y.mName);
    }

    template <>
    inline bool RecordCmp::operator()<ESM3::Pathgrid>(const ESM3::Pathgrid &x, const ESM3::Pathgrid &y) const {
        return Misc::StringUtils::ciLess(x.mCell, y.mCell);
    }

} // end namespace
#endif
