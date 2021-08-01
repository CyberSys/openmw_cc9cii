#ifndef CSM_WOLRD_DATA_H
#define CSM_WOLRD_DATA_H

#include <map>
#include <vector>

#include <boost/filesystem/path.hpp>

#include <QObject>
#include <QModelIndex>

#include <components/esm3/glob.hpp>
#include <components/esm3/gmst.hpp>
#include <components/esm3/skil.hpp>
#include <components/esm3/clas.hpp>
#include <components/esm3/fact.hpp>
#include <components/esm3/race.hpp>
#include <components/esm3/soun.hpp>
#include <components/esm3/scpt.hpp>
#include <components/esm3/sscr.hpp>
#include <components/esm3/regn.hpp>
#include <components/esm3/bsgn.hpp>
#include <components/esm3/spel.hpp>
#include <components/esm3/dial.hpp>
#include <components/esm3/ench.hpp>
#include <components/esm3/body.hpp>
#include <components/esm3/sndg.hpp>
#include <components/esm3/mgef.hpp>
#include <components/esm3/sscr.hpp>
#include <components/esm3/debugprofile.hpp>
#include <components/esm3/filter.hpp>

#include <components/resource/resourcesystem.hpp>

#include <components/files/multidircollection.hpp>
#include <components/to_utf8/to_utf8.hpp>

#include "../doc/stage.hpp"

#include "actoradapter.hpp"
#include "idcollection.hpp"
#include "nestedidcollection.hpp"
#include "universalid.hpp"
#include "cell.hpp"
#include "land.hpp"
#include "landtexture.hpp"
#include "refidcollection.hpp"
#include "refcollection.hpp"
#include "infocollection.hpp"
#include "nestedinfocollection.hpp"
#include "pathgrid.hpp"
#include "resourcesmanager.hpp"
#include "metadata.hpp"
#ifndef Q_MOC_RUN
#include "subcellcollection.hpp"
#endif

class QAbstractItemModel;

namespace VFS
{
    class Manager;
}

namespace Fallback
{
    class Map;
}

namespace ESM3
{
    class Reader;
    struct Dialogue;
}

namespace ESM4
{
    union RecordHeader;
}

namespace CSMWorld
{
    class ResourcesManager;
    class Resources;

    class Data : public QObject
    {
            Q_OBJECT

            ToUTF8::Utf8Encoder mEncoder;
            IdCollection<ESM3::Global> mGlobals;
            IdCollection<ESM3::GameSetting> mGmsts;
            IdCollection<ESM3::Skill> mSkills;
            IdCollection<ESM3::Class> mClasses;
            NestedIdCollection<ESM3::Faction> mFactions;
            NestedIdCollection<ESM3::Race> mRaces;
            IdCollection<ESM3::Sound> mSounds;
            IdCollection<ESM3::Script> mScripts;
            NestedIdCollection<ESM3::Region> mRegions;
            NestedIdCollection<ESM3::BirthSign> mBirthsigns;
            NestedIdCollection<ESM3::Spell> mSpells;
            IdCollection<ESM3::Dialogue> mTopics;
            IdCollection<ESM3::Dialogue> mJournals;
            NestedIdCollection<ESM3::Enchantment> mEnchantments;
            IdCollection<ESM3::BodyPart> mBodyParts;
            IdCollection<ESM3::MagicEffect> mMagicEffects;
            SubCellCollection<Pathgrid> mPathgrids;
            IdCollection<ESM3::DebugProfile> mDebugProfiles;
            IdCollection<ESM3::SoundGenerator> mSoundGens;
            IdCollection<ESM3::StartScript> mStartScripts;
            NestedInfoCollection mTopicInfos;
            InfoCollection mJournalInfos;
            NestedIdCollection<Cell> mCells;
            IdCollection<LandTexture> mLandTextures;
            IdCollection<Land> mLand;
            RefIdCollection mReferenceables;
            RefCollection mRefs;
            IdCollection<ESM3::Filter> mFilters;
            Collection<MetaData> mMetaData;
            std::unique_ptr<ActorAdapter> mActorAdapter;
            std::vector<QAbstractItemModel *> mModels;
            std::map<UniversalId::Type, QAbstractItemModel *> mModelIndex;
            ESM::Reader* mReader;
            const ESM3::Dialogue *mDialogue; // last loaded dialogue
            bool mBase;
            bool mProject;
            std::map<std::string, std::map<unsigned int, unsigned int> > mRefLoadCache;
            int mReaderIndex;
            std::vector<std::string> mLoadedFiles; // FIXME: probably duplicated data

            bool mFsStrict;
            Files::PathContainer mDataPaths;
            std::vector<std::string> mArchives;
            std::unique_ptr<VFS::Manager> mVFS;
            ResourcesManager mResourcesManager;
            std::shared_ptr<Resource::ResourceSystem> mResourceSystem;

            std::vector<std::shared_ptr<ESM::Reader> > mReaders;
            // FIXME: hack to workaround the differences between OpenCS and OpenMW
            //        without rewriting the code block copied from OpenMW
            //std::vector<ESM::Reader*> mReaderList;

            std::map<std::string, int> mContentFileNames;

            // not implemented
            Data (const Data&);
            Data& operator= (const Data&);

            void addModel (QAbstractItemModel *model, UniversalId::Type type,
                bool update = true);

            static void appendIds (std::vector<std::string>& ids, const CollectionBase& collection,
                bool listDeleted);
            ///< Append all IDs from collection to \a ids.

            static int count (RecordBase::State state, const CollectionBase& collection);

            void loadFallbackEntries();

            bool loadTes4Group (CSMDoc::Messages& messages);
            bool loadTes4Record (const ESM4::RecordHeader& hdr, CSMDoc::Messages& messages);

        public:

            Data (ToUTF8::FromType encoding, bool fsStrict, const Files::PathContainer& dataPaths,
                const std::vector<std::string>& archives, const boost::filesystem::path& resDir);

            ~Data() override;

            const VFS::Manager* getVFS() const;

            std::shared_ptr<Resource::ResourceSystem> getResourceSystem();

            std::shared_ptr<const Resource::ResourceSystem> getResourceSystem() const;

            const IdCollection<ESM3::Global>& getGlobals() const;

            IdCollection<ESM3::Global>& getGlobals();

            const IdCollection<ESM3::GameSetting>& getGmsts() const;

            IdCollection<ESM3::GameSetting>& getGmsts();

            const IdCollection<ESM3::Skill>& getSkills() const;

            IdCollection<ESM3::Skill>& getSkills();

            const IdCollection<ESM3::Class>& getClasses() const;

            IdCollection<ESM3::Class>& getClasses();

            const IdCollection<ESM3::Faction>& getFactions() const;

            IdCollection<ESM3::Faction>& getFactions();

            const IdCollection<ESM3::Race>& getRaces() const;

            IdCollection<ESM3::Race>& getRaces();

            const IdCollection<ESM3::Sound>& getSounds() const;

            IdCollection<ESM3::Sound>& getSounds();

            const IdCollection<ESM3::Script>& getScripts() const;

            IdCollection<ESM3::Script>& getScripts();

            const IdCollection<ESM3::Region>& getRegions() const;

            IdCollection<ESM3::Region>& getRegions();

            const IdCollection<ESM3::BirthSign>& getBirthsigns() const;

            IdCollection<ESM3::BirthSign>& getBirthsigns();

            const IdCollection<ESM3::Spell>& getSpells() const;

            IdCollection<ESM3::Spell>& getSpells();

            const IdCollection<ESM3::Dialogue>& getTopics() const;

            IdCollection<ESM3::Dialogue>& getTopics();

            const IdCollection<ESM3::Dialogue>& getJournals() const;

            IdCollection<ESM3::Dialogue>& getJournals();

            const InfoCollection& getTopicInfos() const;

            InfoCollection& getTopicInfos();

            const InfoCollection& getJournalInfos() const;

            InfoCollection& getJournalInfos();

            const IdCollection<Cell>& getCells() const;

            IdCollection<Cell>& getCells();

            const RefIdCollection& getReferenceables() const;

            RefIdCollection& getReferenceables();

            const RefCollection& getReferences() const;

            RefCollection& getReferences();

            const IdCollection<ESM3::Filter>& getFilters() const;

            IdCollection<ESM3::Filter>& getFilters();

            const IdCollection<ESM3::Enchantment>& getEnchantments() const;

            IdCollection<ESM3::Enchantment>& getEnchantments();

            const IdCollection<ESM3::BodyPart>& getBodyParts() const;

            IdCollection<ESM3::BodyPart>& getBodyParts();

            const IdCollection<ESM3::DebugProfile>& getDebugProfiles() const;

            IdCollection<ESM3::DebugProfile>& getDebugProfiles();

            const IdCollection<CSMWorld::Land>& getLand() const;

            IdCollection<CSMWorld::Land>& getLand();

            const IdCollection<CSMWorld::LandTexture>& getLandTextures() const;

            IdCollection<CSMWorld::LandTexture>& getLandTextures();

            const IdCollection<ESM3::SoundGenerator>& getSoundGens() const;

            IdCollection<ESM3::SoundGenerator>& getSoundGens();

            const IdCollection<ESM3::MagicEffect>& getMagicEffects() const;

            IdCollection<ESM3::MagicEffect>& getMagicEffects();

            const SubCellCollection<Pathgrid>& getPathgrids() const;

            SubCellCollection<Pathgrid>& getPathgrids();

            const IdCollection<ESM3::StartScript>& getStartScripts() const;

            IdCollection<ESM3::StartScript>& getStartScripts();

            /// Throws an exception, if \a id does not match a resources list.
            const Resources& getResources (const UniversalId& id) const;

            const MetaData& getMetaData() const;

            void setMetaData (const MetaData& metaData);

            QAbstractItemModel *getTableModel (const UniversalId& id);
            ///< If no table model is available for \a id, an exception is thrown.
            ///
            /// \note The returned table may either be the model for the ID itself or the model that
            /// contains the record specified by the ID.

            const ActorAdapter* getActorAdapter() const;

            ActorAdapter* getActorAdapter();

            void merge();
            ///< Merge modified into base.

            int getTotalRecords (const std::vector<boost::filesystem::path>& files); // for better loading bar

            int startLoading (const boost::filesystem::path& path, bool base, bool project);
            ///< Begin merging content of a file into base or modified.
            ///
            /// \param project load project file instead of content file
            ///
            ///< \return estimated number of records

            bool continueLoading (CSMDoc::Messages& messages);
            ///< \return Finished?

            bool hasId (const std::string& id) const;

            std::vector<std::string> getIds (bool listDeleted = true) const;
            ///< Return a sorted collection of all IDs that are not internal to the editor.
            ///
            /// \param listDeleted include deleted record in the list

            int count (RecordBase::State state) const;
            ///< Return number of top-level records with the given \a state.

        signals:

            void idListChanged();

            void assetTablesChanged();

        private slots:

            void assetsChanged();

            void dataChanged (const QModelIndex& topLeft, const QModelIndex& bottomRight);

            void rowsChanged (const QModelIndex& parent, int start, int end);
    };
}

#endif
