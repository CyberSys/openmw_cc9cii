#ifndef CSM_TOOLS_TOPICINFOCHECK_HPP
#define CSM_TOOLS_TOPICINFOCHECK_HPP

#include <set>

#include <components/esm3/clas.hpp>
#include <components/esm3/dial.hpp>
#include <components/esm3/fact.hpp>
#include <components/esm3/glob.hpp>
#include <components/esm3/gmst.hpp>
#include <components/esm3/race.hpp>
#include <components/esm3/regn.hpp>

#include "../world/cell.hpp"
#include "../world/idcollection.hpp"
#include "../world/infocollection.hpp"
#include "../world/refiddata.hpp"
#include "../world/resources.hpp"

#include "../doc/stage.hpp"

namespace CSMTools
{
    /// \brief VerifyStage: check topics
    class TopicInfoCheckStage : public CSMDoc::Stage
    {
    public:

        TopicInfoCheckStage(
            const CSMWorld::InfoCollection& topicInfos,
            const CSMWorld::IdCollection<CSMWorld::Cell>& cells,
            const CSMWorld::IdCollection<ESM3::Class>& classes,
            const CSMWorld::IdCollection<ESM3::Faction>& factions,
            const CSMWorld::IdCollection<ESM3::GameSetting>& gmsts,
            const CSMWorld::IdCollection<ESM3::Global>& globals,
            const CSMWorld::IdCollection<ESM3::Dialogue>& journals,
            const CSMWorld::IdCollection<ESM3::Race>& races,
            const CSMWorld::IdCollection<ESM3::Region>& regions,
            const CSMWorld::IdCollection<ESM3::Dialogue>& topics,
            const CSMWorld::RefIdData& referencables,
            const CSMWorld::Resources& soundFiles);

        int setup() override;
        ///< \return number of steps

        void perform(int step, CSMDoc::Messages& messages) override;
        ///< Messages resulting from this stage will be appended to \a messages

    private:

        const CSMWorld::InfoCollection& mTopicInfos;

        const CSMWorld::IdCollection<CSMWorld::Cell>& mCells;
        const CSMWorld::IdCollection<ESM3::Class>& mClasses;
        const CSMWorld::IdCollection<ESM3::Faction>& mFactions;
        const CSMWorld::IdCollection<ESM3::GameSetting>& mGameSettings;
        const CSMWorld::IdCollection<ESM3::Global>& mGlobals;
        const CSMWorld::IdCollection<ESM3::Dialogue>& mJournals;
        const CSMWorld::IdCollection<ESM3::Race>& mRaces;
        const CSMWorld::IdCollection<ESM3::Region>& mRegions;
        const CSMWorld::IdCollection<ESM3::Dialogue>& mTopics;

        const CSMWorld::RefIdData& mReferencables;
        const CSMWorld::Resources& mSoundFiles;

        std::set<std::string> mCellNames;

        bool mIgnoreBaseRecords;

        // These return false when not successful and write an error
        bool verifyActor(const std::string& name, const CSMWorld::UniversalId& id, CSMDoc::Messages& messages);
        bool verifyCell(const std::string& name, const CSMWorld::UniversalId& id, CSMDoc::Messages& messages);
        bool verifyFactionRank(const std::string& name, int rank, const CSMWorld::UniversalId& id,
            CSMDoc::Messages& messages);
        bool verifyItem(const std::string& name, const CSMWorld::UniversalId& id, CSMDoc::Messages& messages);
        bool verifySelectStruct(const ESM3::DialInfo::SelectStruct& select, const CSMWorld::UniversalId& id,
            CSMDoc::Messages& messages);
        bool verifySound(const std::string& name, const CSMWorld::UniversalId& id, CSMDoc::Messages& messages);

        template <typename T>
        bool verifyId(const std::string& name, const CSMWorld::IdCollection<T>& collection,
            const CSMWorld::UniversalId& id, CSMDoc::Messages& messages);
    };
}

#endif
