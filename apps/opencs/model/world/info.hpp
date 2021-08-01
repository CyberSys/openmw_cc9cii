#ifndef CSM_WOLRD_INFO_H
#define CSM_WOLRD_INFO_H

#include <components/esm3/info.hpp>

namespace CSMWorld
{
    struct Info : public ESM3::DialInfo
    {
        std::string mTopicId;
    };
}

#endif
