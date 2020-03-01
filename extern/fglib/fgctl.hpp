/*
  Copyright (C) 2020 cc9cii

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
#ifndef FGLIB_FGCTL_H
#define FGLIB_FGCTL_H

#include <cstdint>

//#include <OgreResource.h>

#include "fgstream.hpp"

//namespace Ogre
//{
    //class ResourceManager;
    //class ManualResourceLoader;
//}

namespace FgLib
{
    class FgCtl //: public Ogre::Resource // FIXME: for quick testing
    {
        //FgStream mFgStream;

        //const std::string mGroup;  // Ogre group
        //const std::string mName;   // file name

    //public: // FIXME: should be private
        std::uint64_t mFileType;
        std::uint32_t mGeometryBasisVersion;
        std::uint32_t mTextureBasisVersion;
        std::uint32_t mNumSymMorphModes;    // should be 50
        std::uint32_t mNumAsymMorphModes;   // should be 30
        std::uint32_t mNumSymTextureModes;  // should be 50
        std::uint32_t mNumAsymTextureModes; // should be 0

        // populated by readLinearControls()
        std::uint32_t mNumGSLinearControls;
        std::uint32_t mNumGALinearControls;
        std::uint32_t mNumTSLinearControls;
        std::uint32_t mNumTALinearControls;

        // populated by readLinearControls()
        std::vector<std::pair<std::vector<float>, std::string> > mGSymLinearControls;
        std::vector<std::pair<std::vector<float>, std::string> > mGAsymLinearControls;
        std::vector<std::pair<std::vector<float>, std::string> > mTSymLinearControls;
        std::vector<std::pair<std::vector<float>, std::string> > mTAsymLinearControls;

        // populated by readOffsetLinearControls()
        std::vector<std::pair<std::vector<float>, float> > mAllPrtOffsetLinearControls;
        std::vector<std::pair<std::vector<float>, float> > mAfroPrtOffsetLinearControls;
        std::vector<std::pair<std::vector<float>, float> > mAsiaPrtOffsetLinearControls;
        std::vector<std::pair<std::vector<float>, float> > mEindPrtOffsetLinearControls;
        std::vector<std::pair<std::vector<float>, float> > mEuroPrtOffsetLinearControls;

        // FIXME: add rest of the data in si.ctl

    //private:
        // default, copy and assignment not allowed
        FgCtl();
        FgCtl(const FgCtl& other);
        FgCtl& operator=(const FgCtl& other);

    //protected: // FIXME: pure virtual methods of Ogre::Resource
    //public:
        //void loadImpl();
        //void unloadImpl() {}

    public:
        // The parameter 'name' refers to those in the Ogre::ResourceGroupManager
        // e.g. full path from the directory/BSA added in Bsa::registerResources(), etc
        //
        // NOTE: the constructor may throw
//      FgCtl(Ogre::ResourceManager *creator, const Ogre::String& name, Ogre::ResourceHandle handle,
//              const Ogre::String& group, bool isManual, Ogre::ManualResourceLoader* loader,
//              const Ogre::NameValuePairList* createParams=nullptr/*bool showEditorMarkers=false*/);
        FgCtl(const Ogre::String& name/*, const Ogre::String& group*/); // FIXME: for testing only
        ~FgCtl();

        //const std::string& getOgreGroup() const { return mGroup; }
        //const std::string& getName() const { return mName; }

        inline const std::vector<std::pair<std::vector<float>, std::string> >& symMorphCtl() const {
            return mGSymLinearControls;
        }

        inline const std::vector<std::pair<std::vector<float>, std::string> >& symTextureCtl() const {
            return mTSymLinearControls;
        }

        inline const std::vector<std::pair<std::vector<float>, float> >& allPrtOffsetCtl() const {
            return mAllPrtOffsetLinearControls;
        }

    private:
        void readLinearControls(uint32_t& numControls, uint32_t numModes,
                std::vector<std::pair<std::vector<float>, std::string> >& controls, FgStream& ctl);

        void readOffsetLinearControls(uint32_t numModes,
                std::vector<std::pair<std::vector<float>, float> >& controls, FgStream& ctl);
    };
}

#endif // FGLIB_FGCTL_H
