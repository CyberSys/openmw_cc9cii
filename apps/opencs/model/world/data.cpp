#include "data.hpp"

#include <stdexcept>
#include <algorithm>

#include <boost/filesystem.hpp>

#include <QAbstractItemModel>

#include <components/esm/esmreader.hpp>
#include <components/esm/esm4reader.hpp>
#include <components/esm/defs.hpp>
#include <components/esm/loadglob.hpp>
#include <components/esm/cellref.hpp>

#include <extern/esm4/common.hpp>

#include "../foreign/regionmap.hpp"

#include "idtable.hpp"
#include "idtree.hpp"
#include "columnimp.hpp"
#include "regionmap.hpp"
#include "columns.hpp"
#include "resourcesmanager.hpp"
#include "resourcetable.hpp"
#include "nestedcoladapterimp.hpp"
#include "npcautocalc.hpp"

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

CSMWorld::Data::Data (ToUTF8::FromType encoding, const ResourcesManager& resourcesManager)
: mEncoder (encoding), mPathgrids (mCells), mReferenceables(self()), mRefs (mCells),
  mForeignCells(mForeignWorlds), mForeignLands(mForeignCells), mForeignRefs(mForeignCells),
  mForeignChars(mForeignCells),
  mNavigation(mCells), mNavMesh(mCells),
  mResourcesManager (resourcesManager), mReader (0), mDialogue (0), mReaderIndex(0), mNpcAutoCalc (0)
{
    int index = 0;

    mGlobals.addColumn (new StringIdColumn<ESM::Global>);
    mGlobals.addColumn (new RecordStateColumn<ESM::Global>);
    mGlobals.addColumn (new FixedRecordTypeColumn<ESM::Global> (UniversalId::Type_Global));
    mGlobals.addColumn (new VarTypeColumn<ESM::Global> (ColumnBase::Display_GlobalVarType));
    mGlobals.addColumn (new VarValueColumn<ESM::Global>);

    mGmsts.addColumn (new StringIdColumn<ESM::GameSetting>);
    mGmsts.addColumn (new RecordStateColumn<ESM::GameSetting>);
    mGmsts.addColumn (new FixedRecordTypeColumn<ESM::GameSetting> (UniversalId::Type_Gmst));
    mGmsts.addColumn (new VarTypeColumn<ESM::GameSetting> (ColumnBase::Display_GmstVarType));
    mGmsts.addColumn (new VarValueColumn<ESM::GameSetting>);

    mSkills.addColumn (new StringIdColumn<ESM::Skill>);
    mSkills.addColumn (new RecordStateColumn<ESM::Skill>);
    mSkills.addColumn (new FixedRecordTypeColumn<ESM::Skill> (UniversalId::Type_Skill));
    mSkills.addColumn (new AttributeColumn<ESM::Skill>);
    mSkills.addColumn (new SpecialisationColumn<ESM::Skill>);
    for (int i=0; i<4; ++i)
        mSkills.addColumn (new UseValueColumn<ESM::Skill> (i));
    mSkills.addColumn (new DescriptionColumn<ESM::Skill>);

    mClasses.addColumn (new StringIdColumn<ESM::Class>);
    mClasses.addColumn (new RecordStateColumn<ESM::Class>);
    mClasses.addColumn (new FixedRecordTypeColumn<ESM::Class> (UniversalId::Type_Class));
    mClasses.addColumn (new NameColumn<ESM::Class>);
    mClasses.addColumn (new AttributesColumn<ESM::Class> (0));
    mClasses.addColumn (new AttributesColumn<ESM::Class> (1));
    mClasses.addColumn (new SpecialisationColumn<ESM::Class>);
    for (int i=0; i<5; ++i)
        mClasses.addColumn (new SkillsColumn<ESM::Class> (i, true, true));
    for (int i=0; i<5; ++i)
        mClasses.addColumn (new SkillsColumn<ESM::Class> (i, true, false));
    mClasses.addColumn (new PlayableColumn<ESM::Class>);
    mClasses.addColumn (new DescriptionColumn<ESM::Class>);

    mFactions.addColumn (new StringIdColumn<ESM::Faction>);
    mFactions.addColumn (new RecordStateColumn<ESM::Faction>);
    mFactions.addColumn (new FixedRecordTypeColumn<ESM::Faction> (UniversalId::Type_Faction));
    // The savegame format limits the player faction string to 32 characters.
    mFactions.addColumn (new NameColumn<ESM::Faction>(ColumnBase::Display_String32));
    mFactions.addColumn (new AttributesColumn<ESM::Faction> (0));
    mFactions.addColumn (new AttributesColumn<ESM::Faction> (1));
    mFactions.addColumn (new HiddenColumn<ESM::Faction>);
    for (int i=0; i<7; ++i)
        mFactions.addColumn (new SkillsColumn<ESM::Faction> (i));
    // Faction Reactions
    mFactions.addColumn (new NestedParentColumn<ESM::Faction> (Columns::ColumnId_FactionReactions));
    index = mFactions.getColumns()-1;
    mFactions.addAdapter (std::make_pair(&mFactions.getColumn(index), new FactionReactionsAdapter ()));
    // NAME32 enforced in IdCompletionDelegate::createEditor()
    mFactions.getNestableColumn(index)->addColumn(
        new NestedChildColumn (Columns::ColumnId_Faction, ColumnBase::Display_Faction));
    mFactions.getNestableColumn(index)->addColumn(
        new NestedChildColumn (Columns::ColumnId_FactionReaction, ColumnBase::Display_Integer));

    mRaces.addColumn (new StringIdColumn<ESM::Race>);
    mRaces.addColumn (new RecordStateColumn<ESM::Race>);
    mRaces.addColumn (new FixedRecordTypeColumn<ESM::Race> (UniversalId::Type_Race));
    mRaces.addColumn (new NameColumn<ESM::Race>);
    mRaces.addColumn (new DescriptionColumn<ESM::Race>);
    mRaces.addColumn (new FlagColumn<ESM::Race> (Columns::ColumnId_Playable, 0x1));
    mRaces.addColumn (new FlagColumn<ESM::Race> (Columns::ColumnId_BeastRace, 0x2));
    mRaces.addColumn (new WeightHeightColumn<ESM::Race> (true, true));
    mRaces.addColumn (new WeightHeightColumn<ESM::Race> (true, false));
    mRaces.addColumn (new WeightHeightColumn<ESM::Race> (false, true));
    mRaces.addColumn (new WeightHeightColumn<ESM::Race> (false, false));
    // Race spells
    mRaces.addColumn (new NestedParentColumn<ESM::Race> (Columns::ColumnId_PowerList));
    index = mRaces.getColumns()-1;
    mRaces.addAdapter (std::make_pair(&mRaces.getColumn(index), new SpellListAdapter<ESM::Race> ()));
    mRaces.getNestableColumn(index)->addColumn(
        new NestedChildColumn (Columns::ColumnId_SpellId, ColumnBase::Display_Spell));
    // Race attributes
    mRaces.addColumn (new NestedParentColumn<ESM::Race> (Columns::ColumnId_RaceAttributes,
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
    mRaces.addColumn (new NestedParentColumn<ESM::Race> (Columns::ColumnId_RaceSkillBonus,
        ColumnBase::Flag_Dialogue, true)); // fixed rows table
    index = mRaces.getColumns()-1;
    mRaces.addAdapter (std::make_pair(&mRaces.getColumn(index), new RaceSkillsBonusAdapter()));
    mRaces.getNestableColumn(index)->addColumn(
        new NestedChildColumn (Columns::ColumnId_Skill, ColumnBase::Display_SkillId));
    mRaces.getNestableColumn(index)->addColumn(
        new NestedChildColumn (Columns::ColumnId_RaceBonus, ColumnBase::Display_Integer));

    mSounds.addColumn (new StringIdColumn<ESM::Sound>);
    mSounds.addColumn (new RecordStateColumn<ESM::Sound>);
    mSounds.addColumn (new FixedRecordTypeColumn<ESM::Sound> (UniversalId::Type_Sound));
    mSounds.addColumn (new SoundParamColumn<ESM::Sound> (SoundParamColumn<ESM::Sound>::Type_Volume));
    mSounds.addColumn (new SoundParamColumn<ESM::Sound> (SoundParamColumn<ESM::Sound>::Type_MinRange));
    mSounds.addColumn (new SoundParamColumn<ESM::Sound> (SoundParamColumn<ESM::Sound>::Type_MaxRange));
    mSounds.addColumn (new SoundFileColumn<ESM::Sound>);

    mScripts.addColumn (new StringIdColumn<ESM::Script>);
    mScripts.addColumn (new RecordStateColumn<ESM::Script>);
    mScripts.addColumn (new FixedRecordTypeColumn<ESM::Script> (UniversalId::Type_Script));
    mScripts.addColumn (new ScriptColumn<ESM::Script> (ScriptColumn<ESM::Script>::Type_File));

    mRegions.addColumn (new StringIdColumn<ESM::Region>);
    mRegions.addColumn (new RecordStateColumn<ESM::Region>);
    mRegions.addColumn (new FixedRecordTypeColumn<ESM::Region> (UniversalId::Type_Region));
    mRegions.addColumn (new NameColumn<ESM::Region>);
    mRegions.addColumn (new MapColourColumn<ESM::Region>);
    mRegions.addColumn (new SleepListColumn<ESM::Region>);
    // Region Sounds
    mRegions.addColumn (new NestedParentColumn<ESM::Region> (Columns::ColumnId_RegionSounds));
    index = mRegions.getColumns()-1;
    mRegions.addAdapter (std::make_pair(&mRegions.getColumn(index), new RegionSoundListAdapter ()));
    mRegions.getNestableColumn(index)->addColumn(
    // NAME32 enforced in IdCompletionDelegate::createEditor()
        new NestedChildColumn (Columns::ColumnId_SoundName, ColumnBase::Display_Sound));
    mRegions.getNestableColumn(index)->addColumn(
        new NestedChildColumn (Columns::ColumnId_SoundChance, ColumnBase::Display_Integer));

    mBirthsigns.addColumn (new StringIdColumn<ESM::BirthSign>);
    mBirthsigns.addColumn (new RecordStateColumn<ESM::BirthSign>);
    mBirthsigns.addColumn (new FixedRecordTypeColumn<ESM::BirthSign> (UniversalId::Type_Birthsign));
    mBirthsigns.addColumn (new NameColumn<ESM::BirthSign>);
    mBirthsigns.addColumn (new TextureColumn<ESM::BirthSign>);
    mBirthsigns.addColumn (new DescriptionColumn<ESM::BirthSign>);
    // Birthsign spells
    mBirthsigns.addColumn (new NestedParentColumn<ESM::BirthSign> (Columns::ColumnId_PowerList));
    index = mBirthsigns.getColumns()-1;
    mBirthsigns.addAdapter (std::make_pair(&mBirthsigns.getColumn(index),
        new SpellListAdapter<ESM::BirthSign> ()));
    mBirthsigns.getNestableColumn(index)->addColumn(
        new NestedChildColumn (Columns::ColumnId_SpellId, ColumnBase::Display_Spell));

    mSpells.addColumn (new StringIdColumn<ESM::Spell>);
    mSpells.addColumn (new RecordStateColumn<ESM::Spell>);
    mSpells.addColumn (new FixedRecordTypeColumn<ESM::Spell> (UniversalId::Type_Spell));
    mSpells.addColumn (new NameColumn<ESM::Spell>);
    mSpells.addColumn (new SpellTypeColumn<ESM::Spell>); // ColumnId_SpellType
    mSpells.addColumn (new CostColumn<ESM::Spell>);
    mSpells.addColumn (new FlagColumn<ESM::Spell> (Columns::ColumnId_AutoCalc, 0x1));
    mSpells.addColumn (new FlagColumn<ESM::Spell> (Columns::ColumnId_StarterSpell, 0x2));
    mSpells.addColumn (new FlagColumn<ESM::Spell> (Columns::ColumnId_AlwaysSucceeds, 0x4));
    // Spell effects
    mSpells.addColumn (new NestedParentColumn<ESM::Spell> (Columns::ColumnId_EffectList));
    index = mSpells.getColumns()-1;
    mSpells.addAdapter (std::make_pair(&mSpells.getColumn(index), new EffectsListAdapter<ESM::Spell> ()));
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

    mTopics.addColumn (new StringIdColumn<ESM::Dialogue>);
    mTopics.addColumn (new RecordStateColumn<ESM::Dialogue>);
    mTopics.addColumn (new FixedRecordTypeColumn<ESM::Dialogue> (UniversalId::Type_Topic));
    mTopics.addColumn (new DialogueTypeColumn<ESM::Dialogue>);

    mJournals.addColumn (new StringIdColumn<ESM::Dialogue>);
    mJournals.addColumn (new RecordStateColumn<ESM::Dialogue>);
    mJournals.addColumn (new FixedRecordTypeColumn<ESM::Dialogue> (UniversalId::Type_Journal));
    mJournals.addColumn (new DialogueTypeColumn<ESM::Dialogue> (true));

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
        new NestedChildColumn (Columns::ColumnId_InfoCondVar, ColumnBase::Display_String));
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
    // NAME64 enforced in IdCompletionDelegate::createEditor()
    mCells.addColumn (new NameColumn<Cell>(ColumnBase::Display_String64));
    mCells.addColumn (new FlagColumn<Cell> (Columns::ColumnId_SleepForbidden, ESM::Cell::NoSleep));
    mCells.addColumn (new FlagColumn<Cell> (Columns::ColumnId_InteriorWater, ESM::Cell::HasWater,
        ColumnBase::Flag_Table | ColumnBase::Flag_Dialogue | ColumnBase::Flag_Dialogue_Refresh));
    mCells.addColumn (new FlagColumn<Cell> (Columns::ColumnId_InteriorSky, ESM::Cell::QuasiEx,
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
        new NestedChildColumn (Columns::ColumnId_Ambient, ColumnBase::Display_Integer));
    mCells.getNestableColumn(index)->addColumn(
        new NestedChildColumn (Columns::ColumnId_Sunlight, ColumnBase::Display_Integer));
    mCells.getNestableColumn(index)->addColumn(
        new NestedChildColumn (Columns::ColumnId_Fog, ColumnBase::Display_Integer));
    mCells.getNestableColumn(index)->addColumn(
        new NestedChildColumn (Columns::ColumnId_FogDensity, ColumnBase::Display_Float));
    mCells.getNestableColumn(index)->addColumn(
        new NestedChildColumn (Columns::ColumnId_WaterLevel, ColumnBase::Display_Float));
    mCells.getNestableColumn(index)->addColumn(
        new NestedChildColumn (Columns::ColumnId_MapColor, ColumnBase::Display_Integer));

    mEnchantments.addColumn (new StringIdColumn<ESM::Enchantment>);
    mEnchantments.addColumn (new RecordStateColumn<ESM::Enchantment>);
    mEnchantments.addColumn (new FixedRecordTypeColumn<ESM::Enchantment> (UniversalId::Type_Enchantment));
    mEnchantments.addColumn (new EnchantmentTypeColumn<ESM::Enchantment>);
    mEnchantments.addColumn (new CostColumn<ESM::Enchantment>);
    mEnchantments.addColumn (new ChargesColumn2<ESM::Enchantment>);
    mEnchantments.addColumn (new AutoCalcColumn<ESM::Enchantment>);
    // Enchantment effects
    mEnchantments.addColumn (new NestedParentColumn<ESM::Enchantment> (Columns::ColumnId_EffectList));
    index = mEnchantments.getColumns()-1;
    mEnchantments.addAdapter (std::make_pair(&mEnchantments.getColumn(index),
        new EffectsListAdapter<ESM::Enchantment> ()));
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

    mBodyParts.addColumn (new StringIdColumn<ESM::BodyPart>);
    mBodyParts.addColumn (new RecordStateColumn<ESM::BodyPart>);
    mBodyParts.addColumn (new FixedRecordTypeColumn<ESM::BodyPart> (UniversalId::Type_BodyPart));
    mBodyParts.addColumn (new BodyPartTypeColumn<ESM::BodyPart>);
    mBodyParts.addColumn (new VampireColumn<ESM::BodyPart>);
    mBodyParts.addColumn (new FlagColumn<ESM::BodyPart> (Columns::ColumnId_Female, ESM::BodyPart::BPF_Female));
    mBodyParts.addColumn (new FlagColumn<ESM::BodyPart> (Columns::ColumnId_Playable,
        ESM::BodyPart::BPF_NotPlayable, ColumnBase::Flag_Table | ColumnBase::Flag_Dialogue, true));

    int meshTypeFlags = ColumnBase::Flag_Table | ColumnBase::Flag_Dialogue | ColumnBase::Flag_Dialogue_Refresh;
    MeshTypeColumn<ESM::BodyPart> *meshTypeColumn = new MeshTypeColumn<ESM::BodyPart>(meshTypeFlags);
    mBodyParts.addColumn (meshTypeColumn);
    mBodyParts.addColumn (new ModelColumn<ESM::BodyPart>);
    mBodyParts.addColumn (new BodyPartRaceColumn(meshTypeColumn));

    mSoundGens.addColumn (new StringIdColumn<ESM::SoundGenerator>);
    mSoundGens.addColumn (new RecordStateColumn<ESM::SoundGenerator>);
    mSoundGens.addColumn (new FixedRecordTypeColumn<ESM::SoundGenerator> (UniversalId::Type_SoundGen));
    mSoundGens.addColumn (new CreatureColumn<ESM::SoundGenerator>);
    mSoundGens.addColumn (new SoundColumn<ESM::SoundGenerator>);
    mSoundGens.addColumn (new SoundGeneratorTypeColumn<ESM::SoundGenerator>);

    mMagicEffects.addColumn (new StringIdColumn<ESM::MagicEffect>);
    mMagicEffects.addColumn (new RecordStateColumn<ESM::MagicEffect>);
    mMagicEffects.addColumn (new FixedRecordTypeColumn<ESM::MagicEffect> (UniversalId::Type_MagicEffect));
    mMagicEffects.addColumn (new SchoolColumn<ESM::MagicEffect>);
    mMagicEffects.addColumn (new BaseCostColumn<ESM::MagicEffect>);
    mMagicEffects.addColumn (new EffectTextureColumn<ESM::MagicEffect> (Columns::ColumnId_Icon));
    mMagicEffects.addColumn (new EffectTextureColumn<ESM::MagicEffect> (Columns::ColumnId_Particle));
    mMagicEffects.addColumn (new EffectObjectColumn<ESM::MagicEffect> (Columns::ColumnId_CastingObject));
    mMagicEffects.addColumn (new EffectObjectColumn<ESM::MagicEffect> (Columns::ColumnId_HitObject));
    mMagicEffects.addColumn (new EffectObjectColumn<ESM::MagicEffect> (Columns::ColumnId_AreaObject));
    mMagicEffects.addColumn (new EffectObjectColumn<ESM::MagicEffect> (Columns::ColumnId_BoltObject));
    mMagicEffects.addColumn (new EffectSoundColumn<ESM::MagicEffect> (Columns::ColumnId_CastingSound));
    mMagicEffects.addColumn (new EffectSoundColumn<ESM::MagicEffect> (Columns::ColumnId_HitSound));
    mMagicEffects.addColumn (new EffectSoundColumn<ESM::MagicEffect> (Columns::ColumnId_AreaSound));
    mMagicEffects.addColumn (new EffectSoundColumn<ESM::MagicEffect> (Columns::ColumnId_BoltSound));
    mMagicEffects.addColumn (new FlagColumn<ESM::MagicEffect> (
        Columns::ColumnId_AllowSpellmaking, ESM::MagicEffect::AllowSpellmaking));
    mMagicEffects.addColumn (new FlagColumn<ESM::MagicEffect> (
        Columns::ColumnId_AllowEnchanting, ESM::MagicEffect::AllowEnchanting));
    mMagicEffects.addColumn (new FlagColumn<ESM::MagicEffect> (
        Columns::ColumnId_NegativeLight, ESM::MagicEffect::NegativeLight));
    mMagicEffects.addColumn (new DescriptionColumn<ESM::MagicEffect>);

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

    mStartScripts.addColumn (new StringIdColumn<ESM::StartScript>);
    mStartScripts.addColumn (new RecordStateColumn<ESM::StartScript>);
    mStartScripts.addColumn (new FixedRecordTypeColumn<ESM::StartScript> (UniversalId::Type_StartScript));

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

    mFilters.addColumn (new StringIdColumn<ESM::Filter>);
    mFilters.addColumn (new RecordStateColumn<ESM::Filter>);
    mFilters.addColumn (new FixedRecordTypeColumn<ESM::Filter> (UniversalId::Type_Filter));
    mFilters.addColumn (new FilterColumn<ESM::Filter>);
    mFilters.addColumn (new DescriptionColumn<ESM::Filter>);

    mDebugProfiles.addColumn (new StringIdColumn<ESM::DebugProfile>);
    mDebugProfiles.addColumn (new RecordStateColumn<ESM::DebugProfile>);
    mDebugProfiles.addColumn (new FixedRecordTypeColumn<ESM::DebugProfile> (UniversalId::Type_DebugProfile));
    mDebugProfiles.addColumn (new FlagColumn2<ESM::DebugProfile> (
        Columns::ColumnId_DefaultProfile, ESM::DebugProfile::Flag_Default));
    mDebugProfiles.addColumn (new FlagColumn2<ESM::DebugProfile> (
        Columns::ColumnId_BypassNewGame, ESM::DebugProfile::Flag_BypassNewGame));
    mDebugProfiles.addColumn (new FlagColumn2<ESM::DebugProfile> (
        Columns::ColumnId_GlobalProfile, ESM::DebugProfile::Flag_Global));
    mDebugProfiles.addColumn (new DescriptionColumn<ESM::DebugProfile>);
    mDebugProfiles.addColumn (new ScriptColumn<ESM::DebugProfile> (
        ScriptColumn<ESM::DebugProfile>::Type_Lines));

    mMetaData.appendBlankRecord ("sys::meta");

    mMetaData.addColumn (new StringIdColumn<MetaData> (true));
    mMetaData.addColumn (new RecordStateColumn<MetaData>);
    mMetaData.addColumn (new FixedRecordTypeColumn<MetaData> (UniversalId::Type_MetaData));
    mMetaData.addColumn (new FormatColumn<MetaData>);
    mMetaData.addColumn (new AuthorColumn<MetaData>);
    mMetaData.addColumn (new FileDescriptionColumn<MetaData>);

    mLandTextures.addColumn (new StringIdColumn<LandTexture>);
    mLandTextures.addColumn (new RecordStateColumn<LandTexture>);
    mLandTextures.addColumn (new FixedRecordTypeColumn<LandTexture> (UniversalId::Type_LandTexture));

    mLand.addColumn (new StringIdColumn<Land>);
    mLand.addColumn (new RecordStateColumn<Land>);
    mLand.addColumn (new FixedRecordTypeColumn<Land> (UniversalId::Type_Land));

    mForeignWorlds.addColumn (new StringIdColumn<CSMForeign::World>);
    mForeignWorlds.addColumn (new RecordStateColumn<CSMForeign::World>);
    mForeignWorlds.addColumn (new FixedRecordTypeColumn<CSMForeign::World> (UniversalId::Type_ForeignWorld));
    mForeignWorlds.addColumn (new NameColumn<CSMForeign::World>);
    mForeignWorlds.addColumn (new EditorIdColumn<CSMForeign::World>);

    mForeignRegions.addColumn (new StringIdColumn<CSMForeign::Region>);
    mForeignRegions.addColumn (new RecordStateColumn<CSMForeign::Region>);
    mForeignRegions.addColumn (new FixedRecordTypeColumn<CSMForeign::Region> (UniversalId::Type_ForeignRegion));
    mForeignRegions.addColumn (new EditorIdColumn<CSMForeign::Region>);
    mForeignRegions.addColumn (new WorldColumn<CSMForeign::Region>);
    mForeignRegions.addColumn (new MapNameColumn<CSMForeign::Region>);
    mForeignRegions.addColumn (new ShaderColumn<CSMForeign::Region>);
    mForeignRegions.addColumn (new MapColourColumn<CSMForeign::Region>);

    mForeignCells.addColumn (new StringIdColumn<CSMForeign::Cell>/*(true)*/);
    mForeignCells.addColumn (new RecordStateColumn<CSMForeign::Cell>);
    mForeignCells.addColumn (new FixedRecordTypeColumn<CSMForeign::Cell> (UniversalId::Type_ForeignCell));
    mForeignCells.addColumn (new EditorIdColumn<CSMForeign::Cell>);
    mForeignCells.addColumn (new FullNameColumn<CSMForeign::Cell>);
    mForeignCells.addColumn (new CellIdColumn<CSMForeign::Cell>);
    mForeignCells.addColumn (new WorldColumn<CSMForeign::Cell>);
#if 0
    mForeignCells.addColumn (new FlagColumn<Cell> (Columns::ColumnId_InteriorWater, ESM::Cell::HasWater,
        ColumnBase::Flag_Table | ColumnBase::Flag_Dialogue | ColumnBase::Flag_Dialogue_Refresh));
    mForeignCells.addColumn (new FlagColumn<Cell> (Columns::ColumnId_InteriorSky, ESM::Cell::QuasiEx,
        ColumnBase::Flag_Table | ColumnBase::Flag_Dialogue | ColumnBase::Flag_Dialogue_Refresh));
#endif

    mForeignLandTextures.addColumn (new StringIdColumn<CSMForeign::LandTexture>);
    mForeignLandTextures.addColumn (new RecordStateColumn<CSMForeign::LandTexture>);
    mForeignLandTextures.addColumn (
             new FixedRecordTypeColumn<CSMForeign::LandTexture> (UniversalId::Type_ForeignLandTexture));
    mForeignLandTextures.addColumn (new EditorIdColumn<CSMForeign::LandTexture>);
    mForeignLandTextures.addColumn (new TextureFileColumn<CSMForeign::LandTexture>);

    mForeignLands.addColumn (new StringIdColumn<CSMForeign::Land>);
    mForeignLands.addColumn (new RecordStateColumn<CSMForeign::Land>);
    mForeignLands.addColumn (new FixedRecordTypeColumn<CSMForeign::Land> (UniversalId::Type_ForeignLand));
    mForeignLands.addColumn (new CellIdColumn<CSMForeign::Land>);

    // FIXME: delete once refidcollection is available
    mForeignStatics.addColumn (new StringIdColumn<CSMForeign::Static>);
    mForeignStatics.addColumn (new RecordStateColumn<CSMForeign::Static>);
    mForeignStatics.addColumn (new FixedRecordTypeColumn<CSMForeign::Static> (UniversalId::Type_ForeignStatic));
    mForeignStatics.addColumn (new ModelColumn<CSMForeign::Static>);

    mForeignAnimObjs.addColumn (new StringIdColumn<CSMForeign::AnimObject>);
    mForeignAnimObjs.addColumn (new RecordStateColumn<CSMForeign::AnimObject>);
    mForeignAnimObjs.addColumn (new FixedRecordTypeColumn<CSMForeign::AnimObject> (UniversalId::Type_ForeignAnimObj));
    mForeignAnimObjs.addColumn (new ModelColumn<CSMForeign::AnimObject>);

    mForeignContainers.addColumn (new StringIdColumn<CSMForeign::Container>);
    mForeignContainers.addColumn (new RecordStateColumn<CSMForeign::Container>);
    mForeignContainers.addColumn (new FixedRecordTypeColumn<CSMForeign::Container> (UniversalId::Type_ForeignContainer));
    mForeignContainers.addColumn (new FullNameColumn<CSMForeign::Container>);
    mForeignContainers.addColumn (new ModelColumn<CSMForeign::Container>);

    mForeignMiscItems.addColumn (new StringIdColumn<CSMForeign::MiscItem>);
    mForeignMiscItems.addColumn (new RecordStateColumn<CSMForeign::MiscItem>);
    mForeignMiscItems.addColumn (new FixedRecordTypeColumn<CSMForeign::MiscItem> (UniversalId::Type_ForeignMiscItem));
    mForeignMiscItems.addColumn (new FullNameColumn<CSMForeign::MiscItem>);
    mForeignMiscItems.addColumn (new ModelColumn<CSMForeign::MiscItem>);

    mForeignActivators.addColumn (new StringIdColumn<CSMForeign::Activator>);
    mForeignActivators.addColumn (new RecordStateColumn<CSMForeign::Activator>);
    mForeignActivators.addColumn (new FixedRecordTypeColumn<CSMForeign::Activator> (UniversalId::Type_ForeignActivator));
    mForeignActivators.addColumn (new FullNameColumn<CSMForeign::Activator>);
    mForeignActivators.addColumn (new ModelColumn<CSMForeign::Activator>);

    mForeignArmors.addColumn (new StringIdColumn<CSMForeign::Armor>);
    mForeignArmors.addColumn (new RecordStateColumn<CSMForeign::Armor>);
    mForeignArmors.addColumn (new FixedRecordTypeColumn<CSMForeign::Armor> (UniversalId::Type_ForeignArmor));
    mForeignArmors.addColumn (new FullNameColumn<CSMForeign::Armor>);
    mForeignArmors.addColumn (new ModelColumn<CSMForeign::Armor>);

    mForeignNpcs.addColumn (new StringIdColumn<CSMForeign::Npc>);
    mForeignNpcs.addColumn (new RecordStateColumn<CSMForeign::Npc>);
    mForeignNpcs.addColumn (new FixedRecordTypeColumn<CSMForeign::Npc> (UniversalId::Type_ForeignNpc));
    mForeignNpcs.addColumn (new FullNameColumn<CSMForeign::Npc>);
    mForeignNpcs.addColumn (new ModelColumn<CSMForeign::Npc>);

    mForeignFloras.addColumn (new StringIdColumn<CSMForeign::Flora>);
    mForeignFloras.addColumn (new RecordStateColumn<CSMForeign::Flora>);
    mForeignFloras.addColumn (new FixedRecordTypeColumn<CSMForeign::Flora> (UniversalId::Type_ForeignFloras));
    mForeignFloras.addColumn (new FullNameColumn<CSMForeign::Flora>);
    mForeignFloras.addColumn (new ModelColumn<CSMForeign::Flora>);

    mForeignGrasses.addColumn (new StringIdColumn<CSMForeign::Grass>);
    mForeignGrasses.addColumn (new RecordStateColumn<CSMForeign::Grass>);
    mForeignGrasses.addColumn (new FixedRecordTypeColumn<CSMForeign::Grass> (UniversalId::Type_ForeignGrasses));
    mForeignGrasses.addColumn (new ModelColumn<CSMForeign::Grass>);

    mForeignTrees.addColumn (new StringIdColumn<CSMForeign::Tree>);
    mForeignTrees.addColumn (new RecordStateColumn<CSMForeign::Tree>);
    mForeignTrees.addColumn (new FixedRecordTypeColumn<CSMForeign::Tree> (UniversalId::Type_ForeignTrees));
    mForeignTrees.addColumn (new ModelColumn<CSMForeign::Tree>);

    mForeignLights.addColumn (new StringIdColumn<CSMForeign::Light>);
    mForeignLights.addColumn (new RecordStateColumn<CSMForeign::Light>);
    mForeignLights.addColumn (new FixedRecordTypeColumn<CSMForeign::Light> (UniversalId::Type_ForeignLights));
    mForeignLights.addColumn (new ModelColumn<CSMForeign::Light>);

    mForeignRefs.addColumn (new StringIdColumn<CSMForeign::CellRef>/*(true)*/);
    mForeignRefs.addColumn (new RecordStateColumn<CSMForeign::CellRef>);
    mForeignRefs.addColumn (new FixedRecordTypeColumn<CSMForeign::CellRef> (UniversalId::Type_ForeignReference));
    mForeignRefs.addColumn (new EditorIdColumn<CSMForeign::CellRef>);
    mForeignRefs.addColumn (new FullNameColumn<CSMForeign::CellRef>);
    mForeignRefs.addColumn (new CellColumn<CSMForeign::CellRef> (true));
    //mForeignRefs.addColumn (new OriginalCellColumn<CSMForeign::CellRef>);
    mForeignRefs.addColumn (new IdColumn<CSMForeign::CellRef>); // mRefID
    mForeignRefs.addColumn (new PosColumn<CSMForeign::CellRef> (&CSMForeign::CellRef::mPos, 0, false));
    mForeignRefs.addColumn (new PosColumn<CSMForeign::CellRef> (&CSMForeign::CellRef::mPos, 1, false));
    mForeignRefs.addColumn (new PosColumn<CSMForeign::CellRef> (&CSMForeign::CellRef::mPos, 2, false));
    mForeignRefs.addColumn (new RotColumn<CSMForeign::CellRef> (&CSMForeign::CellRef::mPos, 0, false));
    mForeignRefs.addColumn (new RotColumn<CSMForeign::CellRef> (&CSMForeign::CellRef::mPos, 1, false));
    mForeignRefs.addColumn (new RotColumn<CSMForeign::CellRef> (&CSMForeign::CellRef::mPos, 2, false));
    mForeignRefs.addColumn (new ScaleColumn<CSMForeign::CellRef>);

    // FIXME: duplication
    mForeignChars.addColumn (new StringIdColumn<CSMForeign::CellChar>/*(true)*/);
    mForeignChars.addColumn (new RecordStateColumn<CSMForeign::CellChar>);
    mForeignChars.addColumn (new FixedRecordTypeColumn<CSMForeign::CellChar> (UniversalId::Type_ForeignReference));
    mForeignChars.addColumn (new EditorIdColumn<CSMForeign::CellChar>);
    mForeignChars.addColumn (new CellColumn<CSMForeign::CellChar> (true));
    //mForeignChars.addColumn (new OriginalCellColumn<CSMForeign::CellChar>);
    mForeignChars.addColumn (new IdColumn<CSMForeign::CellChar>); // mRefID
    mForeignChars.addColumn (new PosColumn<CSMForeign::CellChar> (&CSMForeign::CellChar::mPos, 0, false));
    mForeignChars.addColumn (new PosColumn<CSMForeign::CellChar> (&CSMForeign::CellChar::mPos, 1, false));
    mForeignChars.addColumn (new PosColumn<CSMForeign::CellChar> (&CSMForeign::CellChar::mPos, 2, false));
    mForeignChars.addColumn (new RotColumn<CSMForeign::CellChar> (&CSMForeign::CellChar::mPos, 0, false));
    mForeignChars.addColumn (new RotColumn<CSMForeign::CellChar> (&CSMForeign::CellChar::mPos, 1, false));
    mForeignChars.addColumn (new RotColumn<CSMForeign::CellChar> (&CSMForeign::CellChar::mPos, 2, false));
    mForeignChars.addColumn (new ScaleColumn<CSMForeign::CellChar>);

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
    addModel (new IdTable (&mLand), UniversalId::Type_Land);
    addModel (new IdTable (&mLandTextures), UniversalId::Type_LandTexture);
    addModel (new IdTable (&mForeignWorlds), UniversalId::Type_ForeignWorld);
    addModel (new IdTable (&mForeignRegions), UniversalId::Type_ForeignRegion);
    addModel (new IdTable (&mForeignCells), UniversalId::Type_ForeignCell);
    addModel (new IdTable (&mForeignLandTextures), UniversalId::Type_ForeignLandTexture);
    addModel (new IdTable (&mForeignLands), UniversalId::Type_ForeignLand);
    addModel (new IdTable (&mForeignStatics), UniversalId::Type_ForeignStatic); // FIXME: temp, should be refid
    addModel (new IdTable (&mForeignAnimObjs), UniversalId::Type_ForeignAnimObj); // FIXME: temp, should be refid
    addModel (new IdTable (&mForeignContainers), UniversalId::Type_ForeignContainer); // FIXME: temp, should be refid
    addModel (new IdTable (&mForeignMiscItems), UniversalId::Type_ForeignMiscItem); // FIXME: temp, should be refid
    addModel (new IdTable (&mForeignActivators), UniversalId::Type_ForeignActivator); // FIXME: temp, should be refid
    addModel (new IdTable (&mForeignArmors), UniversalId::Type_ForeignArmor); // FIXME: temp, should be refid
    addModel (new IdTable (&mForeignNpcs), UniversalId::Type_ForeignNpc); // FIXME: temp, should be refid
    addModel (new IdTable (&mForeignFloras), UniversalId::Type_ForeignFlora); // FIXME: temp, should be refid
    addModel (new IdTable (&mForeignGrasses), UniversalId::Type_ForeignGrass); // FIXME: temp, should be refid
    addModel (new IdTable (&mForeignTrees), UniversalId::Type_ForeignTree); // FIXME: temp, should be refid
    addModel (new IdTable (&mForeignLights), UniversalId::Type_ForeignLight); // FIXME: temp, should be refid
    addModel (new IdTable (&mForeignRefs, IdTable::Feature_ViewCell | IdTable::Feature_Preview),
            UniversalId::Type_ForeignReference);
    addModel (new IdTable (&mForeignChars, IdTable::Feature_ViewCell | IdTable::Feature_Preview),
            UniversalId::Type_ForeignChar);

    // for autocalc updates when gmst/race/class/skils tables change
    CSMWorld::IdTable *gmsts =
        static_cast<CSMWorld::IdTable*>(getTableModel(UniversalId::Type_Gmst));
    CSMWorld::IdTable *skills =
        static_cast<CSMWorld::IdTable*>(getTableModel(UniversalId::Type_Skill));
    CSMWorld::IdTable *classes =
        static_cast<CSMWorld::IdTable*>(getTableModel(UniversalId::Type_Class));
    CSMWorld::IdTree *races =
        static_cast<CSMWorld::IdTree*>(getTableModel(UniversalId::Type_Race));
    CSMWorld::IdTree *objects =
        static_cast<CSMWorld::IdTree*>(getTableModel(UniversalId::Type_Referenceable));

    mNpcAutoCalc = new NpcAutoCalc (self(), gmsts, skills, classes, races, objects);

    mRefLoadCache.clear(); // clear here rather than startLoading() and continueLoading() for multiple content files
}

CSMWorld::Data::~Data()
{
    for (std::vector<QAbstractItemModel *>::iterator iter (mModels.begin()); iter!=mModels.end(); ++iter)
        delete *iter;

    delete mReader;
    delete mNpcAutoCalc;
}

const CSMWorld::IdCollection<ESM::Global>& CSMWorld::Data::getGlobals() const
{
    return mGlobals;
}

CSMWorld::IdCollection<ESM::Global>& CSMWorld::Data::getGlobals()
{
    return mGlobals;
}

const CSMWorld::IdCollection<ESM::GameSetting>& CSMWorld::Data::getGmsts() const
{
    return mGmsts;
}

CSMWorld::IdCollection<ESM::GameSetting>& CSMWorld::Data::getGmsts()
{
    return mGmsts;
}

const CSMWorld::IdCollection<ESM::Skill>& CSMWorld::Data::getSkills() const
{
    return mSkills;
}

CSMWorld::IdCollection<ESM::Skill>& CSMWorld::Data::getSkills()
{
    return mSkills;
}

const CSMWorld::IdCollection<ESM::Class>& CSMWorld::Data::getClasses() const
{
    return mClasses;
}

CSMWorld::IdCollection<ESM::Class>& CSMWorld::Data::getClasses()
{
    return mClasses;
}

const CSMWorld::IdCollection<ESM::Faction>& CSMWorld::Data::getFactions() const
{
    return mFactions;
}

CSMWorld::IdCollection<ESM::Faction>& CSMWorld::Data::getFactions()
{
    return mFactions;
}

const CSMWorld::IdCollection<ESM::Race>& CSMWorld::Data::getRaces() const
{
    return mRaces;
}

CSMWorld::IdCollection<ESM::Race>& CSMWorld::Data::getRaces()
{
    return mRaces;
}

const CSMWorld::IdCollection<ESM::Sound>& CSMWorld::Data::getSounds() const
{
    return mSounds;
}

CSMWorld::IdCollection<ESM::Sound>& CSMWorld::Data::getSounds()
{
    return mSounds;
}

const CSMWorld::IdCollection<ESM::Script>& CSMWorld::Data::getScripts() const
{
    return mScripts;
}

CSMWorld::IdCollection<ESM::Script>& CSMWorld::Data::getScripts()
{
    return mScripts;
}

const CSMWorld::IdCollection<ESM::Region>& CSMWorld::Data::getRegions() const
{
    return mRegions;
}

CSMWorld::IdCollection<ESM::Region>& CSMWorld::Data::getRegions()
{
    return mRegions;
}

const CSMWorld::IdCollection<ESM::BirthSign>& CSMWorld::Data::getBirthsigns() const
{
    return mBirthsigns;
}

CSMWorld::IdCollection<ESM::BirthSign>& CSMWorld::Data::getBirthsigns()
{
    return mBirthsigns;
}

const CSMWorld::IdCollection<ESM::Spell>& CSMWorld::Data::getSpells() const
{
    return mSpells;
}

CSMWorld::IdCollection<ESM::Spell>& CSMWorld::Data::getSpells()
{
    return mSpells;
}


const CSMWorld::IdCollection<ESM::Dialogue>& CSMWorld::Data::getTopics() const
{
    return mTopics;
}

CSMWorld::IdCollection<ESM::Dialogue>& CSMWorld::Data::getTopics()
{
    return mTopics;
}

const CSMWorld::IdCollection<ESM::Dialogue>& CSMWorld::Data::getJournals() const
{
    return mJournals;
}

CSMWorld::IdCollection<ESM::Dialogue>& CSMWorld::Data::getJournals()
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

const CSMWorld::IdCollection<ESM::Filter>& CSMWorld::Data::getFilters() const
{
    return mFilters;
}

CSMWorld::IdCollection<ESM::Filter>& CSMWorld::Data::getFilters()
{
    return mFilters;
}

const CSMWorld::IdCollection<ESM::Enchantment>& CSMWorld::Data::getEnchantments() const
{
    return mEnchantments;
}

CSMWorld::IdCollection<ESM::Enchantment>& CSMWorld::Data::getEnchantments()
{
    return mEnchantments;
}

const CSMWorld::IdCollection<ESM::BodyPart>& CSMWorld::Data::getBodyParts() const
{
    return mBodyParts;
}

CSMWorld::IdCollection<ESM::BodyPart>& CSMWorld::Data::getBodyParts()
{
    return mBodyParts;
}

const CSMWorld::IdCollection<ESM::DebugProfile>& CSMWorld::Data::getDebugProfiles() const
{
    return mDebugProfiles;
}

CSMWorld::IdCollection<ESM::DebugProfile>& CSMWorld::Data::getDebugProfiles()
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

const CSMWorld::IdCollection<ESM::SoundGenerator>& CSMWorld::Data::getSoundGens() const
{
    return mSoundGens;
}

CSMWorld::IdCollection<ESM::SoundGenerator>& CSMWorld::Data::getSoundGens()
{
    return mSoundGens;
}

const CSMWorld::IdCollection<ESM::MagicEffect>& CSMWorld::Data::getMagicEffects() const
{
    return mMagicEffects;
}

CSMWorld::IdCollection<ESM::MagicEffect>& CSMWorld::Data::getMagicEffects()
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

const CSMWorld::IdCollection<ESM::StartScript>& CSMWorld::Data::getStartScripts() const
{
    return mStartScripts;
}

CSMWorld::IdCollection<ESM::StartScript>& CSMWorld::Data::getStartScripts()
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
            Record<MetaData>(RecordBase::State_ModifiedOnly, 0, &metaData)));
}

const CSMWorld::NpcAutoCalc& CSMWorld::Data::getNpcAutoCalc() const
{
    return *mNpcAutoCalc;
}

const CSMForeign::NavigationCollection& CSMWorld::Data::getNavigation() const
{
    return mNavigation;
}

CSMForeign::NavigationCollection& CSMWorld::Data::getNavigation()
{
    return mNavigation;
}

const CSMForeign::NavMeshCollection& CSMWorld::Data::getNavMeshes() const
{
    return mNavMesh;
}

CSMForeign::NavMeshCollection& CSMWorld::Data::getNavMeshes()
{
    return mNavMesh;
}

const CSMForeign::WorldCollection& CSMWorld::Data::getForeignWorlds() const
{
    return mForeignWorlds;
}

CSMForeign::WorldCollection& CSMWorld::Data::getForeignWorlds()
{
    return mForeignWorlds;
}

const CSMForeign::RegionCollection& CSMWorld::Data::getForeignRegions() const
{
    return mForeignRegions;
}

CSMForeign::RegionCollection& CSMWorld::Data::getForeignRegions()
{
    return mForeignRegions;
}

const CSMForeign::CellCollection& CSMWorld::Data::getForeignCells() const
{
    return mForeignCells;
}

CSMForeign::CellCollection& CSMWorld::Data::getForeignCells()
{
    return mForeignCells;
}

const CSMForeign::LandTextureCollection& CSMWorld::Data::getForeignLandTextures() const
{
    return mForeignLandTextures;
}

CSMForeign::LandTextureCollection& CSMWorld::Data::getForeignLandTextures()
{
    return mForeignLandTextures;
}

const CSMForeign::LandCollection& CSMWorld::Data::getForeignLands() const
{
    return mForeignLands;
}

CSMForeign::LandCollection& CSMWorld::Data::getForeignLands()
{
    return mForeignLands;
}

const CSMForeign::RefCollection& CSMWorld::Data::getForeignReferences() const
{
    return mForeignRefs;
}

CSMForeign::RefCollection& CSMWorld::Data::getForeignReferences()
{
    return mForeignRefs;
}

const CSMForeign::CharCollection& CSMWorld::Data::getForeignChars() const
{
    return mForeignChars;
}

CSMForeign::CharCollection& CSMWorld::Data::getForeignChars()
{
    return mForeignChars;
}

const CSMForeign::StaticCollection& CSMWorld::Data::getForeignStatics() const
{
    return mForeignStatics;
}

CSMForeign::StaticCollection& CSMWorld::Data::getForeignStatics()
{
    return mForeignStatics;
}

const CSMForeign::IdCollection<CSMForeign::AnimObject>& CSMWorld::Data::getForeignAnimObjs() const
{
    return mForeignAnimObjs;
}

CSMForeign::IdCollection<CSMForeign::AnimObject>& CSMWorld::Data::getForeignAnimObjs()
{
    return mForeignAnimObjs;
}

const CSMForeign::IdCollection<CSMForeign::Container>& CSMWorld::Data::getForeignContainers() const
{
    return mForeignContainers;
}

CSMForeign::IdCollection<CSMForeign::Container>& CSMWorld::Data::getForeignContainers()
{
    return mForeignContainers;
}

const CSMForeign::IdCollection<CSMForeign::MiscItem>& CSMWorld::Data::getForeignMiscItems() const
{
    return mForeignMiscItems;
}

CSMForeign::IdCollection<CSMForeign::MiscItem>& CSMWorld::Data::getForeignMiscItems()
{
    return mForeignMiscItems;
}

const CSMForeign::IdCollection<CSMForeign::Activator>& CSMWorld::Data::getForeignActivators() const
{
    return mForeignActivators;
}

CSMForeign::IdCollection<CSMForeign::Activator>& CSMWorld::Data::getForeignActivators()
{
    return mForeignActivators;
}

const CSMForeign::IdCollection<CSMForeign::Armor>& CSMWorld::Data::getForeignArmors() const
{
    return mForeignArmors;
}

CSMForeign::IdCollection<CSMForeign::Armor>& CSMWorld::Data::getForeignArmors()
{
    return mForeignArmors;
}

const CSMForeign::IdCollection<CSMForeign::Npc>& CSMWorld::Data::getForeignNpcs() const
{
    return mForeignNpcs;
}

CSMForeign::IdCollection<CSMForeign::Npc>& CSMWorld::Data::getForeignNpcs()
{
    return mForeignNpcs;
}

const CSMForeign::IdCollection<CSMForeign::Flora>& CSMWorld::Data::getForeignFloras() const
{
    return mForeignFloras;
}

CSMForeign::IdCollection<CSMForeign::Flora>& CSMWorld::Data::getForeignFloras()
{
    return mForeignFloras;
}

const CSMForeign::IdCollection<CSMForeign::Grass>& CSMWorld::Data::getForeignGrasses() const
{
    return mForeignGrasses;
}

CSMForeign::IdCollection<CSMForeign::Grass>& CSMWorld::Data::getForeignGrasses()
{
    return mForeignGrasses;
}

const CSMForeign::IdCollection<CSMForeign::Tree>& CSMWorld::Data::getForeignTrees() const
{
    return mForeignTrees;
}

CSMForeign::IdCollection<CSMForeign::Tree>& CSMWorld::Data::getForeignTrees()
{
    return mForeignTrees;
}

const CSMForeign::IdCollection<CSMForeign::Light>& CSMWorld::Data::getForeignLights() const
{
    return mForeignLights;
}

CSMForeign::IdCollection<CSMForeign::Light>& CSMWorld::Data::getForeignLights()
{
    return mForeignLights;
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
            RegionMap *table = 0;
            addModel (table = new RegionMap (*this), UniversalId::Type_RegionMap, false);
            return table;
        }
        else if (id.getType()==UniversalId::Type_ForeignRegionMap)
        {
            CSMForeign::RegionMap *table = 0;
            addModel (table = new CSMForeign::RegionMap (*this), UniversalId::Type_ForeignRegionMap, false);
            return table;
        }
        throw std::logic_error ("No table model available for " + id.toString());
    }

    return iter->second;
}

void CSMWorld::Data::merge()
{
    mGlobals.merge();
}

int CSMWorld::Data::getTotalRecords (const std::vector<boost::filesystem::path>& files)
{
    int records = 0;

    std::unique_ptr<ESM::ESMReader> reader = std::unique_ptr<ESM::ESMReader>(new ESM::ESMReader);

    for (unsigned int i = 0; i < files.size(); ++i)
    {
        if (boost::filesystem::exists(files[i].string()))
        {
            reader->open(files[i].string());
            records += reader->getRecordCount();
            reader->close();
        }
    }

    return records;
}

int CSMWorld::Data::startLoading (const boost::filesystem::path& path, bool base, bool project)
{
    // Don't delete the Reader yet. Some record types store a reference to the Reader to handle on-demand loading
    boost::shared_ptr<ESM::ESMReader> ptr(mReader);
    mReaders.push_back(ptr);
    mReader = 0;

    mDialogue = 0;

    mReader = new ESM::ESMReader;
    mReader->setEncoder (&mEncoder);
    mReader->setIndex(mReaderIndex++);
    mReader->open (path.string());
    if (mReader->getVer() == ESM::VER_080 // TES4
        || mReader->getVer() == ESM::VER_094 || mReader->getVer() == ESM::VER_17) // TES5
    {
        delete mReader;
        mReader = new ESM::ESM4Reader(mReader->getVer() == ESM::VER_080);
        mReader->setEncoder(&mEncoder);
        mReader->setIndex(mReaderIndex); // use the same index
        static_cast<ESM::ESM4Reader*>(mReader)->openTes4File(path.string());
    }
    mLoadedFiles.push_back(path.filename().string());

    // at this point mReader->mHeader.mMaster have been populated for the file being loaded
    for (size_t f = 0; f < mReader->getGameFiles().size(); ++f)
    {
        ESM::Header::MasterData& m = const_cast<ESM::Header::MasterData&>(mReader->getGameFiles().at(f));

        int index = -1;
        for (size_t i = 0; i < mLoadedFiles.size()-1; ++i) // -1 to ignore the current file
        {
            if (Misc::StringUtils::ciEqual(m.name, mLoadedFiles.at(i)))
            {
                index = static_cast<int>(i);
                break;
            }
        }

        if (index == -1)
        {
            // Tried to load a parent file that has not been loaded yet. This is bad,
            //  the launcher should have taken care of this.
            std::string fstring = "File " + mReader->getName() + " asks for parent file " + m.name
                + ", but it has not been loaded yet. Please check your load order.";
            mReader->fail(fstring);
        }

        m.index = index;
    }

    mBase = base;
    mProject = project;

    if (!mProject && !mBase)
    {
        MetaData metaData;
        metaData.mId = "sys::meta";
        metaData.load (*mReader);

        mMetaData.setRecord (0, std::make_unique<Record<MetaData> >(
                    Record<MetaData> (RecordBase::State_ModifiedOnly, 0, &metaData)));
    }

    return mReader->getRecordCount();
}

bool CSMWorld::Data::continueLoading (CSMDoc::Messages& messages)
{
    if (!mReader)
        throw std::logic_error ("can't continue loading, because no load has been started");

    // Check if previous record/group was the final one in this group.  Must be done before
    // calling mReader->hasMoreRecs() below, because all records may have been processed when
    // the previous group is popped off the stack.
    if (mReader->getVer() == ESM::VER_080 // TES4
            || mReader->getVer() == ESM::VER_094 || mReader->getVer() == ESM::VER_17) // TES5
        static_cast<ESM::ESM4Reader*>(mReader)->reader().checkGroupStatus();

    if (!mReader->hasMoreRecs())
    {
        if (mBase)
        {
            // Don't delete the Reader yet. Some record types store a reference to the Reader to handle on-demand loading.
            // We don't store non-base reader, because everything going into modified will be
            // fully loaded during the initial loading process.
            boost::shared_ptr<ESM::ESMReader> ptr(mReader);
            mReaders.push_back(ptr);
        }
        else
            delete mReader;

        mReader = 0;

        mDialogue = 0;

        // FIXME: runs once per each document!
        // check the number of docs and only run at the end, see mReader->getGameFiles().size()
        mForeignRegions.updateWorldNames(mForeignWorlds);

        return true;
    }

    if (mReader->getVer() == ESM::VER_080 // TES4
            || mReader->getVer() == ESM::VER_094 || mReader->getVer() == ESM::VER_17) // TES5
        return loadTes4Group(messages);

    ESM::NAME n = mReader->getRecName();
    mReader->getRecHeader();

    bool unhandledRecord = false;

    switch (n.val)
    {
        case ESM::REC_GLOB: mGlobals.load (*mReader, mBase); break;
        case ESM::REC_GMST: mGmsts.load (*mReader, mBase); break;
        case ESM::REC_SKIL: mSkills.load (*mReader, mBase); break;
        case ESM::REC_CLAS: mClasses.load (*mReader, mBase); break;
        case ESM::REC_FACT: mFactions.load (*mReader, mBase); break;
        case ESM::REC_RACE: mRaces.load (*mReader, mBase); break;
        case ESM::REC_SOUN: mSounds.load (*mReader, mBase); break;
        case ESM::REC_SCPT: mScripts.load (*mReader, mBase); break;
        case ESM::REC_REGN: mRegions.load (*mReader, mBase); break;
        case ESM::REC_BSGN: mBirthsigns.load (*mReader, mBase); break;
        case ESM::REC_SPEL: mSpells.load (*mReader, mBase); break;
        case ESM::REC_ENCH: mEnchantments.load (*mReader, mBase); break;
        case ESM::REC_BODY: mBodyParts.load (*mReader, mBase); break;
        case ESM::REC_SNDG: mSoundGens.load (*mReader, mBase); break;
        case ESM::REC_MGEF: mMagicEffects.load (*mReader, mBase); break;
        case ESM::REC_PGRD: mPathgrids.load (*mReader, mBase); break;
        case ESM::REC_SSCR: mStartScripts.load (*mReader, mBase); break;

        case ESM::REC_LTEX: mLandTextures.load (*mReader, mBase); break;

        case ESM::REC_LAND:
        {
            int index = mLand.load(*mReader, mBase);

            // Load all land data for now. A future optimisation may only load non-base data
            // if a suitable mechanism for avoiding race conditions can be established.
            if (index!=-1/* && !mBase*/)
                mLand.getRecord (index).get().loadData (
                    ESM::Land::DATA_VHGT | ESM::Land::DATA_VNML | ESM::Land::DATA_VCLR |
                    ESM::Land::DATA_VTEX | ESM::Land::DATA_WNAM);

            // FIXME: if one cell does not have a heightmap none of the cells render
            //if (!mLand.getRecord(index).get().getLandData (ESM::Land::DATA_VHGT))
                //std::cout << "no heightmap: " << mLand.getRecord(index).get().mId << std::endl;
            break;
        }

        case ESM::REC_CELL:
        {
            int index = mCells.load (*mReader, mBase);
            if (index < 0 || index >= mCells.getSize())
            {
                // log an error and continue loading the refs to the last loaded cell
                CSMWorld::UniversalId id (CSMWorld::UniversalId::Type_None);
                messages.add (id, "Logic error: cell index out of bounds", "", CSMDoc::Message::Severity_Error);
                index = mCells.getSize()-1;
            }
            std::string cellId = Misc::StringUtils::lowerCase (mCells.getId (index));
            mRefs.load (*mReader, index, mBase, mRefLoadCache[cellId], messages);
            break;
        }

        case ESM::REC_ACTI: mReferenceables.load (*mReader, mBase, UniversalId::Type_Activator); break;
        case ESM::REC_ALCH: mReferenceables.load (*mReader, mBase, UniversalId::Type_Potion); break;
        case ESM::REC_APPA: mReferenceables.load (*mReader, mBase, UniversalId::Type_Apparatus); break;
        case ESM::REC_ARMO: mReferenceables.load (*mReader, mBase, UniversalId::Type_Armor); break;
        case ESM::REC_BOOK: mReferenceables.load (*mReader, mBase, UniversalId::Type_Book); break;
        case ESM::REC_CLOT: mReferenceables.load (*mReader, mBase, UniversalId::Type_Clothing); break;
        case ESM::REC_CONT: mReferenceables.load (*mReader, mBase, UniversalId::Type_Container); break;
        case ESM::REC_CREA: mReferenceables.load (*mReader, mBase, UniversalId::Type_Creature); break;
        case ESM::REC_DOOR: mReferenceables.load (*mReader, mBase, UniversalId::Type_Door); break;
        case ESM::REC_INGR: mReferenceables.load (*mReader, mBase, UniversalId::Type_Ingredient); break;
        case ESM::REC_LEVC:
            mReferenceables.load (*mReader, mBase, UniversalId::Type_CreatureLevelledList); break;
        case ESM::REC_LEVI:
            mReferenceables.load (*mReader, mBase, UniversalId::Type_ItemLevelledList); break;
        case ESM::REC_LIGH: mReferenceables.load (*mReader, mBase, UniversalId::Type_Light); break;
        case ESM::REC_LOCK: mReferenceables.load (*mReader, mBase, UniversalId::Type_Lockpick); break;
        case ESM::REC_MISC:
            mReferenceables.load (*mReader, mBase, UniversalId::Type_Miscellaneous); break;
        case ESM::REC_NPC_: mReferenceables.load (*mReader, mBase, UniversalId::Type_Npc); break;
        case ESM::REC_PROB: mReferenceables.load (*mReader, mBase, UniversalId::Type_Probe); break;
        case ESM::REC_REPA: mReferenceables.load (*mReader, mBase, UniversalId::Type_Repair); break;
        case ESM::REC_STAT: mReferenceables.load (*mReader, mBase, UniversalId::Type_Static); break;
        case ESM::REC_WEAP: mReferenceables.load (*mReader, mBase, UniversalId::Type_Weapon); break;

        case ESM::REC_DIAL:
        {
            ESM::Dialogue record;
            bool isDeleted = false;

            record.load (*mReader, isDeleted);

            if (isDeleted)
            {
                // record vector can be shuffled around which would make pointer to record invalid
                mDialogue = 0;

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
                if (record.mType == ESM::Dialogue::Journal)
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

        case ESM::REC_INFO:
        {
            if (!mDialogue)
            {
                messages.add (UniversalId::Type_None,
                    "Found info record not following a dialogue record", "", CSMDoc::Message::Severity_Error);

                mReader->skipRecord();
                break;
            }

            if (mDialogue->mType==ESM::Dialogue::Journal)
                mJournalInfos.load (*mReader, mBase, *mDialogue);
            else
                mTopicInfos.load (*mReader, mBase, *mDialogue);

            break;
        }

        case ESM::REC_FILT:

            if (!mProject)
            {
                unhandledRecord = true;
                break;
            }

            mFilters.load (*mReader, mBase);
            break;

        case ESM::REC_DBGP:

            if (!mProject)
            {
                unhandledRecord = true;
                break;
            }

            mDebugProfiles.load (*mReader, mBase);
            break;

        default:

            unhandledRecord = true;
    }

    if (unhandledRecord)
    {
        messages.add (UniversalId::Type_None, "Unsupported record type: " + n.toString(), "",
            CSMDoc::Message::Severity_Error);

        mReader->skipRecord();
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

void CSMWorld::Data::dataChanged (const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
    if (topLeft.column()<=0)
        emit idListChanged();
}

void CSMWorld::Data::rowsChanged (const QModelIndex& parent, int start, int end)
{
    emit idListChanged();
}

const CSMWorld::Data& CSMWorld::Data::self ()
{
    return *this;
}
bool CSMWorld::Data::loadTes4Group (CSMDoc::Messages& messages)
{
    ESM4::Reader& reader = static_cast<ESM::ESM4Reader*>(mReader)->reader();

    reader.getRecordHeader();
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
                    hdr.group.label.value == ESM4::REC_CELL || hdr.group.label.value == ESM4::REC_LTEX)
            {
                // NOTE: The label field of a group is not reliable.  See:
                // http://www.uesp.net/wiki/Tes4Mod:Mod_File_Format
                //
                // Workaround by getting the record header and checking its typeId
                reader.saveGroupStatus(hdr);
                // CELL group with record type may have sub-groups
                loadTes4Group(messages);
                //reader.getRecordHeader(); // NOTE: header re-read
                //return loadTes4Record(reader.hdr(), messages);
            }
            else
            {
                //std::cout << "skipping group..." << std::endl; // FIXME
                reader.skipGroup();
                return false;
            }

            break;
        }
        case ESM4::Grp_WorldChild:
        case ESM4::Grp_ExteriorCell:
        case ESM4::Grp_ExteriorSubCell:
        case ESM4::Grp_InteriorCell:
        case ESM4::Grp_InteriorSubCell:
        case ESM4::Grp_CellChild:
        case ESM4::Grp_TopicChild:
        case ESM4::Grp_CellPersistentChild:
        case ESM4::Grp_CellTemporaryChild:
        case ESM4::Grp_CellVisibleDistChild:
        {
            reader.saveGroupStatus(hdr);
            loadTes4Group(messages);

            break;
        }
        default:
            //std::cout << "unknown group..." << std::endl; // FIXME
            break;
    }

    return false;
}

// Deal with Tes4 records separately, as some have the same name as Tes3, e.g. REC_CELL
bool CSMWorld::Data::loadTes4Record (const ESM4::RecordHeader& hdr, CSMDoc::Messages& messages)
{
    ESM4::Reader& reader = static_cast<ESM::ESM4Reader*>(mReader)->reader();

    switch (hdr.record.typeId)
    {
        case ESM4::REC_CELL:
        {
//FIXME: debug only
#if 0
            std::string padding = "";
            padding.insert(0, reader.stackSize()*2, ' ');
            std::cout << padding << "CELL flags 0x" << std::hex << hdr.record.flags << std::endl;
            std::cout << padding << "CELL id 0x" << std::hex << hdr.record.id << std::endl;
            std::cout << padding << "CELL group " << ESM4::printLabel(reader.grp().label, reader.grp().type) << std::endl;
#endif
            reader.getRecordData();
            mForeignCells.load(reader, mBase);
            break;
        }
        case ESM4::REC_NAVM:
        {
            // FIXME: should update mNavMesh to indicate this record was deleted
            if ((hdr.record.flags & ESM4::Rec_Deleted) != 0)
            {
//FIXME: debug only
#if 0
                std::string padding = "";
                padding.insert(0, reader.stackSize()*2, ' ');
                std::cout << padding << "NAVM: deleted record id "
                          << std::hex << hdr.record.id << std::endl;
#endif
                reader.skipRecordData();
                break;
            }

            reader.getRecordData();
            mNavMesh.load(reader, mBase);
            break;
        }
        case ESM4::REC_NAVI:
        {
            reader.getRecordData();
            mNavigation.load(reader, mBase);
            break;
        }
        case ESM4::REC_WRLD:
        {
            reader.getRecordData();
            mForeignWorlds.load(reader, mBase);
            break;
        }
        case ESM4::REC_REGN:
        {
            reader.getRecordData();
            mForeignRegions.load(reader, mBase);
            break;
        }
        case ESM4::REC_LAND:
        {
            reader.getRecordData();
            mForeignLands.load(reader, mBase);
            break;
        }
        case ESM4::REC_LTEX:
        {
            reader.getRecordData();
            mForeignLandTextures.load(reader, mBase);
            break;
        }
        case ESM4::REC_STAT:
        {
            reader.getRecordData();
            mForeignStatics.load(reader, mBase);
            break;
        }
        case ESM4::REC_ANIO:
        {
            reader.getRecordData();
            mForeignAnimObjs.load(reader, mBase);
            break;
        }
        case ESM4::REC_CONT:
        {
            reader.getRecordData();
            mForeignContainers.load(reader, mBase);
            break;
        }
        case ESM4::REC_MISC:
        {
            reader.getRecordData();
            mForeignMiscItems.load(reader, mBase);
            break;
        }
        case ESM4::REC_ACTI:
        {
            reader.getRecordData();
            mForeignActivators.load(reader, mBase);
            break;
        }
        case ESM4::REC_ARMO:
        {
            reader.getRecordData();
            mForeignArmors.load(reader, mBase);
            break;
        }
        case ESM4::REC_NPC_:
        {
            reader.getRecordData();
            mForeignNpcs.load(reader, mBase);
            break;
        }
        case ESM4::REC_FLOR:
        {
            reader.getRecordData();
            mForeignFloras.load(reader, mBase);
            break;
        }
        case ESM4::REC_GRAS:
        {
            reader.getRecordData();
            mForeignGrasses.load(reader, mBase);
            break;
        }
        case ESM4::REC_TREE:
        {
            reader.getRecordData();
            mForeignTrees.load(reader, mBase);
            break;
        }
        case ESM4::REC_LIGH:
        {
            reader.getRecordData();
            mForeignLights.load(reader, mBase);
            break;
        }
        case ESM4::REC_ACHR:
        {
            reader.getRecordData();
            mForeignChars.load(reader, mBase);
            break;
        }
        case ESM4::REC_REFR:
        {
            //std::cout << ESM4::printName(hdr.record.typeId) << " skipping..." << std::endl;
            //reader.skipRecordData();
            reader.getRecordData();
            mForeignRefs.load(reader, mBase);
            break;
        }
        case ESM4::REC_PHZD:
        case ESM4::REC_PGRE:
        case ESM4::REC_PGRD: // Oblivion only?
        case ESM4::REC_ACRE: // Oblivion only?
        case ESM4::REC_ROAD: // Oblivion only?
        {
            //std::cout << ESM4::printName(hdr.record.typeId) << " skipping..." << std::endl;
            reader.skipRecordData();
            break;
        }
        default:
        {
            messages.add (UniversalId::Type_None,
                "Unsupported TES4 record type: " + ESM4::printName(hdr.record.typeId), "",
                CSMDoc::Message::Severity_Error);

            reader.skipRecordData();
        }
    }

    return false;
}
