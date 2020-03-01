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
#include "niheader.hpp"

#include <stdexcept>

#include "nistream.hpp"
#include "nidata.hpp"

std::string NiBtOgre::NiHeader::mEmptyString = "";

// See NifTools/NifSkope/doc/header.html
NiBtOgre::NiHeader::NiHeader(NiBtOgre::NiStream *stream) : mVer(0), mUserVer(0), mUserVer2(0), mNumBlocks(0)
{
    stream->setHeader(this);

    std::string header = stream->getLine();

    if (header.find("Gamebryo File Format") == std::string::npos &&
        header.find("NetImmerse File Format") == std::string::npos)
    {
        throw std::runtime_error("NiBtOgre::NiHeader::unsupported NIF file format");
    }

    // check supported file versions; TES4: 20.0.0.4, 20.0.0.5  TES5/FO3: 20.2.0.7  TES3: 4.0.0.2
    //
    // TES4 old versions:
    //
    //   ./creatures/boxtest/idle.kf:                     Gamebryo File Format, Version 10.2.0.0
    //   ./creatures/deer/idleanims/graze.kf:             Gamebryo File Format, Version 10.2.0.0
    //   ./creatures/deer/idleanims/lookleft.kf:          Gamebryo File Format, Version 10.2.0.0
    //   ./creatures/deer/idleanims/lookright.kf:         Gamebryo File Format, Version 10.2.0.0
    //   ./creatures/deer/idleanims/startle.kf:           Gamebryo File Format, Version 10.2.0.0
    //   ./creatures/dog/idleanims/specialidle_sandup.kf: Gamebryo File Format, Version 10.2.0.0
    //   ./creatures/endgame/battle.kf:                   Gamebryo File Format, Version 10.2.0.0
    //   ./creatures/endgame/entry.kf:                    Gamebryo File Format, Version 10.2.0.0
    //
    // In GOG version there are old versions:
    //   ./clutter/farm/oar01.nif                         NetImmerse File Format, Version 10.0.1.0
    //   etc
    //
    // This file exists but doesn't seem to be used in any of the ESM/ESP:
    //   ./creatures/minotaur/minotaurold.nif:            NetImmerse File Format, Version 10.0.1.2
    //
    // These are only used for OpenCS:
    //   ./marker_arrow.nif:                              NetImmerse File Format, Version 4.0.0.2
    //   ./marker_divine.nif:                             NetImmerse File Format, Version 4.0.0.2
    //   ./marker_temple.nif:                             NetImmerse File Format, Version 4.0.0.2
    //   ./marker_travel.nif:                             NetImmerse File Format, Version 4.0.0.2
    //
    //   $ find . -type f -print0 | xargs -0 strings -f | grep -E '3\.3\.0\.13'
    //   ./marker_radius.nif:                             NetImmerse File Format, Version 3.3.0.13
    //
    // FIXME: what about TES4 versions 10.0.1.0, 10.1.0.101 and 10.1.0.106 (although they don't
    //        seem to be used in the official TES4/TES5 meshes)
    //
    // TODO: check FONV/FO4 versions
    //
    stream->readNifVer(mVer);
    if (mVer != 0x14000004 && mVer != 0x14000005 && mVer != 0x14020007 && mVer != 0x04000002 &&
        mVer != 0x0a020000 && mVer != 0x0303000d && mVer != 0x0a000100 && mVer != 0x0a01006a/*&& mVer != 0x0a000102*/) // comment out unused
    {
        throw std::runtime_error("NiBtOgre::NiHeader::unsupported NIF file version " + std::to_string(mVer));
    }

    // verify the byte order we support
    if (mVer >= 0x14000004) // 20.0.0.4
    {
        char endian;

        stream->read(endian);
        if (!endian)
            throw std::runtime_error("NiBtOgre::NiHeader::unsupported byte order");
    }

    if (mVer >= 0x0a010000) // 10.1.0.0
    {
        stream->readUserVer(mUserVer);
    }

    stream->read(mNumBlocks);

    if (mVer >= 0x0a000100) // 5.0.0.1 but the oldest we support is 10.0.1.0
    {
        uint16_t numBlockTypes = 0;

        // WARN: this block needs updating if we ever end up supporting version 10.0.1.2
        //(
        // (Version == 20.2.0.7) ||
        // (Version == 20.0.0.5) ||
        // ((Version > 10.0.1.2) && (Version < 20.0.0.4) && (User Version < 11))
        //) && (User Version > 3)
        if ((mUserVer >= 10) || ((mUserVer == 1) && (mVer != 0x0a020000)))
        {
            stream->readUserVer2(mUserVer2);

            stream->readShortString(mCreator);
            stream->readShortString(mExportInfo1);
            stream->readShortString(mExportInfo2);
        }

        stream->read(numBlockTypes);

        mBlockTypes.resize(numBlockTypes);
        for (uint16_t i = 0; i < numBlockTypes; ++i)
            stream->readSizedString(mBlockTypes.at(i));

        mBlockTypeIndex.resize(mNumBlocks);
        for (uint32_t i = 0; i < mNumBlocks; ++i)
            stream->read(mBlockTypeIndex.at(i));
    }

    if (mVer >= 0x14020007) // 20.2.0.7
    {
        mBlockSize.resize(mNumBlocks);
        for (uint32_t i = 0; i < mNumBlocks; ++i)
            stream->read(mBlockSize.at(i));
    }

    if (mVer >= 0x14010003) // 20.1.0.3
    {
        uint32_t numStrings;
        uint32_t maxStringLength;

        stream->read(numStrings);
        stream->read(maxStringLength); // possibly useful for reading strings and setting bufer size?

        mStrings.resize(numStrings);
        for (uint32_t i = 0; i < numStrings; ++i)
            stream->readSizedString(mStrings.at(i));
    }

    if (mVer >= 0x0a000100) // 5.0.0.6 but the oldest we support is 10.0.1.0
    {
        uint32_t unknown;
        stream->read(unknown);
    }
}

// FIXME: should search for duplicates and return the corresponding index
//        (may not be worth the trouble since the frequency of duplicates may be low)
std::int32_t NiBtOgre::NiHeader::appendLongString(std::string&& str)
{
    if (str.empty())
        return -1;

    mStrings.push_back(std::move(str));
    return (std::uint32_t)mStrings.size()-1;
}

const std::string& NiBtOgre::NiHeader::blockType(std::uint32_t index) const
{
    // FIXME: should add a check for mBlockTypeIndex high bit
    return mBlockTypes[mBlockTypeIndex[index]];
}

//void NiBtOgre::NiHeader::getNiSkinInstances(std::vector<NiSkinInstance*>& skins,
//                                            std::vector<std::unique_ptr<NiObject> >& objects)
//{
//    std::vector<std::string>::const_iterator iter
//        = std::find(mBlockTypes.begin(), mBlockTypes.end(), "NiSkinInstance");
//
//    if (iter != mBlockTypes.end())
//    {
//        std::size_t skinIndex = iter - mBlockTypes.begin();
//        for (unsigned int i = 0; i < mNumBlocks; ++i)
//        {
//            if (mBlockTypeIndex[i] == skinIndex)
//                skins.push_back(static_cast<NiSkinInstance*>(objects[i].get()));
//        }
//    }
//}
