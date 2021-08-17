#include "luascripts.hpp"

//#ifdef NDEBUG
//#undef NDEBUG
//#endif

#include <cassert>

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
    assert (esm.subRecordHeader().typeId == ESM3::SUB_LUAD);

    std::string data;
    data.resize(esm.subRecordHeader().dataSize);
    esm.get(*data.data(), data.size());

    return data;
}

// called from LuaManager::readRecord() after reading SUB_LUAW,
// or from ObjectState::load()
void ESM::LuaScripts::load(ESM3::Reader& esm)
{
    bool subDataRemaining = false;
    while (subDataRemaining || esm.getSubRecordHeader())
    {
        subDataRemaining = false;
        const ESM3::SubRecordHeader& subHdr = esm.subRecordHeader();
        switch (subHdr.typeId)
        {
            case ESM3::SUB_LUAS:
            {
                std::string name;
                esm.getZString(name);

                std::string data = loadLuaBinaryData(esm);
                std::vector<LuaTimer> timers;

                while (esm.getSubRecordHeader())
                {
                    subDataRemaining = false;
                    const ESM3::SubRecordHeader& subHdr2 = esm.subRecordHeader();
                    if (subHdr2.typeId == ESM3::SUB_LUAT)
                    {
                        LuaTimer timer;
                        esm.get(timer.mUnit);
                        esm.get(timer.mTime);
                        esm.getSubRecordHeader();
                        assert(esm.subRecordHeader().typeId == ESM3::SUB_LUAC);
                        esm.getZString(timer.mCallbackName);

                        esm.getSubRecordHeader();
                        if (esm.subRecordHeader().typeId == ESM3::SUB_LUAD)
                            timer.mCallbackArgument = loadLuaBinaryData(esm);
                        else
                        {
                            subDataRemaining = true;
                            break;
                        }

                        timers.push_back(std::move(timer));
                    }
                    else
                    {
                        subDataRemaining = true;
                        break;
                    }
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
