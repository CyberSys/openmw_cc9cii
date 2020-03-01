/*
  Copyright (C) 2015-2020 cc9cii

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  cc9cii cc9c@iinet.net.au

  Much of the information on the NIF file structures are based on the NifSkope
  documenation.  See http://niftools.sourceforge.net/wiki/NifSkope for details.

*/
#ifndef NIBTOGRE_NIFLIPCONTROLLER_H
#define NIBTOGRE_NIFLIPCONTROLLER_H

#include <string>
#include <vector>
#include <cstdint>

#include "nitimecontroller.hpp"
#include "niobject.hpp"

// Based on NifTools/NifSkope/doc/index.html
//
// NiTimeController
//     NiInterpController <------------------------------- /* NiTimeController */
//         NiSingleInterpController
//             NiFloatInterpController <------------------ /* not implemented */
//                 NiFlipController
namespace NiBtOgre
{
    class NiObjectNET;

    class NiFlipController : public NiSingleInterpController
    {
    public:
        std::int32_t mTexureSlot;
        float mDelta;
        std::vector<NiSourceTextureRef> mSources;

        NiFlipController(uint32_t index, NiStream *stream, const NiModel& model, BuildData& data);
    };
}

#endif // NIBTOGRE_NIFLIPCONTROLLER_H
