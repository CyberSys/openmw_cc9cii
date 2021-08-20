#ifndef OPENMW_ESMTOOL_RECORD_H
#define OPENMW_ESMTOOL_RECORD_H

#include <string>

#include <components/esm3/records.hpp>

namespace ESM
{
    class ESMWriter;
}

namespace ESM3
{
    class Reader;
}

namespace EsmTool
{
    template <class T> class Record;

    class RecordBase
    {
    protected:
        std::string mId;
        uint32_t mFlags;
        ESM::NAME mType;
        bool mPrintPlain;

    public:
        RecordBase ()
          : mFlags(0)
          , mPrintPlain(false)
        {
        }

        virtual ~RecordBase() {}

        virtual std::string getId() const = 0;

        uint32_t getFlags() const {
            return mFlags;
        }

        void setFlags(uint32_t flags) {
            mFlags = flags;
        }

        ESM::NAME getType() const {
            return mType;
        }

        void setPrintPlain(bool plain) {
            mPrintPlain = plain;
        }

        virtual void load(ESM3::Reader& esm) = 0;
        virtual void save(ESM::ESMWriter& esm) = 0;
        virtual void print() = 0;

        static RecordBase *create(std::uint32_t typeId);

        // just make it a bit shorter
        template <class T>
        Record<T> *cast() {
            return static_cast<Record<T> *>(this);
        }
    };

    template <class T>
    class Record : public RecordBase
    {
        T mData;
        bool mIsDeleted;

    public:
        Record()
            : mIsDeleted(false)
        {}

        std::string getId() const override {
            return mData.mId;
        }

        T &get() {
            return mData;
        }

        void save(ESM::ESMWriter& esm) override {
            mData.save(esm, mIsDeleted);
        }

        void load(ESM3::Reader& esm) override {
            mData.load(esm, mIsDeleted);
        }

        void print() override;
    };

    template<> std::string Record<ESM3::Cell>::getId() const;
    template<> std::string Record<ESM3::Land>::getId() const;
    template<> std::string Record<ESM3::MagicEffect>::getId() const;
    template<> std::string Record<ESM3::Pathgrid>::getId() const;
    template<> std::string Record<ESM3::Skill>::getId() const;

    template<> void Record<ESM3::Activator>::print();
    template<> void Record<ESM3::Potion>::print();
    template<> void Record<ESM3::Armor>::print();
    template<> void Record<ESM3::Apparatus>::print();
    template<> void Record<ESM3::BodyPart>::print();
    template<> void Record<ESM3::Book>::print();
    template<> void Record<ESM3::BirthSign>::print();
    template<> void Record<ESM3::Cell>::print();
    template<> void Record<ESM3::Class>::print();
    template<> void Record<ESM3::Clothing>::print();
    template<> void Record<ESM3::Container>::print();
    template<> void Record<ESM3::Creature>::print();
    template<> void Record<ESM3::Dialogue>::print();
    template<> void Record<ESM3::Door>::print();
    template<> void Record<ESM3::Enchantment>::print();
    template<> void Record<ESM3::Faction>::print();
    template<> void Record<ESM3::Global>::print();
    template<> void Record<ESM3::GameSetting>::print();
    template<> void Record<ESM3::DialInfo>::print();
    template<> void Record<ESM3::Ingredient>::print();
    template<> void Record<ESM3::Land>::print();
    template<> void Record<ESM3::CreatureLevList>::print();
    template<> void Record<ESM3::ItemLevList>::print();
    template<> void Record<ESM3::Light>::print();
    template<> void Record<ESM3::Lockpick>::print();
    template<> void Record<ESM3::Probe>::print();
    template<> void Record<ESM3::Repair>::print();
    template<> void Record<ESM3::LandTexture>::print();
    template<> void Record<ESM3::MagicEffect>::print();
    template<> void Record<ESM3::Miscellaneous>::print();
    template<> void Record<ESM3::NPC>::print();
    template<> void Record<ESM3::Pathgrid>::print();
    template<> void Record<ESM3::Race>::print();
    template<> void Record<ESM3::Region>::print();
    template<> void Record<ESM3::Script>::print();
    template<> void Record<ESM3::Skill>::print();
    template<> void Record<ESM3::SoundGenerator>::print();
    template<> void Record<ESM3::Sound>::print();
    template<> void Record<ESM3::Spell>::print();
    template<> void Record<ESM3::StartScript>::print();
    template<> void Record<ESM3::Static>::print();
    template<> void Record<ESM3::Weapon>::print();
}

#endif
