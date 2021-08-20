
#include "mergeoperation.hpp"

#include "../doc/state.hpp"
#include "../doc/document.hpp"

#include "mergestages.hpp"

CSMTools::MergeOperation::MergeOperation (CSMDoc::Document& document, ToUTF8::FromType encoding)
: CSMDoc::Operation (CSMDoc::State_Merging, true), mState (document)
{
    appendStage (new StartMergeStage (mState));

    appendStage (new MergeIdCollectionStage<ESM3::Global> (mState, &CSMWorld::Data::getGlobals));
    appendStage (new MergeIdCollectionStage<ESM3::GameSetting> (mState, &CSMWorld::Data::getGmsts));
    appendStage (new MergeIdCollectionStage<ESM3::Skill> (mState, &CSMWorld::Data::getSkills));
    appendStage (new MergeIdCollectionStage<ESM3::Class> (mState, &CSMWorld::Data::getClasses));
    appendStage (new MergeIdCollectionStage<ESM3::Faction> (mState, &CSMWorld::Data::getFactions));
    appendStage (new MergeIdCollectionStage<ESM3::Race> (mState, &CSMWorld::Data::getRaces));
    appendStage (new MergeIdCollectionStage<ESM3::Sound> (mState, &CSMWorld::Data::getSounds));
    appendStage (new MergeIdCollectionStage<ESM3::Script> (mState, &CSMWorld::Data::getScripts));
    appendStage (new MergeIdCollectionStage<ESM3::Region> (mState, &CSMWorld::Data::getRegions));
    appendStage (new MergeIdCollectionStage<ESM3::BirthSign> (mState, &CSMWorld::Data::getBirthsigns));
    appendStage (new MergeIdCollectionStage<ESM3::Spell> (mState, &CSMWorld::Data::getSpells));
    appendStage (new MergeIdCollectionStage<ESM3::Dialogue> (mState, &CSMWorld::Data::getTopics));
    appendStage (new MergeIdCollectionStage<ESM3::Dialogue> (mState, &CSMWorld::Data::getJournals));
    appendStage (new MergeIdCollectionStage<CSMWorld::Cell> (mState, &CSMWorld::Data::getCells));
    appendStage (new MergeIdCollectionStage<ESM3::Filter> (mState, &CSMWorld::Data::getFilters));
    appendStage (new MergeIdCollectionStage<ESM3::Enchantment> (mState, &CSMWorld::Data::getEnchantments));
    appendStage (new MergeIdCollectionStage<ESM3::BodyPart> (mState, &CSMWorld::Data::getBodyParts));
    appendStage (new MergeIdCollectionStage<ESM3::DebugProfile> (mState, &CSMWorld::Data::getDebugProfiles));
    appendStage (new MergeIdCollectionStage<ESM3::SoundGenerator> (mState, &CSMWorld::Data::getSoundGens));
    appendStage (new MergeIdCollectionStage<ESM3::MagicEffect> (mState, &CSMWorld::Data::getMagicEffects));
    appendStage (new MergeIdCollectionStage<ESM3::StartScript> (mState, &CSMWorld::Data::getStartScripts));
    appendStage (new MergeIdCollectionStage<CSMWorld::Pathgrid, CSMWorld::SubCellCollection<CSMWorld::Pathgrid> > (mState, &CSMWorld::Data::getPathgrids));
    appendStage (new MergeIdCollectionStage<CSMWorld::Info, CSMWorld::InfoCollection> (mState, &CSMWorld::Data::getTopicInfos));
    appendStage (new MergeIdCollectionStage<CSMWorld::Info, CSMWorld::InfoCollection> (mState, &CSMWorld::Data::getJournalInfos));
    appendStage (new MergeRefIdsStage (mState));
    appendStage (new MergeReferencesStage (mState));
    appendStage (new MergeReferencesStage (mState));
    appendStage (new PopulateLandTexturesMergeStage (mState));
    appendStage (new MergeLandStage (mState));
    appendStage (new FixLandsAndLandTexturesMergeStage (mState));
    appendStage (new CleanupLandTexturesMergeStage (mState));

    appendStage (new FinishMergedDocumentStage (mState, encoding));
}

void CSMTools::MergeOperation::setTarget (std::unique_ptr<CSMDoc::Document> document)
{
    mState.mTarget = std::move(document);
}

void CSMTools::MergeOperation::operationDone()
{
    CSMDoc::Operation::operationDone();

    if (mState.mCompleted)
        emit mergeDone (mState.mTarget.release());
}
