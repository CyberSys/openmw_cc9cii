#ifndef ENGINE_H
#define ENGINE_H

#include <string>

#include <boost/filesystem.hpp>

#include "apps/openmw/mwrender/mwscene.hpp"
#include "components/misc/tsdeque.hpp"
#include "components/commandserver/server.hpp"
#include "components/commandserver/command.hpp"


namespace MWRender
{
    class SkyManager;
}

namespace OMW
{
    /// \brief Main engine class, that brings together all the components of OpenMW

    class Engine
    {
            enum { kCommandServerPort = 27917 };

            boost::filesystem::path mDataDir;
            Render::OgreRenderer mOgre;
            std::string mCellName;
            std::string mMaster;
            
            bool                  mEnableSky;
            MWRender::SkyManager* mpSkyManager;

            TsDeque<OMW::Command>                     mCommandQueue;
            std::auto_ptr<OMW::CommandServer::Server> mspCommandServer;

            // not implemented
            Engine (const Engine&);
            Engine& operator= (const Engine&);

            /// add resources directory
            /// \note This function works recursively.
            void addResourcesDirectory (const boost::filesystem::path& path);

            /// Load all BSA files in data directory.
            void loadBSA();

        public:

            Engine();

            /// Set data dir
            void setDataDir (const boost::filesystem::path& dataDir);

            /// Set start cell name (only interiors for now)
            void setCell (const std::string& cellName);

            /// Set master file (esm)
            /// - If the given name does not have an extension, ".esm" is added automatically
            /// - Currently OpenMW only supports one master at the same time.
            void addMaster (const std::string& master);

            /// Enables rendering of the sky (off by default).
            void enableSky (bool bEnable);

            /// Process pending commands
            void processCommands();

            /// Initialise and enter main loop.
            void go();
    };
}

#endif
