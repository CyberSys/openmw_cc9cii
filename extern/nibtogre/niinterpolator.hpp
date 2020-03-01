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
#ifndef NIBTOGRE_NIINTERPOLATOR_H
#define NIBTOGRE_NIINTERPOLATOR_H

#include <string>
#include <cstdint>

#include <OgreVector3.h>
#include <OgreQuaternion.h>

#include "niobject.hpp"

// Based on NifTools/NifSkope/doc/index.html
//
// NiInterpolator <--------------------------------- /* NiObject */
//     NiBSplineInterpolator
//         NiBSplineFloatInterpolator <------------- /*NiBSplineInterpolator */
//         NiBSplinePoint3Interpolator
//         NiBSplineTransformInterpolator
//             NiBSplineCompTransformInterpolator
//     NiBlendInterpolator
//         NiBlendBoolInterpolator
//         NiBlendFloatInterpolator
//         NiBlendPoint3Interpolator
//         NiBlendTransformInterpolator <----------- /* NiBlendInterpolator */
//     NiKeyBasedInterpolator <--------------------- /* not implemented */
//         NiBoolInterpolator
//             NiBoolTimelineInterpolator <--------- /* NiBoolInterpolator */
//         NiFloatInterpolator
//         NiPathInterpolator
//         NiPoint3Interpolator
//         NiTransformInterpolator
//     NiLookAtInterpolator
namespace NiBtOgre
{
    class NiStream;
    class Header;
    template<class T> struct KeyGroup;

//  typedef NiObject NiInterpolator; // Seen in NIF version 20.0.0.4, 20.0.0.5
    struct NiInterpolator : public NiObject
    {
        NiInterpolator(uint32_t index, NiStream *stream, const NiModel& model, BuildData& data);

        virtual const KeyGroup<float> *getMorphKeyGroup();
    };

    // Seen in NIF version 20.2.0.7
    struct NiBSplineInterpolator : public NiInterpolator
    {
        float                 mStartTime;
        float                 mStopTime;
        NiBSplineDataRef      mSplineDataRef;
        NiBSplineBasisDataRef mBasisDataRef;

        NiBSplineInterpolator(uint32_t index, NiStream *stream, const NiModel& model, BuildData& data);
    };

    //typedef NiBSplineInterpolator NiBSplineFloatInterpolator; // Seen in NIF version 20.2.0.7
    struct NiBSplineFloatInterpolator : public NiBSplineInterpolator
    {
        float                 mValue;
        std::uint32_t         mHandle;

        NiBSplineFloatInterpolator(uint32_t index, NiStream *stream, const NiModel& model, BuildData& data);
    };

    // Seen in NIF version 20.2.0.7
    struct NiBSplinePoint3Interpolator : public NiBSplineInterpolator
    {
        float mUnknown1;
        float mUnknown2;
        float mUnknown3;
        float mUnknown4;
        float mUnknown5;
        float mUnknown6;

        NiBSplinePoint3Interpolator(uint32_t index, NiStream *stream, const NiModel& model, BuildData& data);
    };

    // Seen in NIF version 20.0.0.4, 20.0.0.5
    struct NiBSplineTransformInterpolator : public NiBSplineInterpolator
    {
        Ogre::Vector3 mTranslation;
        Ogre::Quaternion mRotation;
        float         mScale;
        std::uint32_t mTranslationHandle;
        std::uint32_t mRotationHandle;
        std::uint32_t mScaleHandle;

        NiBSplineTransformInterpolator(uint32_t index, NiStream *stream, const NiModel& model, BuildData& data);
    };

    // Seen in NIF version 20.0.0.4, 20.0.0.5
    struct NiBSplineCompTransformInterpolator : public NiBSplineTransformInterpolator
    {
        float mTranslationOffset;
        float mTranslationMultiplier;
        float mRotationOffset;
        float mRotationMultiplier;
        float mScaleOffset;
        float mScaleMultiplier;

        NiBSplineCompTransformInterpolator(uint32_t index, NiStream *stream, const NiModel& model, BuildData& data);
    };

    // Seen in NIF version 20.0.0.4, 20.0.0.5
    struct NiBSplineCompFloatInterpolator : public NiBSplineFloatInterpolator
    {
        float mFloatOffset;
        float mFloatHalfRange;

        NiBSplineCompFloatInterpolator(uint32_t index, NiStream *stream, const NiModel& model, BuildData& data);
    };

    struct NiBlendInterpolator : public NiInterpolator
    {
        std::uint16_t mUnknownShort;
        std::uint32_t mUnknownInt;

        NiBlendInterpolator(uint32_t index, NiStream *stream, const NiModel& model, BuildData& data);
    };

    // Seen in NIF version 20.0.0.4, 20.0.0.5
    struct NiBlendBoolInterpolator : public NiBlendInterpolator
    {
        unsigned char mValue;

        NiBlendBoolInterpolator(uint32_t index, NiStream *stream, const NiModel& model, BuildData& data);
    };

    // Seen in NIF version 20.0.0.4, 20.0.0.5
    struct NiBlendFloatInterpolator : public NiBlendInterpolator
    {
        float mValue;

        NiBlendFloatInterpolator(uint32_t index, NiStream *stream, const NiModel& model, BuildData& data);
    };

    // Seen in NIF version 20.0.0.4, 20.0.0.5
    struct NiBlendPoint3Interpolator : public NiBlendInterpolator
    {
        Ogre::Vector3 mValue;

        NiBlendPoint3Interpolator(uint32_t index, NiStream *stream, const NiModel& model, BuildData& data);
    };

    typedef NiBlendInterpolator NiBlendTransformInterpolator; // Seen in NIF version 20.0.0.4, 20.0.0.5

    // Seen in NIF version 20.0.0.4, 20.0.0.5
    struct NiBoolInterpolator : public NiInterpolator
    {
        bool          mBoolalue;
        NiBoolDataRef mDataRef;

        NiBoolInterpolator(uint32_t index, NiStream *stream, const NiModel& model, BuildData& data);
    };

    typedef NiBoolInterpolator NiBoolTimelineInterpolator; // Seen in NIF version 20.0.0.4, 20.0.0.5

    // Seen in NIF version 20.0.0.4, 20.0.0.5
    struct NiFloatInterpolator : public NiInterpolator
    {
        float          mFloatValue;
        NiFloatDataRef mDataRef;

        NiFloatInterpolator(uint32_t index, NiStream *stream, const NiModel& model, BuildData& data);

        const KeyGroup<float> *getMorphKeyGroup();
    };

    // Seen in NIF version 20.0.0.4, 20.0.0.5
    struct NiPathInterpolator : public NiInterpolator
    {
        NiPosDataRef   mPosDataRef;
        NiFloatDataRef mFloatDataRef;

        NiPathInterpolator(uint32_t index, NiStream *stream, const NiModel& model, BuildData& data);
    };

    // Seen in NIF version 20.0.0.4, 20.0.0.5
    struct NiPoint3Interpolator : public NiInterpolator
    {
        Ogre::Vector3 mPoint3Value;
        NiPosDataRef  mDataRef;

        NiPoint3Interpolator(uint32_t index, NiStream *stream, const NiModel& model, BuildData& data);
    };

    // Seen in NIF version 20.0.0.4, 20.0.0.5
    struct NiTransformInterpolator : public NiInterpolator
    {
        Ogre::Vector3      mTranslation;
        Ogre::Quaternion   mRotation;
        float              mScale;
        NiTransformDataRef mDataRef;

        NiTransformInterpolator(uint32_t index, NiStream *stream, const NiModel& model, BuildData& data);
    };

    class NiNode;

    // Seen in NIF version 20.2.0.7
    struct NiLookAtInterpolator : public NiInterpolator
    {
        std::uint16_t    mUnknown;
        // furniture\smeltermarker.nif (TES5 Whiterun) shows that some of the Ptr refer to
        // objects not yet loaded.  Change to Ref instead.
        //NiNode          *mLookAt; // Ptr
        NiNodeRef        mLookAt;
        StringIndex      mTarget;
        Ogre::Vector3    mTranslation;
        Ogre::Quaternion mRotation;
        float            mScale;
        NiPoint3InterpolatorRef mUnknownLink1;
        NiFloatInterpolatorRef  mUnknownLink2;
        NiFloatInterpolatorRef  mUnknownLink3;

        NiLookAtInterpolator(uint32_t index, NiStream *stream, const NiModel& model, BuildData& data);
    };
}

#endif // NIBTOGRE_NIINTERPOLATOR_H
