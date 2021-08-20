#ifndef MWINPUT_CONTROLSWITCH_H
#define MWINPUT_CONTROLSWITCH_H

#include <map>
#include <string>

namespace ESM
{
    class ESMWriter;
}

namespace ESM3
{
    struct ControlsState;
    class Reader;
}

namespace Loading
{
    class Listener;
}

namespace MWInput
{
    class ControlSwitch
    {
    public:
        ControlSwitch();

        bool get(const std::string& key);
        void set(const std::string& key, bool value);
        void clear();

        void write(ESM::ESMWriter& writer, Loading::Listener& progress);
        void readRecord(ESM3::Reader& reader, uint32_t type);
        int countSavedGameRecords() const;

    private:
        std::map<std::string, bool> mSwitches;
    };
}
#endif
