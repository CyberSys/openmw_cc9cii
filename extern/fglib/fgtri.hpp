/*
  Copyright (C) 2019, 2020 cc9cii

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

*/
#ifndef FGLIB_FGTRI_H
#define FGLIB_FGTRI_H

#include <string>
#include <cstdint>
#include <vector>

#include <boost/scoped_array.hpp>

namespace Ogre
{
    class Vector3;
}

namespace FgLib
{
    class FgTri
    {
        std::uint64_t mFileType;
        std::uint32_t mNumVertices;
        std::uint32_t mNumTriangles;
        std::uint32_t mNumQuads;
        std::uint32_t mNumLabelledVertices;
        std::uint32_t mNumLabelledSurfacePoints;
        std::uint32_t mNumTextureCoordinates;
        std::uint32_t mExtensionInfo;
        std::uint32_t mNumLabelledDiffMorphs;
        std::uint32_t mNumLabelledStatMorphs;
        std::uint32_t mNumTotalStatMorphVertices;

        boost::scoped_array<float> mVertices;
        boost::scoped_array<std::int32_t> mTriangleIndicies;
        boost::scoped_array<std::int32_t> mQuadIndicies;

        bool mNeedsNifVertices;

        // default, copy and assignment not allowed
        FgTri();
        FgTri(const FgTri& other);
        FgTri& operator=(const FgTri& other);

    public:
        // The parameter 'name' refers to those in the Ogre::ResourceGroupManager
        // e.g. full path from the directory/BSA added in Bsa::registerResources(), etc
        FgTri(const std::string& name);
        FgTri(const std::vector<Ogre::Vector3>& nifVerts); // for creating a dummy
        ~FgTri();

        inline const std::uint32_t numVertices() const { return mNumVertices; }
        inline const std::uint32_t numMorphVertices() const { return mNumTotalStatMorphVertices; }

        inline const boost::scoped_array<float>& vertices() const { return mVertices; }

        inline const bool needsNifVertices() const { return mNeedsNifVertices; }
    };
}

#endif // FGLIB_FGTRI_H
