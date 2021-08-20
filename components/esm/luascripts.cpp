#include "luascripts.hpp"

#include "../esm3/reader.hpp"
#include "esmwriter.hpp"

// List of all records, that are related to Lua.
//
// Record:
// LUAM - MWLua::LuaManager
//
// Subrecords:
// LUAW - Start of MWLua::WorldView data
// LUAE - Start of MWLua::LocalEvent or MWLua::GlobalEvent (eventName)
// LUAS - Start LuaUtil::ScriptsContainer data (scriptName)
// LUAD - Serialized Lua variable
// LUAT - MWLua::ScriptsContainer::Timer
// LUAC - Name of a timer callback (string)

void ESM::saveLuaBinaryData(ESMWriter& esm, const std::string& data)
{
    if (data.empty())
        return;
    esm.startSubRecord("LUAD");
    esm.write(data.data(), data.size());
    esm.endRecord("LUAD");
}

// NOTE: assumes that the sub-record header was just read
std::string ESM::loadLuaBinaryData(ESM3::Reader& esm)
{
    if (esm.subRecordHeader().typeId != ESM3::SUB_LUAD)
        esm.fail("Expected LUAD but got " + ESM::printName(esm.subRecordHeader().typeId));

    std::string data;
    data.resize(esm.subRecordHeader().dataSize);
    esm.get(*data.data(), data.size());

    return data;
}

// called from LuaManager::readRecord() after reading SUB_LUAW,
// or from ObjectState::load()
void ESM::LuaScripts::load(ESM3::Reader& esm)
{
    while (esm.getSubRecordHeader())
    {
        const ESM3::SubRecordHeader& subHdr = esm.subRecordHeader();
        switch (subHdr.typeId)
        {
            case ESM3::SUB_LUAS:
            {
                std::string name;
                esm.getString(name); // TODO: check string not null terminated

                std::string data = loadLuaBinaryData(esm);
                std::vector<LuaTimer> timers;

                while (esm.getNextSubRecordHeader(ESM3::SUB_LUAT))
                {
                    LuaTimer timer;
                    esm.get(timer.mUnit);
                    esm.get(timer.mTime);
                    esm.getSubRecordHeader(ESM3::SUB_LUAC);
                    esm.getString(timer.mCallbackName); // TODO: check string not null terminated

                    if (esm.getNextSubRecordHeader(ESM3::SUB_LUAD))
                        timer.mCallbackArgument = loadLuaBinaryData(esm);

                    timers.push_back(std::move(timer));
                }

                mScripts.push_back({std::move(name), std::move(data), std::move(timers)});
                break;
            }
            default:
                esm.cacheSubRecordHeader();
                return;
        }
    }
}

void ESM::LuaScripts::save(ESMWriter& esm) const
{
    for (const LuaScript& script : mScripts)
    {
        esm.writeHNString("LUAS", script.mScriptPath);
        if (!script.mData.empty())
            saveLuaBinaryData(esm, script.mData);
        for (const LuaTimer& timer : script.mTimers)
        {
            esm.startSubRecord("LUAT");
            esm.writeT(timer.mUnit);
            esm.writeT(timer.mTime);
            esm.endRecord("LUAT");
            esm.writeHNString("LUAC", timer.mCallbackName);
            if (!timer.mCallbackArgument.empty())
                saveLuaBinaryData(esm, timer.mCallbackArgument);
        }
    }
}
