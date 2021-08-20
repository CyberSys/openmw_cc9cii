#include "data.hpp"

#include <stdexcept>
#include <algorithm>
#include <iostream> // FIXME

#include <QAbstractItemModel>

#include <components/esm/defs.hpp>
#include <components/esm3/glob.hpp>
#include <components/esm3/cellref.hpp>

#include <components/esm3/reader.hpp>
#include <components/esm4/reader.hpp>

#include <components/resource/scenemanager.hpp>
#include <components/sceneutil/shadow.hpp>
#include <components/shader/shadermanager.hpp>
#include <components/vfs/manager.hpp>
#include <components/vfs/registerarchives.hpp>

#include "idtable.hpp"
#include "idtree.hpp"
#include "columnimp.hpp"
#include "regionmap.hpp"
#include "columns.hpp"
#include "resourcesmanager.hpp"
#include "resourcetable.hpp"
#include "nestedcoladapterimp.hpp"

void CSMWorld::Data::addModel (QAbstractItemModel *model, UniversalId::Type type, bool update)
{
    mModels.push_back (model);
    mModelIndex.insert (std::make_pair (type, model));

    UniversalId::Type type2 = UniversalId::getParentType (type);

    if (type2!=UniversalId::Type_None)
        mModelIndex.insert (std::make_pair (type2, model));

    if (update)
    {
        connect (model, SIGNAL (dataChanged (const QModelIndex&, const QModelIndex&)),
            this, SLOT (dataChanged (const QModelIndex&, const QModelIndex&)));
        connect (model, SIGNAL (rowsInserted (const QModelIndex&, int, int)),
            this, SLOT (rowsChanged (const QModelIndex&, int, int)));
        connect (model, SIGNAL (rowsRemoved (const QModelIndex&, int, int)),
            this, SLOT (rowsChanged (const QModelIndex&, int, int)));
    }
}

void CSMWorld::Data::appendIds (std::vector<std::string>& ids, const CollectionBase& collection,
    bool listDeleted)
{
    std::vector<std::string> ids2 = collection.getIds (listDeleted);

    ids.insert (ids.end(), ids2.begin(), ids2.end());
}

int CSMWorld::Data::count (RecordBase::State state, const CollectionBase& collection)
{
    int number = 0;

    for (int i=0; i<collection.getSize(); ++i)
        if (collection.getRecord (i).mState==state)
            ++number;

    return number;
}

CSMWorld::Data::Data (ToUTF8::FromType encoding, bool fsStrict, const Files::PathContainer& dataPaths,
    const std::vector<std::string>& archives, const boost::filesystem::path& resDir)
: mEncoder (encoding), mPathgrids (mCells), mRefs (mCells),
  mReader (nullptr), mDialogue (nullptr), mReaderIndex(1),
  mFsStrict(fsStrict), mDataPaths(dataPaths), mArchives(archives)
{
    mVFS.reset(new VFS::Manager(mFsStrict));
    VFS::registerArchives(mVFS.get(), Files::Collections(mDataPaths, !mFsStrict), mArchives, true);

    mResourcesManager.setVFS(mVFS.get());
    mResourceSystem.reset(new Resource::ResourceSystem(mVFS.get()));

    Shader::ShaderManager::DefineMap defines = mResourceSystem->getSceneManager()->getShaderManager().getGlobalDefines();
    Shader::ShaderManager::DefineMap shadowDefines = SceneUtil::ShadowManager::getShadowsDisabledDefines();
    defines["forcePPL"] = "0"; // Don't force per-pixel lighting
    defines["clamp"] = "1"; // Clamp lighting
    defines["preLightEnv"] = "0"; // Apply environment maps after lighting like Morrowind
    defines["radialFog"] = "0";
    defines["lightingModel"] = "0";
    defines["reverseZ"] = "0";
    for (const auto& define : shadowDefines)
        defines[define.first] = define.second;
    mResourceSystem->getSceneManager()->getShaderManager().setGlobalDefines(defines);

    mResourceSystem->getSceneManager()->setShaderPath((resDir / "shaders").string());

    int index = 0;

    mGlobals.addColumn (new StringIdColumn<ESM3::Global>);
    mGlobals.addColumn (new RecordStateColumn<ESM3::Global>);
    mGlobals.addColumn (new FixedRecordTypeColumn<ESM3::Global> (UniversalId::Type_Global));
    mGlobals.addColumn (new VarTypeColumn<ESM3::Global> (ColumnBase::Display_GlobalVarType));
    mGlobals.addColumn (new VarValueColumn<ESM3::Global>);

    mGmsts.addColumn (new StringIdColumn<ESM3::GameSetting>);
    mGmsts.addColumn (new RecordStateColumn<ESM3::GameSetting>);
    mGmsts.addColumn (new FixedRecordTypeColumn<ESM3::GameSetting> (UniversalId::Type_Gmst));
    mGmsts.addColumn (new VarTypeColumn<ESM3::GameSetting> (ColumnBase::Display_GmstVarType));
    mGmsts.addColumn (new VarValueColumn<ESM3::GameSetting>);

    mSkills.addColumn (new StringIdColumn<ESM3::Skill>);
    mSkills.addColumn (new RecordStateColumn<ESM3::Skill>);
    mSkills.addColumn (new FixedRecordTypeColumn<ESM3::Skill> (UniversalId::Type_Skill));
    mSkills.addColumn (new AttributeColumn<ESM3::Skill>);
    mSkills.addColumn (new SpecialisationColumn<ESM3::Skill>);
    for (int i=0; i<4; ++i)
        mSkills.addColumn (new UseValueColumn<ESM3::Skill> (i));
    mSkills.addColumn (new DescriptionColumn<ESM3::Skill>);

    mClasses.addColumn (new StringIdColumn<ESM3::Class>);
    mClasses.addColumn (new RecordStateColumn<ESM3::Class>);
    mClasses.addColumn (new FixedRecordTypeColumn<ESM3::Class> (UniversalId::Type_Class));
    mClasses.addColumn (new NameColumn<ESM3::Class>);
    mClasses.addColumn (new AttributesColumn<ESM3::Class> (0));
    mClasses.addColumn (new AttributesColumn<ESM3::Class> (1));
    mClasses.addColumn (new SpecialisationColumn<ESM3::Class>);
    for (int i=0; i<5; ++i)
        mClasses.addColumn (new SkillsColumn<ESM3::Class> (i, true, true));
    for (int i=0; i<5; ++i)
        mClasses.addColumn (new SkillsColumn<ESM3::Class> (i, true, false));
    mClasses.addColumn (new PlayableColumn<ESM3::Class>);
    mClasses.addColumn (new DescriptionColumn<ESM3::Class>);

    mFactions.addColumn (new StringIdColumn<ESM3::Faction>);
    mFactions.addColumn (new RecordStateColumn<ESM3::Faction>);
    mFactions.addColumn (new FixedRecordTypeColumn<ESM3::Faction> (UniversalId::Type_Faction));
    mFactions.addColumn (new NameColumn<ESM3::Faction>(ColumnBase::Display_String32));
    mFactions.addColumn (new AttributesColumn<ESM3::Faction> (0));
    mFactions.addColumn (new AttributesColumn<ESM3::Faction> (1));
    mFactions.addColumn (new HiddenColumn<ESM3::Faction>);
    for (int i=0; i<7; ++i)
        mFactions.addColumn (new SkillsColumn<ESM3::Faction> (i));
    // Faction Reactions
    mFactions.addColumn (new NestedParentColumn<ESM3::Faction> (Columns::ColumnId_FactionReactions));
    index = mFactions.getColumns()-1;
    mFactions.addAdapter (std::make_pair(&mFactions.getColumn(index), new FactionReactionsAdapter ()));
    mFactions.getNestableColumn(index)->addColumn(
        new NestedChildColumn (Columns::ColumnId_Faction, ColumnBase::Display_Faction));
    mFactions.getNestableColumn(index)->addColumn(
        new NestedChildColumn (Columns::ColumnId_FactionReaction, ColumnBase::Display_Integer));

    // Faction Ranks
    mFactions.addColumn (new NestedParentColumn<ESM3::Faction> (Columns::ColumnId_FactionRanks));
    index = mFactions.getColumns()-1;
    mFactions.addAdapter (std::make_pair(&mFactions.getColumn(index), new FactionRanksAdapter ()));
    mFactions.getNestableColumn(index)->addColumn(
        new NestedChildColumn (Columns::ColumnId_RankName, ColumnBase::Display_Rank));
    mFactions.getNestableColumn(index)->addColumn(
        new NestedChildColumn (Columns::ColumnId_FactionAttrib1, ColumnBase::Display_Integer));
    mFactions.getNestableColumn(index)->addColumn(
        new NestedChildColumn (Columns::ColumnId_FactionAttrib2, ColumnBase::Display_Integer));
    mFactions.getNestableColumn(index)->addColumn(
        new NestedChildColumn (Columns::ColumnId_FactionPrimSkill, ColumnBase::Display_Integer));
    mFactions.getNestableColumn(index)->addColumn(
        new NestedChildColumn (Columns::ColumnId_FactionFavSkill, ColumnBase::Display_Integer));
    mFactions.getNestableColumn(index)->addColumn(
        new NestedChildColumn (Columns::ColumnId_FactionRep, ColumnBase::Display_Integer));

    mRaces.addColumn (new StringIdColumn<ESM3::Race>);
    mRaces.addColumn (new RecordStateColumn<ESM3::Race>);
    mRaces.addColumn (new FixedRecordTypeColumn<ESM3::Race> (UniversalId::Type_Race));
    mRaces.addColumn (new NameColumn<ESM3::Race>);
    mRaces.addColumn (new DescriptionColumn<ESM3::Race>);
    mRaces.addColumn (new FlagColumn<ESM3::Race> (Columns::ColumnId_Playable, 0x1));
    mRaces.addColumn (new FlagColumn<ESM3::Race> (Columns::ColumnId_BeastRace, 0x2));
    mRaces.addColumn (new WeightHeightColumn<ESM3::Race> (true, true));
    mRaces.addColumn (new WeightHeightColumn<ESM3::Race> (true, false));
    mRaces.addColumn (new WeightHeightColumn<ESM3::Race> (false, true));
    mRaces.addColumn (new WeightHeightColumn<ESM3::Race> (false, false));
    // Race spells
    mRaces.addColumn (new NestedParentColumn<ESM3::Race> (Columns::ColumnId_PowerList));
    index = mRaces.getColumns()-1;
    mRaces.addAdapter (std::make_pair(&mRaces.getColumn(index), new SpellListAdapter<ESM3::Race> ()));
    mRaces.getNestableColumn(index)->addColumn(
        new NestedChildColumn (Columns::ColumnId_SpellId, ColumnBase::Display_Spell));
    // Race attributes
    mRaces.addColumn (new NestedParentColumn<ESM3::Race> (Columns::ColumnId_RaceAttributes,
        ColumnBase::Flag_Dialogue, true)); // fixed rows table
    index = mRaces.getColumns()-1;
    mRaces.addAdapter (std::make_pair(&mRaces.getColumn(index), new RaceAttributeAdapter()));
    mRaces.getNestableColumn(index)->addColumn(
        new NestedChildColumn (Columns::ColumnId_Attribute, ColumnBase::Display_Attribute,
            ColumnBase::Flag_Dialogue, false));
    mRaces.getNestableColumn(index)->addColumn(
        new NestedChildColumn (Columns::ColumnId_Male, ColumnBase::Display_Integer));
    mRaces.getNestableColumn(index)->addColumn(
        new NestedChildColumn (Columns::ColumnId_Female, ColumnBase::Display_Integer));
    // Race skill bonus
    mRaces.addColumn (new NestedParentColumn<ESM3::Race> (Columns::ColumnId_RaceSkillBonus,
        ColumnBase::Flag_Dialogue, true)); // fixed rows table
    index = mRaces.getColumns()-1;
    mRaces.addAdapter (std::make_pair(&mRaces.getColumn(index), new RaceSkillsBonusAdapter()));
    mRaces.getNestableColumn(index)->addColumn(
        new NestedChildColumn (Columns::ColumnId_Skill, ColumnBase::Display_SkillId));
    mRaces.getNestableColumn(index)->addColumn(
        new NestedChildColumn (Columns::ColumnId_RaceBonus, ColumnBase::Display_Integer));

    mSounds.addColumn (new StringIdColumn<ESM3::Sound>);
    mSounds.addColumn (new RecordStateColumn<ESM3::Sound>);
    mSounds.addColumn (new FixedRecordTypeColumn<ESM3::Sound> (UniversalId::Type_Sound));
    mSounds.addColumn (new SoundParamColumn<ESM3::Sound> (SoundParamColumn<ESM3::Sound>::Type_Volume));
    mSounds.addColumn (new SoundParamColumn<ESM3::Sound> (SoundParamColumn<ESM3::Sound>::Type_MinRange));
    mSounds.addColumn (new SoundParamColumn<ESM3::Sound> (SoundParamColumn<ESM3::Sound>::Type_MaxRange));
    mSounds.addColumn (new SoundFileColumn<ESM3::Sound>);

    mScripts.addColumn (new StringIdColumn<ESM3::Script>);
    mScripts.addColumn (new RecordStateColumn<ESM3::Script>);
    mScripts.addColumn (new FixedRecordTypeColumn<ESM3::Script> (UniversalId::Type_Script));
    mScripts.addColumn (new ScriptColumn<ESM3::Script> (ScriptColumn<ESM3::Script>::Type_File));

    mRegions.addColumn (new StringIdColumn<ESM3::Region>);
    mRegions.addColumn (new RecordStateColumn<ESM3::Region>);
    mRegions.addColumn (new FixedRecordTypeColumn<ESM3::Region> (UniversalId::Type_Region));
    mRegions.addColumn (new NameColumn<ESM3::Region>);
    mRegions.addColumn (new MapColourColumn<ESM3::Region>);
    mRegions.addColumn (new SleepListColumn<ESM3::Region>);
    // Region Weather
    mRegions.addColumn (new NestedParentColumn<ESM3::Region> (Columns::ColumnId_RegionWeather));
    index = mRegions.getColumns()-1;
    mRegions.addAdapter (std::make_pair(&mRegions.getColumn(index), new RegionWeatherAdapter ()));
    mRegions.getNestableColumn(index)->addColumn(
        new NestedChildColumn (Columns::ColumnId_WeatherName, ColumnBase::Display_String, false));
    mRegions.getNestableColumn(index)->addColumn(
        new NestedChildColumn (Columns::ColumnId_WeatherChance, ColumnBase::Display_UnsignedInteger8));
    // Region Sounds
    mRegions.addColumn (new NestedParentColumn<ESM3::Region> (Columns::ColumnId_RegionSounds));
    index = mRegions.getColumns()-1;
    mRegions.addAdapter (std::make_pair(&mRegions.getColumn(index), new RegionSoundListAdapter ()));
    mRegions.getNestableColumn(index)->addColumn(
        new NestedChildColumn (Columns::ColumnId_SoundName, ColumnBase::Display_Sound));
    mRegions.getNestableColumn(index)->addColumn(
        new NestedChildColumn (Columns::ColumnId_SoundChance, ColumnBase::Display_UnsignedInteger8));

    mBirthsigns.addColumn (new StringIdColumn<ESM3::BirthSign>);
    mBirthsigns.addColumn (new RecordStateColumn<ESM3::BirthSign>);
    mBirthsigns.addColumn (new FixedRecordTypeColumn<ESM3::BirthSign> (UniversalId::Type_Birthsign));
    mBirthsigns.addColumn (new NameColumn<ESM3::BirthSign>);
    mBirthsigns.addColumn (new TextureColumn<ESM3::BirthSign>);
    mBirthsigns.addColumn (new DescriptionColumn<ESM3::BirthSign>);
    // Birthsign spells
    mBirthsigns.addColumn (new NestedParentColumn<ESM3::BirthSign> (Columns::ColumnId_PowerList));
    index = mBirthsigns.getColumns()-1;
    mBirthsigns.addAdapter (std::make_pair(&mBirthsigns.getColumn(index),
        new SpellListAdapter<ESM3::BirthSign> ()));
    mBirthsigns.getNestableColumn(index)->addColumn(
        new NestedChildColumn (Columns::ColumnId_SpellId, ColumnBase::Display_Spell));

    mSpells.addColumn (new StringIdColumn<ESM3::Spell>);
    mSpells.addColumn (new RecordStateColumn<ESM3::Spell>);
    mSpells.addColumn (new FixedRecordTypeColumn<ESM3::Spell> (UniversalId::Type_Spell));
    mSpells.addColumn (new NameColumn<ESM3::Spell>);
    mSpells.addColumn (new SpellTypeColumn<ESM3::Spell>);
    mSpells.addColumn (new CostColumn<ESM3::Spell>);
    mSpells.addColumn (new FlagColumn<ESM3::Spell> (Columns::ColumnId_AutoCalc, 0x1));
    mSpells.addColumn (new FlagColumn<ESM3::Spell> (Columns::ColumnId_StarterSpell, 0x2));
    mSpells.addColumn (new FlagColumn<ESM3::Spell> (Columns::ColumnId_AlwaysSucceeds, 0x4));
    // Spell effects
    mSpells.addColumn (new NestedParentColumn<ESM3::Spell> (Columns::ColumnId_EffectList));
    index = mSpells.getColumns()-1;
    mSpells.addAdapter (std::make_pair(&mSpells.getColumn(index), new EffectsListAdapter<ESM3::Spell> ()));
    mSpells.getNestableColumn(index)->addColumn(
        new NestedChildColumn (Columns::ColumnId_EffectId, ColumnBase::Display_EffectId));
    mSpells.getNestableColumn(index)->addColumn(
        new NestedChildColumn (Columns::ColumnId_Skill, ColumnBase::Display_EffectSkill));
    mSpells.getNestableColumn(index)->addColumn(
        new NestedChildColumn (Columns::ColumnId_Attribute, ColumnBase::Display_EffectAttribute));
    mSpells.getNestableColumn(index)->addColumn(
        new NestedChildColumn (Columns::ColumnId_EffectRange, ColumnBase::Display_EffectRange));
    mSpells.getNestableColumn(index)->addColumn(
        new NestedChildColumn (Columns::ColumnId_EffectArea, ColumnBase::Display_String));
    mSpells.getNestableColumn(index)->addColumn(
        new NestedChildColumn (Columns::ColumnId_Duration, ColumnBase::Display_Integer)); // reuse from light
    mSpells.getNestableColumn(index)->addColumn(
        new NestedChildColumn (Columns::ColumnId_MinMagnitude, ColumnBase::Display_Integer));
    mSpells.getNestableColumn(index)->addColumn(
        new NestedChildColumn (Columns::ColumnId_MaxMagnitude, ColumnBase::Display_Integer));

    mTopics.addColumn (new StringIdColumn<ESM3::Dialogue>);
    mTopics.addColumn (new RecordStateColumn<ESM3::Dialogue>);
    mTopics.addColumn (new FixedRecordTypeColumn<ESM3::Dialogue> (UniversalId::Type_Topic));
    mTopics.addColumn (new DialogueTypeColumn<ESM3::Dialogue>);

    mJournals.addColumn (new StringIdColumn<ESM3::Dialogue>);
    mJournals.addColumn (new RecordStateColumn<ESM3::Dialogue>);
    mJournals.addColumn (new FixedRecordTypeColumn<ESM3::Dialogue> (UniversalId::Type_Journal));
    mJournals.addColumn (new DialogueTypeColumn<ESM3::Dialogue> (true));

    mTopicInfos.addColumn (new StringIdColumn<Info> (true));
    mTopicInfos.addColumn (new RecordStateColumn<Info>);
    mTopicInfos.addColumn (new FixedRecordTypeColumn<Info> (UniversalId::Type_TopicInfo));
    mTopicInfos.addColumn (new TopicColumn<Info> (false));
    mTopicInfos.addColumn (new ActorColumn<Info>);
    mTopicInfos.addColumn (new RaceColumn<Info>);
    mTopicInfos.addColumn (new ClassColumn<Info>);
    mTopicInfos.addColumn (new FactionColumn<Info>);
    mTopicInfos.addColumn (new CellColumn<Info>);
    mTopicInfos.addColumn (new DispositionColumn<Info>);
    mTopicInfos.addColumn (new RankColumn<Info>);
    mTopicInfos.addColumn (new GenderColumn<Info>);
    mTopicInfos.addColumn (new PcFactionColumn<Info>);
    mTopicInfos.addColumn (new PcRankColumn<Info>);
    mTopicInfos.addColumn (new SoundFileColumn<Info>);
    mTopicInfos.addColumn (new ResponseColumn<Info>);
    // Result script
    mTopicInfos.addColumn (new NestedParentColumn<Info> (Columns::ColumnId_InfoList,
        ColumnBase::Flag_Dialogue | ColumnBase::Flag_Dialogue_List));
    index = mTopicInfos.getColumns()-1;
    mTopicInfos.addAdapter (std::make_pair(&mTopicInfos.getColumn(index), new InfoListAdapter ()));
    mTopicInfos.getNestableColumn(index)->addColumn(
        new NestedChildColumn (Columns::ColumnId_ScriptText, ColumnBase::Display_ScriptLines));
    // Special conditions
    mTopicInfos.addColumn (new NestedParentColumn<Info> (Columns::ColumnId_InfoCondition));
    index = mTopicInfos.getColumns()-1;
    mTopicInfos.addAdapter (std::make_pair(&mTopicInfos.getColumn(index), new InfoConditionAdapter ()));
    mTopicInfos.getNestableColumn(index)->addColumn(
        new NestedChildColumn (Columns::ColumnId_InfoCondFunc, ColumnBase::Display_InfoCondFunc));
    // FIXME: don't have dynamic value enum delegate, use Display_String for now
    mTopicInfos.getNestableColumn(index)->addColumn(
        new NestedChildColumn (Columns::ColumnId_InfoCondVar, ColumnBase::Display_InfoCondVar));
    mTopicInfos.getNestableColumn(index)->addColumn(
        new NestedChildColumn (Columns::ColumnId_InfoCondComp, ColumnBase::Display_InfoCondComp));
    mTopicInfos.getNestableColumn(index)->addColumn(
        new NestedChildColumn (Columns::ColumnId_Value, ColumnBase::Display_Var));

    mJournalInfos.addColumn (new StringIdColumn<Info> (true));
    mJournalInfos.addColumn (new RecordStateColumn<Info>);
    mJournalInfos.addColumn (new FixedRecordTypeColumn<Info> (UniversalId::Type_JournalInfo));
    mJournalInfos.addColumn (new TopicColumn<Info> (true));
    mJournalInfos.addColumn (new QuestStatusTypeColumn<Info>);
    mJournalInfos.addColumn (new QuestIndexColumn<Info>);
    mJournalInfos.addColumn (new QuestDescriptionColumn<Info>);

    mCells.addColumn (new StringIdColumn<Cell>);
    mCells.addColumn (new RecordStateColumn<Cell>);
    mCells.addColumn (new FixedRecordTypeColumn<Cell> (UniversalId::Type_Cell));
    mCells.addColumn (new NameColumn<Cell>(ColumnBase::Display_String64));
    mCells.addColumn (new FlagColumn<Cell> (Columns::ColumnId_SleepForbidden, ESM3::Cell::NoSleep));
    mCells.addColumn (new FlagColumn<Cell> (Columns::ColumnId_InteriorWater, ESM3::Cell::HasWater,
        ColumnBase::Flag_Table | ColumnBase::Flag_Dialogue | ColumnBase::Flag_Dialogue_Refresh));
    mCells.addColumn (new FlagColumn<Cell> (Columns::ColumnId_InteriorSky, ESM3::Cell::QuasiEx,
        ColumnBase::Flag_Table | ColumnBase::Flag_Dialogue | ColumnBase::Flag_Dialogue_Refresh));
    mCells.addColumn (new RegionColumn<Cell>);
    mCells.addColumn (new RefNumCounterColumn<Cell>);
    // Misc Cell data
    mCells.addColumn (new NestedParentColumn<Cell> (Columns::ColumnId_Cell,
        ColumnBase::Flag_Dialogue | ColumnBase::Flag_Dialogue_List));
    index = mCells.getColumns()-1;
    mCells.addAdapter (std::make_pair(&mCells.getColumn(index), new CellListAdapter ()));
    mCells.getNestableColumn(index)->addColumn(
        new NestedChildColumn (Columns::ColumnId_Interior, ColumnBase::Display_Boolean,
        ColumnBase::Flag_Table | ColumnBase::Flag_Dialogue | ColumnBase::Flag_Dialogue_Refresh));
    mCells.getNestableColumn(index)->addColumn(
        new NestedChildColumn (Columns::ColumnId_Ambient, ColumnBase::Display_Colour));
    mCells.getNestableColumn(index)->addColumn(
        new NestedChildColumn (Columns::ColumnId_Sunlight, ColumnBase::Display_Colour));
    mCells.getNestableColumn(index)->addColumn(
        new NestedChildColumn (Columns::ColumnId_Fog, ColumnBase::Display_Colour));
    mCells.getNestableColumn(index)->addColumn(
        new NestedChildColumn (Columns::ColumnId_FogDensity, ColumnBase::Display_Float));
    mCells.getNestableColumn(index)->addColumn(
        new NestedChildColumn (Columns::ColumnId_WaterLevel, ColumnBase::Display_Float));
    mCells.getNestableColumn(index)->addColumn(
        new NestedChildColumn (Columns::ColumnId_MapColor, ColumnBase::Display_Colour));

    mEnchantments.addColumn (new StringIdColumn<ESM3::Enchantment>);
    mEnchantments.addColumn (new RecordStateColumn<ESM3::Enchantment>);
    mEnchantments.addColumn (new FixedRecordTypeColumn<ESM3::Enchantment> (UniversalId::Type_Enchantment));
    mEnchantments.addColumn (new EnchantmentTypeColumn<ESM3::Enchantment>);
    mEnchantments.addColumn (new CostColumn<ESM3::Enchantment>);
    mEnchantments.addColumn (new ChargesColumn2<ESM3::Enchantment>);
    mEnchantments.addColumn (new FlagColumn<ESM3::Enchantment> (Columns::ColumnId_AutoCalc, ESM3::Enchantment::Autocalc));
    // Enchantment effects
    mEnchantments.addColumn (new NestedParentColumn<ESM3::Enchantment> (Columns::ColumnId_EffectList));
    index = mEnchantments.getColumns()-1;
    mEnchantments.addAdapter (std::make_pair(&mEnchantments.getColumn(index),
        new EffectsListAdapter<ESM3::Enchantment> ()));
    mEnchantments.getNestableColumn(index)->addColumn(
        new NestedChildColumn (Columns::ColumnId_EffectId, ColumnBase::Display_EffectId));
    mEnchantments.getNestableColumn(index)->addColumn(
        new NestedChildColumn (Columns::ColumnId_Skill, ColumnBase::Display_EffectSkill));
    mEnchantments.getNestableColumn(index)->addColumn(
        new NestedChildColumn (Columns::ColumnId_Attribute, ColumnBase::Display_EffectAttribute));
    mEnchantments.getNestableColumn(index)->addColumn(
        new NestedChildColumn (Columns::ColumnId_EffectRange, ColumnBase::Display_EffectRange));
    mEnchantments.getNestableColumn(index)->addColumn(
        new NestedChildColumn (Columns::ColumnId_EffectArea, ColumnBase::Display_String));
    mEnchantments.getNestableColumn(index)->addColumn(
        new NestedChildColumn (Columns::ColumnId_Duration, ColumnBase::Display_Integer)); // reuse from light
    mEnchantments.getNestableColumn(index)->addColumn(
        new NestedChildColumn (Columns::ColumnId_MinMagnitude, ColumnBase::Display_Integer));
    mEnchantments.getNestableColumn(index)->addColumn(
        new NestedChildColumn (Columns::ColumnId_MaxMagnitude, ColumnBase::Display_Integer));

    mBodyParts.addColumn (new StringIdColumn<ESM3::BodyPart>);
    mBodyParts.addColumn (new RecordStateColumn<ESM3::BodyPart>);
    mBodyParts.addColumn (new FixedRecordTypeColumn<ESM3::BodyPart> (UniversalId::Type_BodyPart));
    mBodyParts.addColumn (new BodyPartTypeColumn<ESM3::BodyPart>);
    mBodyParts.addColumn (new VampireColumn<ESM3::BodyPart>);
    mBodyParts.addColumn(new GenderNpcColumn<ESM3::BodyPart>);
    mBodyParts.addColumn (new FlagColumn<ESM3::BodyPart> (Columns::ColumnId_Playable,
        ESM3::BodyPart::BPF_NotPlayable, ColumnBase::Flag_Table | ColumnBase::Flag_Dialogue, true));

    int meshTypeFlags = ColumnBase::Flag_Table | ColumnBase::Flag_Dialogue | ColumnBase::Flag_Dialogue_Refresh;
    MeshTypeColumn<ESM3::BodyPart> *meshTypeColumn = new MeshTypeColumn<ESM3::BodyPart>(meshTypeFlags);
    mBodyParts.addColumn (meshTypeColumn);
    mBodyParts.addColumn (new ModelColumn<ESM3::BodyPart>);
    mBodyParts.addColumn (new BodyPartRaceColumn(meshTypeColumn));

    mSoundGens.addColumn (new StringIdColumn<ESM3::SoundGenerator>);
    mSoundGens.addColumn (new RecordStateColumn<ESM3::SoundGenerator>);
    mSoundGens.addColumn (new FixedRecordTypeColumn<ESM3::SoundGenerator> (UniversalId::Type_SoundGen));
    mSoundGens.addColumn (new CreatureColumn<ESM3::SoundGenerator>);
    mSoundGens.addColumn (new SoundColumn<ESM3::SoundGenerator>);
    mSoundGens.addColumn (new SoundGeneratorTypeColumn<ESM3::SoundGenerator>);

    mMagicEffects.addColumn (new StringIdColumn<ESM3::MagicEffect>);
    mMagicEffects.addColumn (new RecordStateColumn<ESM3::MagicEffect>);
    mMagicEffects.addColumn (new FixedRecordTypeColumn<ESM3::MagicEffect> (UniversalId::Type_MagicEffect));
    mMagicEffects.addColumn (new SchoolColumn<ESM3::MagicEffect>);
    mMagicEffects.addColumn (new BaseCostColumn<ESM3::MagicEffect>);
    mMagicEffects.addColumn (new EffectTextureColumn<ESM3::MagicEffect> (Columns::ColumnId_Icon));
    mMagicEffects.addColumn (new EffectTextureColumn<ESM3::MagicEffect> (Columns::ColumnId_Particle));
    mMagicEffects.addColumn (new EffectObjectColumn<ESM3::MagicEffect> (Columns::ColumnId_CastingObject));
    mMagicEffects.addColumn (new EffectObjectColumn<ESM3::MagicEffect> (Columns::ColumnId_HitObject));
    mMagicEffects.addColumn (new EffectObjectColumn<ESM3::MagicEffect> (Columns::ColumnId_AreaObject));
    mMagicEffects.addColumn (new EffectObjectColumn<ESM3::MagicEffect> (Columns::ColumnId_BoltObject));
    mMagicEffects.addColumn (new EffectSoundColumn<ESM3::MagicEffect> (Columns::ColumnId_CastingSound));
    mMagicEffects.addColumn (new EffectSoundColumn<ESM3::MagicEffect> (Columns::ColumnId_HitSound));
    mMagicEffects.addColumn (new EffectSoundColumn<ESM3::MagicEffect> (Columns::ColumnId_AreaSound));
    mMagicEffects.addColumn (new EffectSoundColumn<ESM3::MagicEffect> (Columns::ColumnId_BoltSound));
    mMagicEffects.addColumn (new FlagColumn<ESM3::MagicEffect> (
        Columns::ColumnId_AllowSpellmaking, ESM3::MagicEffect::AllowSpellmaking));
    mMagicEffects.addColumn (new FlagColumn<ESM3::MagicEffect> (
        Columns::ColumnId_AllowEnchanting, ESM3::MagicEffect::AllowEnchanting));
    mMagicEffects.addColumn (new FlagColumn<ESM3::MagicEffect> (
        Columns::ColumnId_NegativeLight, ESM3::MagicEffect::NegativeLight));
    mMagicEffects.addColumn (new DescriptionColumn<ESM3::MagicEffect>);

    mLand.addColumn (new StringIdColumn<Land>);
    mLand.addColumn (new RecordStateColumn<Land>);
    mLand.addColumn (new FixedRecordTypeColumn<Land>(UniversalId::Type_Land));
    mLand.addColumn (new LandPluginIndexColumn);
    mLand.addColumn (new LandNormalsColumn);
    mLand.addColumn (new LandHeightsColumn);
    mLand.addColumn (new LandColoursColumn);
    mLand.addColumn (new LandTexturesColumn);

    mLandTextures.addColumn (new StringIdColumn<LandTexture>(true));
    mLandTextures.addColumn (new RecordStateColumn<LandTexture>);
    mLandTextures.addColumn (new FixedRecordTypeColumn<LandTexture>(UniversalId::Type_LandTexture));
    mLandTextures.addColumn (new LandTextureNicknameColumn);
    mLandTextures.addColumn (new LandTexturePluginIndexColumn);
    mLandTextures.addColumn (new LandTextureIndexColumn);
    mLandTextures.addColumn (new TextureColumn<LandTexture>);

    mPathgrids.addColumn (new StringIdColumn<Pathgrid>);
    mPathgrids.addColumn (new RecordStateColumn<Pathgrid>);
    mPathgrids.addColumn (new FixedRecordTypeColumn<Pathgrid> (UniversalId::Type_Pathgrid));

    // new object deleted in dtor of Collection<T,A>
    mPathgrids.addColumn (new NestedParentColumn<Pathgrid> (Columns::ColumnId_PathgridPoints));
    index = mPathgrids.getColumns()-1;
    // new object deleted in dtor of NestedCollection<T,A>
    mPathgrids.addAdapter (std::make_pair(&mPathgrids.getColumn(index), new PathgridPointListAdapter ()));
    // new objects deleted in dtor of NestableColumn
    // WARNING: The order of the columns below are assumed in PathgridPointListAdapter
    mPathgrids.getNestableColumn(index)->addColumn(
            new NestedChildColumn (Columns::ColumnId_PathgridIndex, ColumnBase::Display_Integer,
                ColumnBase::Flag_Dialogue, false));
    mPathgrids.getNestableColumn(index)->addColumn(
            new NestedChildColumn (Columns::ColumnId_PathgridPosX, ColumnBase::Display_Integer));
    mPathgrids.getNestableColumn(index)->addColumn(
            new NestedChildColumn (Columns::ColumnId_PathgridPosY, ColumnBase::Display_Integer));
    mPathgrids.getNestableColumn(index)->addColumn(
            new NestedChildColumn (Columns::ColumnId_PathgridPosZ, ColumnBase::Display_Integer));

    mPathgrids.addColumn (new NestedParentColumn<Pathgrid> (Columns::ColumnId_PathgridEdges));
    index = mPathgrids.getColumns()-1;
    mPathgrids.addAdapter (std::make_pair(&mPathgrids.getColumn(index), new PathgridEdgeListAdapter ()));
    mPathgrids.getNestableColumn(index)->addColumn(
            new NestedChildColumn (Columns::ColumnId_PathgridEdgeIndex, ColumnBase::Display_Integer,
                ColumnBase::Flag_Dialogue, false));
    mPathgrids.getNestableColumn(index)->addColumn(
            new NestedChildColumn (Columns::ColumnId_PathgridEdge0, ColumnBase::Display_Integer));
    mPathgrids.getNestableColumn(index)->addColumn(
            new NestedChildColumn (Columns::ColumnId_PathgridEdge1, ColumnBase::Display_Integer));

    mStartScripts.addColumn (new StringIdColumn<ESM3::StartScript>);
    mStartScripts.addColumn (new RecordStateColumn<ESM3::StartScript>);
    mStartScripts.addColumn (new FixedRecordTypeColumn<ESM3::StartScript> (UniversalId::Type_StartScript));

    mRefs.addColumn (new StringIdColumn<CellRef> (true));
    mRefs.addColumn (new RecordStateColumn<CellRef>);
    mRefs.addColumn (new FixedRecordTypeColumn<CellRef> (UniversalId::Type_Reference));
    mRefs.addColumn (new CellColumn<CellRef> (true));
    mRefs.addColumn (new OriginalCellColumn<CellRef>);
    mRefs.addColumn (new IdColumn<CellRef>);
    mRefs.addColumn (new PosColumn<CellRef> (&CellRef::mPos, 0, false));
    mRefs.addColumn (new PosColumn<CellRef> (&CellRef::mPos, 1, false));
    mRefs.addColumn (new PosColumn<CellRef> (&CellRef::mPos, 2, false));
    mRefs.addColumn (new RotColumn<CellRef> (&CellRef::mPos, 0, false));
    mRefs.addColumn (new RotColumn<CellRef> (&CellRef::mPos, 1, false));
    mRefs.addColumn (new RotColumn<CellRef> (&CellRef::mPos, 2, false));
    mRefs.addColumn (new ScaleColumn<CellRef>);
    mRefs.addColumn (new OwnerColumn<CellRef>);
    mRefs.addColumn (new SoulColumn<CellRef>);
    mRefs.addColumn (new FactionColumn<CellRef>);
    mRefs.addColumn (new FactionIndexColumn<CellRef>);
    mRefs.addColumn (new ChargesColumn<CellRef>);
    mRefs.addColumn (new EnchantmentChargesColumn<CellRef>);
    mRefs.addColumn (new GoldValueColumn<CellRef>);
    mRefs.addColumn (new TeleportColumn<CellRef>);
    mRefs.addColumn (new TeleportCellColumn<CellRef>);
    mRefs.addColumn (new PosColumn<CellRef> (&CellRef::mDoorDest, 0, true));
    mRefs.addColumn (new PosColumn<CellRef> (&CellRef::mDoorDest, 1, true));
    mRefs.addColumn (new PosColumn<CellRef> (&CellRef::mDoorDest, 2, true));
    mRefs.addColumn (new RotColumn<CellRef> (&CellRef::mDoorDest, 0, true));
    mRefs.addColumn (new RotColumn<CellRef> (&CellRef::mDoorDest, 1, true));
    mRefs.addColumn (new RotColumn<CellRef> (&CellRef::mDoorDest, 2, true));
    mRefs.addColumn (new LockLevelColumn<CellRef>);
    mRefs.addColumn (new KeyColumn<CellRef>);
    mRefs.addColumn (new TrapColumn<CellRef>);
    mRefs.addColumn (new OwnerGlobalColumn<CellRef>);
    mRefs.addColumn (new RefNumColumn<CellRef>);

    mFilters.addColumn (new StringIdColumn<ESM3::Filter>);
    mFilters.addColumn (new RecordStateColumn<ESM3::Filter>);
    mFilters.addColumn (new FixedRecordTypeColumn<ESM3::Filter> (UniversalId::Type_Filter));
    mFilters.addColumn (new FilterColumn<ESM3::Filter>);
    mFilters.addColumn (new DescriptionColumn<ESM3::Filter>);

    mDebugProfiles.addColumn (new StringIdColumn<ESM3::DebugProfile>);
    mDebugProfiles.addColumn (new RecordStateColumn<ESM3::DebugProfile>);
    mDebugProfiles.addColumn (new FixedRecordTypeColumn<ESM3::DebugProfile> (UniversalId::Type_DebugProfile));
    mDebugProfiles.addColumn (new FlagColumn2<ESM3::DebugProfile> (
        Columns::ColumnId_DefaultProfile, ESM3::DebugProfile::Flag_Default));
    mDebugProfiles.addColumn (new FlagColumn2<ESM3::DebugProfile> (
        Columns::ColumnId_BypassNewGame, ESM3::DebugProfile::Flag_BypassNewGame));
    mDebugProfiles.addColumn (new FlagColumn2<ESM3::DebugProfile> (
        Columns::ColumnId_GlobalProfile, ESM3::DebugProfile::Flag_Global));
    mDebugProfiles.addColumn (new DescriptionColumn<ESM3::DebugProfile>);
    mDebugProfiles.addColumn (new ScriptColumn<ESM3::DebugProfile> (
        ScriptColumn<ESM3::DebugProfile>::Type_Lines));

    mMetaData.appendBlankRecord ("sys::meta");

    mMetaData.addColumn (new StringIdColumn<MetaData> (true));
    mMetaData.addColumn (new RecordStateColumn<MetaData>);
    mMetaData.addColumn (new FixedRecordTypeColumn<MetaData> (UniversalId::Type_MetaData));
    mMetaData.addColumn (new FormatColumn<MetaData>);
    mMetaData.addColumn (new AuthorColumn<MetaData>);
    mMetaData.addColumn (new FileDescriptionColumn<MetaData>);

    addModel (new IdTable (&mGlobals), UniversalId::Type_Global);
    addModel (new IdTable (&mGmsts), UniversalId::Type_Gmst);
    addModel (new IdTable (&mSkills), UniversalId::Type_Skill);
    addModel (new IdTable (&mClasses), UniversalId::Type_Class);
    addModel (new IdTree (&mFactions, &mFactions), UniversalId::Type_Faction);
    addModel (new IdTree (&mRaces, &mRaces), UniversalId::Type_Race);
    addModel (new IdTable (&mSounds), UniversalId::Type_Sound);
    addModel (new IdTable (&mScripts), UniversalId::Type_Script);
    addModel (new IdTree (&mRegions, &mRegions), UniversalId::Type_Region);
    addModel (new IdTree (&mBirthsigns, &mBirthsigns), UniversalId::Type_Birthsign);
    addModel (new IdTree (&mSpells, &mSpells), UniversalId::Type_Spell);
    addModel (new IdTable (&mTopics), UniversalId::Type_Topic);
    addModel (new IdTable (&mJournals), UniversalId::Type_Journal);
    addModel (new IdTree (&mTopicInfos, &mTopicInfos, IdTable::Feature_ReorderWithinTopic),
        UniversalId::Type_TopicInfo);
    addModel (new IdTable (&mJournalInfos, IdTable::Feature_ReorderWithinTopic), UniversalId::Type_JournalInfo);
    addModel (new IdTree (&mCells, &mCells, IdTable::Feature_ViewId), UniversalId::Type_Cell);
    addModel (new IdTree (&mEnchantments, &mEnchantments), UniversalId::Type_Enchantment);
    addModel (new IdTable (&mBodyParts), UniversalId::Type_BodyPart);
    addModel (new IdTable (&mSoundGens), UniversalId::Type_SoundGen);
    addModel (new IdTable (&mMagicEffects), UniversalId::Type_MagicEffect);
    addModel (new IdTable (&mLand, IdTable::Feature_AllowTouch), UniversalId::Type_Land);
    addModel (new LandTextureIdTable (&mLandTextures), UniversalId::Type_LandTexture);
    addModel (new IdTree (&mPathgrids, &mPathgrids), UniversalId::Type_Pathgrid);
    addModel (new IdTable (&mStartScripts), UniversalId::Type_StartScript);
    addModel (new IdTree (&mReferenceables, &mReferenceables, IdTable::Feature_Preview),
        UniversalId::Type_Referenceable);
    addModel (new IdTable (&mRefs, IdTable::Feature_ViewCell | IdTable::Feature_Preview), UniversalId::Type_Reference);
    addModel (new IdTable (&mFilters), UniversalId::Type_Filter);
    addModel (new IdTable (&mDebugProfiles), UniversalId::Type_DebugProfile);
    addModel (new ResourceTable (&mResourcesManager.get (UniversalId::Type_Meshes)),
        UniversalId::Type_Mesh);
    addModel (new ResourceTable (&mResourcesManager.get (UniversalId::Type_Icons)),
        UniversalId::Type_Icon);
    addModel (new ResourceTable (&mResourcesManager.get (UniversalId::Type_Musics)),
        UniversalId::Type_Music);
    addModel (new ResourceTable (&mResourcesManager.get (UniversalId::Type_SoundsRes)),
        UniversalId::Type_SoundRes);
    addModel (new ResourceTable (&mResourcesManager.get (UniversalId::Type_Textures)),
        UniversalId::Type_Texture);
    addModel (new ResourceTable (&mResourcesManager.get (UniversalId::Type_Videos)),
        UniversalId::Type_Video);
    addModel (new IdTable (&mMetaData), UniversalId::Type_MetaData);

    mActorAdapter.reset(new ActorAdapter(*this));

    mRefLoadCache.clear(); // clear here rather than startLoading() and continueLoading() for multiple content files
}

CSMWorld::Data::~Data()
{
    for (std::vector<QAbstractItemModel *>::iterator iter (mModels.begin()); iter!=mModels.end(); ++iter)
        delete *iter;

    delete mReader;
}

std::shared_ptr<Resource::ResourceSystem> CSMWorld::Data::getResourceSystem()
{
    return mResourceSystem;
}

std::shared_ptr<const Resource::ResourceSystem> CSMWorld::Data::getResourceSystem() const
{
    return mResourceSystem;
}

const CSMWorld::IdCollection<ESM3::Global>& CSMWorld::Data::getGlobals() const
{
    return mGlobals;
}

CSMWorld::IdCollection<ESM3::Global>& CSMWorld::Data::getGlobals()
{
    return mGlobals;
}

const CSMWorld::IdCollection<ESM3::GameSetting>& CSMWorld::Data::getGmsts() const
{
    return mGmsts;
}

CSMWorld::IdCollection<ESM3::GameSetting>& CSMWorld::Data::getGmsts()
{
    return mGmsts;
}

const CSMWorld::IdCollection<ESM3::Skill>& CSMWorld::Data::getSkills() const
{
    return mSkills;
}

CSMWorld::IdCollection<ESM3::Skill>& CSMWorld::Data::getSkills()
{
    return mSkills;
}

const CSMWorld::IdCollection<ESM3::Class>& CSMWorld::Data::getClasses() const
{
    return mClasses;
}

CSMWorld::IdCollection<ESM3::Class>& CSMWorld::Data::getClasses()
{
    return mClasses;
}

const CSMWorld::IdCollection<ESM3::Faction>& CSMWorld::Data::getFactions() const
{
    return mFactions;
}

CSMWorld::IdCollection<ESM3::Faction>& CSMWorld::Data::getFactions()
{
    return mFactions;
}

const CSMWorld::IdCollection<ESM3::Race>& CSMWorld::Data::getRaces() const
{
    return mRaces;
}

CSMWorld::IdCollection<ESM3::Race>& CSMWorld::Data::getRaces()
{
    return mRaces;
}

const CSMWorld::IdCollection<ESM3::Sound>& CSMWorld::Data::getSounds() const
{
    return mSounds;
}

CSMWorld::IdCollection<ESM3::Sound>& CSMWorld::Data::getSounds()
{
    return mSounds;
}

const CSMWorld::IdCollection<ESM3::Script>& CSMWorld::Data::getScripts() const
{
    return mScripts;
}

CSMWorld::IdCollection<ESM3::Script>& CSMWorld::Data::getScripts()
{
    return mScripts;
}

const CSMWorld::IdCollection<ESM3::Region>& CSMWorld::Data::getRegions() const
{
    return mRegions;
}

CSMWorld::IdCollection<ESM3::Region>& CSMWorld::Data::getRegions()
{
    return mRegions;
}

const CSMWorld::IdCollection<ESM3::BirthSign>& CSMWorld::Data::getBirthsigns() const
{
    return mBirthsigns;
}

CSMWorld::IdCollection<ESM3::BirthSign>& CSMWorld::Data::getBirthsigns()
{
    return mBirthsigns;
}

const CSMWorld::IdCollection<ESM3::Spell>& CSMWorld::Data::getSpells() const
{
    return mSpells;
}

CSMWorld::IdCollection<ESM3::Spell>& CSMWorld::Data::getSpells()
{
    return mSpells;
}


const CSMWorld::IdCollection<ESM3::Dialogue>& CSMWorld::Data::getTopics() const
{
    return mTopics;
}

CSMWorld::IdCollection<ESM3::Dialogue>& CSMWorld::Data::getTopics()
{
    return mTopics;
}

const CSMWorld::IdCollection<ESM3::Dialogue>& CSMWorld::Data::getJournals() const
{
    return mJournals;
}

CSMWorld::IdCollection<ESM3::Dialogue>& CSMWorld::Data::getJournals()
{
    return mJournals;
}

const CSMWorld::InfoCollection& CSMWorld::Data::getTopicInfos() const
{
    return mTopicInfos;
}

CSMWorld::InfoCollection& CSMWorld::Data::getTopicInfos()
{
    return mTopicInfos;
}

const CSMWorld::InfoCollection& CSMWorld::Data::getJournalInfos() const
{
    return mJournalInfos;
}

CSMWorld::InfoCollection& CSMWorld::Data::getJournalInfos()
{
    return mJournalInfos;
}

const CSMWorld::IdCollection<CSMWorld::Cell>& CSMWorld::Data::getCells() const
{
    return mCells;
}

CSMWorld::IdCollection<CSMWorld::Cell>& CSMWorld::Data::getCells()
{
    return mCells;
}

const CSMWorld::RefIdCollection& CSMWorld::Data::getReferenceables() const
{
    return mReferenceables;
}

CSMWorld::RefIdCollection& CSMWorld::Data::getReferenceables()
{
    return mReferenceables;
}

const CSMWorld::RefCollection& CSMWorld::Data::getReferences() const
{
    return mRefs;
}

CSMWorld::RefCollection& CSMWorld::Data::getReferences()
{
    return mRefs;
}

const CSMWorld::IdCollection<ESM3::Filter>& CSMWorld::Data::getFilters() const
{
    return mFilters;
}

CSMWorld::IdCollection<ESM3::Filter>& CSMWorld::Data::getFilters()
{
    return mFilters;
}

const CSMWorld::IdCollection<ESM3::Enchantment>& CSMWorld::Data::getEnchantments() const
{
    return mEnchantments;
}

CSMWorld::IdCollection<ESM3::Enchantment>& CSMWorld::Data::getEnchantments()
{
    return mEnchantments;
}

const CSMWorld::IdCollection<ESM3::BodyPart>& CSMWorld::Data::getBodyParts() const
{
    return mBodyParts;
}

CSMWorld::IdCollection<ESM3::BodyPart>& CSMWorld::Data::getBodyParts()
{
    return mBodyParts;
}

const CSMWorld::IdCollection<ESM3::DebugProfile>& CSMWorld::Data::getDebugProfiles() const
{
    return mDebugProfiles;
}

CSMWorld::IdCollection<ESM3::DebugProfile>& CSMWorld::Data::getDebugProfiles()
{
    return mDebugProfiles;
}

const CSMWorld::IdCollection<CSMWorld::Land>& CSMWorld::Data::getLand() const
{
    return mLand;
}

CSMWorld::IdCollection<CSMWorld::Land>& CSMWorld::Data::getLand()
{
    return mLand;
}

const CSMWorld::IdCollection<CSMWorld::LandTexture>& CSMWorld::Data::getLandTextures() const
{
    return mLandTextures;
}

CSMWorld::IdCollection<CSMWorld::LandTexture>& CSMWorld::Data::getLandTextures()
{
    return mLandTextures;
}

const CSMWorld::IdCollection<ESM3::SoundGenerator>& CSMWorld::Data::getSoundGens() const
{
    return mSoundGens;
}

CSMWorld::IdCollection<ESM3::SoundGenerator>& CSMWorld::Data::getSoundGens()
{
    return mSoundGens;
}

const CSMWorld::IdCollection<ESM3::MagicEffect>& CSMWorld::Data::getMagicEffects() const
{
    return mMagicEffects;
}

CSMWorld::IdCollection<ESM3::MagicEffect>& CSMWorld::Data::getMagicEffects()
{
    return mMagicEffects;
}

const CSMWorld::SubCellCollection<CSMWorld::Pathgrid>& CSMWorld::Data::getPathgrids() const
{
    return mPathgrids;
}

CSMWorld::SubCellCollection<CSMWorld::Pathgrid>& CSMWorld::Data::getPathgrids()
{
    return mPathgrids;
}

const CSMWorld::IdCollection<ESM3::StartScript>& CSMWorld::Data::getStartScripts() const
{
    return mStartScripts;
}

CSMWorld::IdCollection<ESM3::StartScript>& CSMWorld::Data::getStartScripts()
{
    return mStartScripts;
}

const CSMWorld::Resources& CSMWorld::Data::getResources (const UniversalId& id) const
{
    return mResourcesManager.get (id.getType());
}

const CSMWorld::MetaData& CSMWorld::Data::getMetaData() const
{
    return mMetaData.getRecord (0).get();
}

void CSMWorld::Data::setMetaData (const MetaData& metaData)
{
    mMetaData.setRecord (0, std::make_unique<Record<MetaData> >(
            Record<MetaData>(RecordBase::State_ModifiedOnly, nullptr, &metaData)));
}

QAbstractItemModel *CSMWorld::Data::getTableModel (const CSMWorld::UniversalId& id)
{
    std::map<UniversalId::Type, QAbstractItemModel *>::iterator iter = mModelIndex.find (id.getType());

    if (iter==mModelIndex.end())
    {
        // try creating missing (secondary) tables on the fly
        //
        // Note: We create these tables here so we don't have to deal with them during load/initial
        // construction of the ESX data where no update signals are available.
        if (id.getType()==UniversalId::Type_RegionMap)
        {
            RegionMap *table = nullptr;
            addModel (table = new RegionMap (*this), UniversalId::Type_RegionMap, false);
            return table;
        }
        throw std::logic_error ("No table model available for " + id.toString());
    }

    return iter->second;
}

const CSMWorld::ActorAdapter* CSMWorld::Data::getActorAdapter() const
{
    return mActorAdapter.get();
}

CSMWorld::ActorAdapter* CSMWorld::Data::getActorAdapter()
{
    return mActorAdapter.get();
}

void CSMWorld::Data::merge()
{
    mGlobals.merge();
}

int CSMWorld::Data::getTotalRecords (const std::vector<boost::filesystem::path>& files)
{
    int records = 0;

    for (unsigned int i = 0; i < files.size(); ++i)
    {
        if (!boost::filesystem::exists(files[i]))
            continue;

        ESM::Reader* reader = ESM::Reader::getReader(files[i].string());
        records += reader->getRecordCount();
        reader->close();
    }

    return records;
}

int CSMWorld::Data::startLoading (const boost::filesystem::path& path, bool base, bool project)
{
    // This section was probably copied from OpenMW where ESM3Terrain::Storage::getVtexIndexAt()
    // gets the plugin index (i.e. ESM3::Land::mPlugin) but even in OpenMW the readers are not
    // reused (a new reader is created and saved context is restored).
    // Also see: 8786fb639ff5ce1d1e608ede1f11a746df9319cc
#if 0
    // Don't delete the Reader yet. Some record types store a reference to the Reader to handle on-demand loading
    if (mReader) // only for ESM4
    {
        std::shared_ptr<ESM::Reader> ptr;
        ptr.reset(mReader);
        mReaders.push_back(ptr);

        mReader = nullptr;
    }
#else
    delete mReader;
#endif

    if (mDialogue)
        throw std::logic_error ("won't start loading, someone forgot to cleanup");

    mReader = ESM::Reader::getReader(path.string());
    mReader->setEncoder (&mEncoder);

    mReader->setModIndex((project || !base) ? 0 : mReaderIndex++);
    mLoadedFiles.push_back(path.filename().string());

    if (mReader->isEsm4())
    {
        ESM4::Reader* reader = static_cast<ESM4::Reader*>(mReader);
        reader->updateModIndices(mLoadedFiles);
    }

    // NOTE: Unlike OpenMW (see ESMStore::load()) we don't enforce content file master
    //       dependencies.  For OpenCS, the content selector won't allow it anyway.
    //       Even if the content selector rule is relaxed in future we shouldn't enforce
    //       it because the end-user may have legitimate reasons.

    mBase = base;
    mProject = project;

    if (!mProject && !mBase && !mReader->isEsm4()) // FIXME: not yet supported for ESM4
    {
        MetaData metaData;
        metaData.mId = "sys::meta";
        metaData.load (static_cast<ESM3::Reader&>(*mReader));

        mMetaData.setRecord (0, std::make_unique<Record<MetaData> >(
                    Record<MetaData> (RecordBase::State_ModifiedOnly, nullptr, &metaData)));
    }

    return mReader->getRecordCount();
}

void CSMWorld::Data::loadFallbackEntries()
{
    // Load default marker definitions, if game files do not have them for some reason
    std::pair<std::string, std::string> staticMarkers[] = {
        std::make_pair("DivineMarker", "marker_divine.nif"),
        std::make_pair("DoorMarker", "marker_arrow.nif"),
        std::make_pair("NorthMarker", "marker_north.nif"),
        std::make_pair("TempleMarker", "marker_temple.nif"),
        std::make_pair("TravelMarker", "marker_travel.nif")
    };

    std::pair<std::string, std::string> doorMarkers[] = {
        std::make_pair("PrisonMarker", "marker_prison.nif")
    };

    for (const auto &marker : staticMarkers)
    {
        if (mReferenceables.searchId (marker.first)==-1)
        {
            ESM3::Static newMarker;
            newMarker.mId = marker.first;
            newMarker.mModel = marker.second;
            std::unique_ptr<CSMWorld::Record<ESM3::Static> > record(new CSMWorld::Record<ESM3::Static>);
            record->mBase = newMarker;
            record->mState = CSMWorld::RecordBase::State_BaseOnly;
            mReferenceables.appendRecord (std::move(record), CSMWorld::UniversalId::Type_Static);
        }
    }

    for (const auto &marker : doorMarkers)
    {
        if (mReferenceables.searchId (marker.first)==-1)
        {
            ESM3::Door newMarker;
            newMarker.mId = marker.first;
            newMarker.mModel = marker.second;
            std::unique_ptr<CSMWorld::Record<ESM3::Door> > record(new CSMWorld::Record<ESM3::Door>);
            record->mBase = newMarker;
            record->mState = CSMWorld::RecordBase::State_BaseOnly;
            mReferenceables.appendRecord (std::move(record), CSMWorld::UniversalId::Type_Door);
        }
    }
}

bool CSMWorld::Data::continueLoading (CSMDoc::Messages& messages)
{
    if (!mReader)
        throw std::logic_error ("can't continue loading, because no load has been started");

    if (!mReader->hasMoreRecs()) // FIXME: how to do this independently without vtable
    {
#if 0
        if (mBase)
        {
            // Don't delete the Reader yet. Some record types store a reference to the Reader to handle on-demand loading.
            // We don't store non-base reader, because everything going into modified will be
            // fully loaded during the initial loading process.
            std::shared_ptr<ESM::Reader> ptr;
            ptr.reset(mReader);
            mReaders.push_back(ptr);
        }
        else
#endif
            delete mReader;

        mReader = nullptr;

        mDialogue = nullptr;

        loadFallbackEntries();

        return true;
    }

    if (mReader->isEsm4())
    {
        ESM4::Reader* reader = static_cast<ESM4::Reader*>(mReader);

        unsigned int esmVer = reader->esmVersion();
        bool isTes4 = esmVer == ESM4::VER_080 || esmVer == ESM4::VER_100;
        bool isTes5 = esmVer == ESM4::VER_094 || esmVer == ESM4::VER_170;
        bool isFONV = esmVer == ESM4::VER_132 || esmVer == ESM4::VER_133 || esmVer == ESM4::VER_134;
        if (isTes4 || isTes5 || isFONV)
        {
            // Check if previous record/group was the final one in this group.  Must be done before
            // calling mReader->hasMoreRecs() below, because all records may have been processed when
            // the previous group is popped off the stack.
            reader->exitGroupCheck();

            return loadTes4Group(messages);
        }
    }

    ESM3::Reader *reader = static_cast<ESM3::Reader*>(mReader);
    reader->getRecordHeader();
    const ESM3::RecordHeader& hdr = reader->hdr();

    bool unhandledRecord = false;
    switch (hdr.typeId)
    {
        case ESM3::REC_GLOB: mGlobals.load (*reader, mBase); break;
        case ESM3::REC_GMST: mGmsts.load (*reader, mBase); break;
        case ESM3::REC_SKIL: mSkills.load (*reader, mBase); break;
        case ESM3::REC_CLAS: mClasses.load (*reader, mBase); break;
        case ESM3::REC_FACT: mFactions.load (*reader, mBase); break;
        case ESM3::REC_RACE: mRaces.load (*reader, mBase); break;
        case ESM3::REC_SOUN: mSounds.load (*reader, mBase); break;
        case ESM3::REC_SCPT: mScripts.load (*reader, mBase); break;
        case ESM3::REC_REGN: mRegions.load (*reader, mBase); break;
        case ESM3::REC_BSGN: mBirthsigns.load (*reader, mBase); break;
        case ESM3::REC_SPEL: mSpells.load (*reader, mBase); break;
        case ESM3::REC_ENCH: mEnchantments.load (*reader, mBase); break;
        case ESM3::REC_BODY: mBodyParts.load (*reader, mBase); break;
        case ESM3::REC_SNDG: mSoundGens.load (*reader, mBase); break;
        case ESM3::REC_MGEF: mMagicEffects.load (*reader, mBase); break;
        case ESM3::REC_PGRD: mPathgrids.load (*reader, mBase); break;
        case ESM3::REC_SSCR: mStartScripts.load (*reader, mBase); break;

        case ESM3::REC_LTEX: mLandTextures.load (*reader, mBase); break;

        case ESM3::REC_LAND: mLand.load(*reader, mBase); break;

        case ESM3::REC_CELL:
        {
            int index = mCells.load (*reader, mBase);
            if (index < 0 || index >= mCells.getSize())
            {
                // log an error and continue loading the refs to the last loaded cell
                CSMWorld::UniversalId id (CSMWorld::UniversalId::Type_None);
                messages.add (id, "Logic error: cell index out of bounds", "", CSMDoc::Message::Severity_Error);
                index = mCells.getSize()-1;
            }
            std::string cellId = Misc::StringUtils::lowerCase (mCells.getId (index));
            mRefs.load (*reader, index, mBase, mRefLoadCache[cellId], messages);
            break;
        }

        case ESM3::REC_ACTI: mReferenceables.load (*reader, mBase, UniversalId::Type_Activator); break;
        case ESM3::REC_ALCH: mReferenceables.load (*reader, mBase, UniversalId::Type_Potion); break;
        case ESM3::REC_APPA: mReferenceables.load (*reader, mBase, UniversalId::Type_Apparatus); break;
        case ESM3::REC_ARMO: mReferenceables.load (*reader, mBase, UniversalId::Type_Armor); break;
        case ESM3::REC_BOOK: mReferenceables.load (*reader, mBase, UniversalId::Type_Book); break;
        case ESM3::REC_CLOT: mReferenceables.load (*reader, mBase, UniversalId::Type_Clothing); break;
        case ESM3::REC_CONT: mReferenceables.load (*reader, mBase, UniversalId::Type_Container); break;
        case ESM3::REC_CREA: mReferenceables.load (*reader, mBase, UniversalId::Type_Creature); break;
        case ESM3::REC_DOOR: mReferenceables.load (*reader, mBase, UniversalId::Type_Door); break;
        case ESM3::REC_INGR: mReferenceables.load (*reader, mBase, UniversalId::Type_Ingredient); break;
        case ESM3::REC_LEVC:
            mReferenceables.load (*reader, mBase, UniversalId::Type_CreatureLevelledList); break;
        case ESM3::REC_LEVI:
            mReferenceables.load (*reader, mBase, UniversalId::Type_ItemLevelledList); break;
        case ESM3::REC_LIGH: mReferenceables.load (*reader, mBase, UniversalId::Type_Light); break;
        case ESM3::REC_LOCK: mReferenceables.load (*reader, mBase, UniversalId::Type_Lockpick); break;
        case ESM3::REC_MISC:
            mReferenceables.load (*reader, mBase, UniversalId::Type_Miscellaneous); break;
        case ESM3::REC_NPC_: mReferenceables.load (*reader, mBase, UniversalId::Type_Npc); break;
        case ESM3::REC_PROB: mReferenceables.load (*reader, mBase, UniversalId::Type_Probe); break;
        case ESM3::REC_REPA: mReferenceables.load (*reader, mBase, UniversalId::Type_Repair); break;
        case ESM3::REC_STAT: mReferenceables.load (*reader, mBase, UniversalId::Type_Static); break;
        case ESM3::REC_WEAP: mReferenceables.load (*reader, mBase, UniversalId::Type_Weapon); break;

        case ESM3::REC_DIAL:
        {
            ESM3::Dialogue record;
            bool isDeleted = false;

            record.load (*reader, isDeleted);

            if (isDeleted)
            {
                // record vector can be shuffled around which would make pointer to record invalid
                mDialogue = nullptr;

                if (mJournals.tryDelete (record.mId))
                {
                    mJournalInfos.removeDialogueInfos(record.mId);
                }
                else if (mTopics.tryDelete (record.mId))
                {
                    mTopicInfos.removeDialogueInfos(record.mId);
                }
                else
                {
                    messages.add (UniversalId::Type_None,
                        "Trying to delete dialogue record " + record.mId + " which does not exist",
                        "", CSMDoc::Message::Severity_Warning);
                }
            }
            else
            {
                if (record.mType == ESM3::Dialogue::Journal)
                {
                    mJournals.load (record, mBase);
                    mDialogue = &mJournals.getRecord (record.mId).get();
                }
                else
                {
                    mTopics.load (record, mBase);
                    mDialogue = &mTopics.getRecord (record.mId).get();
                }
            }

            break;
        }

        case ESM3::REC_INFO:
        {
            if (!mDialogue)
            {
                messages.add (UniversalId::Type_None,
                    "Found info record not following a dialogue record", "", CSMDoc::Message::Severity_Error);

                reader->skipRecordData();
                break;
            }

            if (mDialogue->mType==ESM3::Dialogue::Journal)
                mJournalInfos.load (*reader, mBase, *mDialogue);
            else
                mTopicInfos.load (*reader, mBase, *mDialogue);

            break;
        }

        case ESM3::REC_FILT:

            if (!mProject)
            {
                unhandledRecord = true;
                break;
            }

            mFilters.load (*reader, mBase);
            break;

        case ESM3::REC_DBGP:

            if (!mProject)
            {
                unhandledRecord = true;
                break;
            }

            mDebugProfiles.load (*reader, mBase);
            break;

        default:

            unhandledRecord = true;
    }

    if (unhandledRecord)
    {
        messages.add (UniversalId::Type_None, "Unsupported record type: " + ESM::printName(hdr.typeId), "",
            CSMDoc::Message::Severity_Error);

        reader->skipRecordData();
    }

    return false;
}

bool CSMWorld::Data::hasId (const std::string& id) const
{
    return
        getGlobals().searchId (id)!=-1 ||
        getGmsts().searchId (id)!=-1 ||
        getSkills().searchId (id)!=-1 ||
        getClasses().searchId (id)!=-1 ||
        getFactions().searchId (id)!=-1 ||
        getRaces().searchId (id)!=-1 ||
        getSounds().searchId (id)!=-1 ||
        getScripts().searchId (id)!=-1 ||
        getRegions().searchId (id)!=-1 ||
        getBirthsigns().searchId (id)!=-1 ||
        getSpells().searchId (id)!=-1 ||
        getTopics().searchId (id)!=-1 ||
        getJournals().searchId (id)!=-1 ||
        getCells().searchId (id)!=-1 ||
        getEnchantments().searchId (id)!=-1 ||
        getBodyParts().searchId (id)!=-1 ||
        getSoundGens().searchId (id)!=-1 ||
        getMagicEffects().searchId (id)!=-1 ||
        getReferenceables().searchId (id)!=-1;
}

int CSMWorld::Data::count (RecordBase::State state) const
{
    return
        count (state, mGlobals) +
        count (state, mGmsts) +
        count (state, mSkills) +
        count (state, mClasses) +
        count (state, mFactions) +
        count (state, mRaces) +
        count (state, mSounds) +
        count (state, mScripts) +
        count (state, mRegions) +
        count (state, mBirthsigns) +
        count (state, mSpells) +
        count (state, mCells) +
        count (state, mEnchantments) +
        count (state, mBodyParts) +
        count (state, mLand) +
        count (state, mLandTextures) +
        count (state, mSoundGens) +
        count (state, mMagicEffects) +
        count (state, mReferenceables) +
        count (state, mPathgrids);
}

std::vector<std::string> CSMWorld::Data::getIds (bool listDeleted) const
{
    std::vector<std::string> ids;

    appendIds (ids, mGlobals, listDeleted);
    appendIds (ids, mGmsts, listDeleted);
    appendIds (ids, mClasses, listDeleted);
    appendIds (ids, mFactions, listDeleted);
    appendIds (ids, mRaces, listDeleted);
    appendIds (ids, mSounds, listDeleted);
    appendIds (ids, mScripts, listDeleted);
    appendIds (ids, mRegions, listDeleted);
    appendIds (ids, mBirthsigns, listDeleted);
    appendIds (ids, mSpells, listDeleted);
    appendIds (ids, mTopics, listDeleted);
    appendIds (ids, mJournals, listDeleted);
    appendIds (ids, mCells, listDeleted);
    appendIds (ids, mEnchantments, listDeleted);
    appendIds (ids, mBodyParts, listDeleted);
    appendIds (ids, mSoundGens, listDeleted);
    appendIds (ids, mMagicEffects, listDeleted);
    appendIds (ids, mReferenceables, listDeleted);

    std::sort (ids.begin(), ids.end());

    return ids;
}

void CSMWorld::Data::assetsChanged()
{
    mVFS.get()->reset();
    VFS::registerArchives(mVFS.get(), Files::Collections(mDataPaths, !mFsStrict), mArchives, true);

    const UniversalId assetTableIds[] = {
        UniversalId::Type_Meshes,
        UniversalId::Type_Icons,
        UniversalId::Type_Musics,
        UniversalId::Type_SoundsRes,
        UniversalId::Type_Textures,
        UniversalId::Type_Videos
    };

    size_t numAssetTables = sizeof(assetTableIds) / sizeof(UniversalId);

    for (size_t i = 0; i < numAssetTables; ++i)
    {
        ResourceTable* table = static_cast<ResourceTable*>(getTableModel(assetTableIds[i]));
        table->beginReset();
    }

    // Trigger recreation
    mResourcesManager.recreateResources();

    for (size_t i = 0; i < numAssetTables; ++i)
    {
        ResourceTable* table = static_cast<ResourceTable*>(getTableModel(assetTableIds[i]));
        table->endReset();
    }

    // Get rid of potentially old cached assets
    mResourceSystem->clearCache();

    emit assetTablesChanged();
}

void CSMWorld::Data::dataChanged (const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
    if (topLeft.column()<=0)
        emit idListChanged();
}

void CSMWorld::Data::rowsChanged (const QModelIndex& parent, int start, int end)
{
    emit idListChanged();
}

const VFS::Manager* CSMWorld::Data::getVFS() const
{
    return mVFS.get();
}

bool CSMWorld::Data::loadTes4Group (CSMDoc::Messages& messages)
{
    ESM4::Reader& reader = static_cast<ESM4::Reader&>(*mReader);

    // check for EOF, sometimes there is a empty group at the end e.g. FONV DeadMoney.esm
    if (!reader.getRecordHeader() || !reader.hasMoreRecs())
        return false;

    const ESM4::RecordHeader& hdr = reader.hdr();

    if (hdr.record.typeId != ESM4::REC_GRUP)
        return loadTes4Record(hdr, messages);

    // Skip groups that are of no interest.  See also:
    // http://www.uesp.net/wiki/Tes4Mod:Mod_File_Format#Hierarchical_Top_Groups
    switch (hdr.group.type)
    {
        case ESM4::Grp_RecordType:
        {
            // FIXME: rewrite to workaround reliability issue
            if (hdr.group.label.value == ESM4::REC_NAVI || hdr.group.label.value == ESM4::REC_WRLD ||
                hdr.group.label.value == ESM4::REC_REGN || hdr.group.label.value == ESM4::REC_STAT ||
                hdr.group.label.value == ESM4::REC_ANIO || hdr.group.label.value == ESM4::REC_CONT ||
                hdr.group.label.value == ESM4::REC_MISC || hdr.group.label.value == ESM4::REC_ACTI ||
                hdr.group.label.value == ESM4::REC_ARMO || hdr.group.label.value == ESM4::REC_NPC_ ||
                hdr.group.label.value == ESM4::REC_FLOR || hdr.group.label.value == ESM4::REC_GRAS ||
                hdr.group.label.value == ESM4::REC_TREE || hdr.group.label.value == ESM4::REC_LIGH ||
                hdr.group.label.value == ESM4::REC_BOOK || hdr.group.label.value == ESM4::REC_FURN ||
                hdr.group.label.value == ESM4::REC_SOUN || hdr.group.label.value == ESM4::REC_WEAP ||
                hdr.group.label.value == ESM4::REC_DOOR || hdr.group.label.value == ESM4::REC_AMMO ||
                hdr.group.label.value == ESM4::REC_CLOT || hdr.group.label.value == ESM4::REC_ALCH ||
                hdr.group.label.value == ESM4::REC_APPA || hdr.group.label.value == ESM4::REC_INGR ||
                hdr.group.label.value == ESM4::REC_SGST || hdr.group.label.value == ESM4::REC_SLGM ||
                hdr.group.label.value == ESM4::REC_KEYM || hdr.group.label.value == ESM4::REC_HAIR ||
                hdr.group.label.value == ESM4::REC_EYES || hdr.group.label.value == ESM4::REC_CELL ||
                hdr.group.label.value == ESM4::REC_CREA || hdr.group.label.value == ESM4::REC_LVLC ||
                hdr.group.label.value == ESM4::REC_LVLI || hdr.group.label.value == ESM4::REC_MATO ||
                hdr.group.label.value == ESM4::REC_IDLE || hdr.group.label.value == ESM4::REC_LTEX
                )
            {
                //std::cout << "loading group... " << ESM4::printLabel(hdr.group.label, hdr.group.type) << std::endl;
                // NOTE: The label field of a group is not reliable.  See:
                // http://www.uesp.net/wiki/Tes4Mod:Mod_File_Format
                //
                // ASCII Q 0x51 0101 0001
                //       A 0x41 0100 0001
                //
                // Ignore flag  0000 1000 (i.e. probably unrelated)
                //
                // Workaround by getting the record header and checking its typeId
                reader.enterGroup();
                // FIXME: comment may no longer be releavant
                loadTes4Group(messages); // CELL group with record type may have sub-groups
            }
            else
            {
                //std::cout << "Skipping group... "  // FIXME: testing only
                    //<< ESM4::printLabel(hdr.group.label, hdr.group.type) << std::endl;

                reader.skipGroup();
                return false;
            }

            break;
        }
        case ESM4::Grp_CellChild:
        {
            reader.adjustGRUPFormId();  // not needed or even shouldn't be done? (only labels anyway)
            reader.enterGroup();
            if (!reader.hasMoreRecs())
                return false; // may have been an empty group followed by EOF

            loadTes4Group(messages);

            break;
        }
        case ESM4::Grp_WorldChild:
        case ESM4::Grp_TopicChild:
        // FIXME: need to save context if skipping
        case ESM4::Grp_CellPersistentChild:
        case ESM4::Grp_CellTemporaryChild:
        case ESM4::Grp_CellVisibleDistChild:
        {
            reader.adjustGRUPFormId();  // not needed or even shouldn't be done? (only labels anyway)
            reader.enterGroup();
            if (!reader.hasMoreRecs())
                return false; // may have been an empty group followed by EOF

            loadTes4Group(messages);

            break;
        }
        case ESM4::Grp_ExteriorCell:
        case ESM4::Grp_ExteriorSubCell:
        case ESM4::Grp_InteriorCell:
        case ESM4::Grp_InteriorSubCell:
        {
            reader.enterGroup();
            loadTes4Group(messages);

            break;
        }
        default:
            std::cout << "unknown group..." << std::endl; // FIXME
            reader.skipGroup();
            break;
    }

    return false;
}

// Deal with Tes4 records separately, as some have the same name as Tes3, e.g. REC_CELL
bool CSMWorld::Data::loadTes4Record (const ESM4::RecordHeader& hdr, CSMDoc::Messages& messages)
{
    ESM4::Reader& reader = static_cast<ESM4::Reader&>(*mReader);

    switch (hdr.record.typeId)
    {

        // FIXME: removed for now

        default:
            reader.skipRecordData();
    }

    return false;
}
