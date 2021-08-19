#include <gtest/gtest.h>

#include <boost/filesystem/fstream.hpp>

#include <components/files/configurationmanager.hpp>
#include <components/files/escape.hpp>
#include <components/esm3/reader.hpp>
#include <components/esm/esmwriter.hpp>
#include <components/loadinglistener/loadinglistener.hpp>

#include "apps/openmw/mwworld/esmstore.hpp"
#include "apps/openmw/mwmechanics/spelllist.hpp"

namespace MWMechanics
{
    SpellList::SpellList(const std::string& id, int type) : mId(id), mType(type) {}
}

static Loading::Listener dummyListener;

/// Base class for tests of ESMStore that rely on external content files to produce the test results
struct ContentFileTest : public ::testing::Test
{
  protected:

    void SetUp() override
    {
        readContentFiles();

        // load the content files
        mReaderList.resize(mContentFiles.size());

        int index=0;
        for (const auto & mContentFile : mContentFiles)
        {
            ESM::Reader* reader = ESM::Reader::getReader(mContentFile.string());
            reader->setEncoder(nullptr);
            reader->setModIndex(index);
            reader->setGlobalReaderList(&mReaderList);
            mReaderList[index] = reader;
            mEsmStore.load(*mReaderList[index], &dummyListener);

            ++index;
        }

        mEsmStore.setUp();
    }

    void TearDown() override
    {
        for (size_t i = 0; i < mReaderList.size(); ++i)
            delete mReaderList[i];
    }

    // read absolute path to content files from openmw.cfg
    void readContentFiles()
    {
        boost::program_options::variables_map variables;

        boost::program_options::options_description desc("Allowed options");
        desc.add_options()
        ("data", boost::program_options::value<Files::EscapePathContainer>()->default_value(Files::EscapePathContainer(), "data")->multitoken()->composing())
        ("content", boost::program_options::value<Files::EscapeStringVector>()->default_value(Files::EscapeStringVector(), "")
            ->multitoken()->composing(), "content file(s): esm/esp, or omwgame/omwaddon")
        ("data-local", boost::program_options::value<Files::EscapePath>()->default_value(Files::EscapePath(), ""));

        boost::program_options::notify(variables);

        mConfigurationManager.readConfiguration(variables, desc, true);

        Files::PathContainer dataDirs, dataLocal;
        if (!variables["data"].empty()) {
            dataDirs = Files::EscapePath::toPathContainer(variables["data"].as<Files::EscapePathContainer>());
        }

        Files::PathContainer::value_type local(variables["data-local"].as<Files::EscapePath>().mPath);
        if (!local.empty()) {
            dataLocal.push_back(local);
        }

        mConfigurationManager.processPaths (dataDirs);
        mConfigurationManager.processPaths (dataLocal, true);

        if (!dataLocal.empty())
            dataDirs.insert (dataDirs.end(), dataLocal.begin(), dataLocal.end());

        Files::Collections collections (dataDirs, true);

        std::vector<std::string> contentFiles = variables["content"].as<Files::EscapeStringVector>().toStdStringVector();
        for (auto & contentFile : contentFiles)
            mContentFiles.push_back(collections.getPath(contentFile));
    }

protected:
    Files::ConfigurationManager mConfigurationManager;
    MWWorld::ESMStore mEsmStore;
    std::vector<boost::filesystem::path> mContentFiles;
    std::vector<ESM::Reader*> mReaderList;
};

/// Print results of the dialogue merging process, i.e. the resulting linked list.
TEST_F(ContentFileTest, dialogue_merging_test)
{
    if (mContentFiles.empty())
    {
        std::cout << "No content files found, skipping test" << std::endl;
        return;
    }

    const std::string file = "test_dialogue_merging.txt";

    boost::filesystem::ofstream stream;
    stream.open(file);

    const MWWorld::Store<ESM3::Dialogue>& dialStore = mEsmStore.get<ESM3::Dialogue>();
    for (const auto & dial : dialStore)
    {
        stream << "Dialogue: " << dial.mId << std::endl;

        for (const auto & info : dial.mInfo)
        {
            stream << info.mId << std::endl;
        }
        stream << std::endl;
    }

    std::cout << "dialogue_merging_test successful, results printed to " << file << std::endl;
}

// Note: here we don't test records that don't use string names (e.g. Land, Pathgrid, Cell)
#define RUN_TEST_FOR_TYPES(func, arg1, arg2) \
    func<ESM3::Activator>(arg1, arg2); \
    func<ESM3::Apparatus>(arg1, arg2); \
    func<ESM3::Armor>(arg1, arg2); \
    func<ESM3::BirthSign>(arg1, arg2); \
    func<ESM3::BodyPart>(arg1, arg2); \
    func<ESM3::Book>(arg1, arg2); \
    func<ESM3::Class>(arg1, arg2); \
    func<ESM3::Clothing>(arg1, arg2); \
    func<ESM3::Container>(arg1, arg2); \
    func<ESM3::Creature>(arg1, arg2); \
    func<ESM3::CreatureLevList>(arg1, arg2); \
    func<ESM3::Dialogue>(arg1, arg2); \
    func<ESM3::Door>(arg1, arg2); \
    func<ESM3::Enchantment>(arg1, arg2); \
    func<ESM3::Faction>(arg1, arg2); \
    func<ESM3::GameSetting>(arg1, arg2); \
    func<ESM3::Global>(arg1, arg2); \
    func<ESM3::Ingredient>(arg1, arg2); \
    func<ESM3::ItemLevList>(arg1, arg2); \
    func<ESM3::Light>(arg1, arg2); \
    func<ESM3::Lockpick>(arg1, arg2); \
    func<ESM3::Miscellaneous>(arg1, arg2); \
    func<ESM3::NPC>(arg1, arg2); \
    func<ESM3::Potion>(arg1, arg2); \
    func<ESM3::Probe>(arg1, arg2); \
    func<ESM3::Race>(arg1, arg2); \
    func<ESM3::Region>(arg1, arg2); \
    func<ESM3::Repair>(arg1, arg2); \
    func<ESM3::Script>(arg1, arg2); \
    func<ESM3::Sound>(arg1, arg2); \
    func<ESM3::SoundGenerator>(arg1, arg2); \
    func<ESM3::Spell>(arg1, arg2); \
    func<ESM3::StartScript>(arg1, arg2); \
    func<ESM3::Weapon>(arg1, arg2);

template <typename T>
void printRecords(MWWorld::ESMStore& esmStore, std::ostream& outStream)
{
    const MWWorld::Store<T>& store = esmStore.get<T>();
    outStream << store.getSize() << " " << T::getRecordType() << " records" << std::endl;

    for (typename MWWorld::Store<T>::iterator it = store.begin(); it != store.end(); ++it)
    {
        const T& record = *it;
        outStream << record.mId << std::endl;
    }

    outStream << std::endl;
}

/// Print some basic diagnostics about the loaded content files, e.g. number of records and names of those records
/// Also used to test the iteration order of records
TEST_F(ContentFileTest, content_diagnostics_test)
{
    if (mContentFiles.empty())
    {
        std::cout << "No content files found, skipping test" << std::endl;
        return;
    }

    const std::string file = "test_content_diagnostics.txt";

    boost::filesystem::ofstream stream;
    stream.open(file);

    RUN_TEST_FOR_TYPES(printRecords, mEsmStore, stream);

    std::cout << "diagnostics_test successful, results printed to " << file << std::endl;
}

// TODO:
/// Print results of autocalculated NPC spell lists. Also serves as test for attribute/skill autocalculation which the spell autocalculation heavily relies on
/// - even incorrect rounding modes can completely change the resulting spell lists.
/*
TEST_F(ContentFileTest, autocalc_test)
{
    if (mContentFiles.empty())
    {
        std::cout << "No content files found, skipping test" << std::endl;
        return;
    }


}
*/

/// Base class for tests of ESMStore that do not rely on external content files
struct StoreTest : public ::testing::Test
{
protected:
    MWWorld::ESMStore mEsmStore;
    std::vector<ESM::Reader*> mReaderList;

    void TearDown() override
    {
        for (size_t i = 0; i < mReaderList.size(); ++i)
            delete mReaderList[i];
    }
};


/// Create an ESM file in-memory containing the specified record.
/// @param deleted Write record with deleted flag?
template <typename T>
Files::IStreamPtr getEsmFile(T record, bool deleted)
{
    ESM::ESMWriter writer;
    auto* stream = new std::stringstream;
    writer.setFormat(0);
    writer.save(*stream);
    writer.startRecord(T::sRecordId);
    record.save(writer, deleted);
    writer.endRecord(T::sRecordId);

    return Files::IStreamPtr(stream);
}

/// Tests deletion of records.
TEST_F(StoreTest, delete_test)
{
    const std::string recordId = "foobar";

    typedef ESM3::Apparatus RecordType;

    RecordType record;
    record.blank();
    record.mId = recordId;

    // master file inserts a record
    Files::IStreamPtr file = getEsmFile(record, false);
    ESM3::Reader* reader = new ESM3::Reader(file, "filename");
    mReaderList.push_back(reader);
    reader->setGlobalReaderList(&mReaderList);
    mEsmStore.load(*reader, &dummyListener);
    mEsmStore.setUp();

    ASSERT_TRUE (mEsmStore.get<RecordType>().getSize() == 1);

    // now a plugin deletes it
    file = getEsmFile(record, true);
    reader->open(file, "filename");
    mEsmStore.load(*reader, &dummyListener);
    mEsmStore.setUp();

    ASSERT_TRUE (mEsmStore.get<RecordType>().getSize() == 0);

    // now another plugin inserts it again
    // expected behaviour is the record to reappear rather than staying deleted
    file = getEsmFile(record, false);
    reader->open(file, "filename");
    mEsmStore.load(*reader, &dummyListener);
    mEsmStore.setUp();

    ASSERT_TRUE (mEsmStore.get<RecordType>().getSize() == 1);
}

/// Tests overwriting of records.
TEST_F(StoreTest, overwrite_test)
{
    const std::string recordId = "foobar";
    const std::string recordIdUpper = "Foobar";

    typedef ESM3::Apparatus RecordType;

    RecordType record;
    record.blank();
    record.mId = recordId;

    // master file inserts a record
    Files::IStreamPtr file = getEsmFile(record, false);
    ESM3::Reader* reader = new ESM3::Reader(file, "filename");
    mReaderList.push_back(reader);
    reader->setGlobalReaderList(&mReaderList);
    reader->open(file, "filename");
    mEsmStore.load(*reader, &dummyListener);
    mEsmStore.setUp();

    // now a plugin overwrites it with changed data
    record.mId = recordIdUpper; // change id to uppercase, to test case smashing while we're at it
    record.mModel = "the_new_model";
    file = getEsmFile(record, false);
    reader->open(file, "filename");
    mEsmStore.load(*reader, &dummyListener);
    mEsmStore.setUp();

    // verify that changes were actually applied
    const RecordType* overwrittenRec = mEsmStore.get<RecordType>().search(recordId);

    ASSERT_TRUE (overwrittenRec != nullptr);

    ASSERT_TRUE (overwrittenRec && overwrittenRec->mModel == "the_new_model");
}
