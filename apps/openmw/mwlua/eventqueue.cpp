#include "eventqueue.hpp"

#include <components/debug/debuglog.hpp>

#include <components/esm3/reader.hpp>
#include <components/esm/esmwriter.hpp>
#include <components/esm/luascripts.hpp>

#include <components/lua/serialization.hpp>

namespace MWLua
{

    template <typename Event>
    void saveEvent(ESM::ESMWriter& esm, const ObjectId& dest, const Event& event)
    {
        esm.writeHNString("LUAE", event.mEventName);
        dest.save(esm, true);
        if (!event.mEventData.empty())
            saveLuaBinaryData(esm, event.mEventData);
    }

    void loadEvents(sol::state& lua, ESM3::Reader& esm, GlobalEventQueue& globalEvents, LocalEventQueue& localEvents,
                    const std::map<int, int>& contentFileMapping, const LuaUtil::UserdataSerializer* serializer)
    {
        while (esm.getSubRecordHeader())
        {
            const ESM3::SubRecordHeader& subHdr = esm.subRecordHeader();
            switch (subHdr.typeId)
            {
                case ESM3::SUB_LUAE:
                {
                    std::string name;
                    esm.getZString(name);

                    ObjectId dest;
                    dest.load(esm, true);

                    esm.getSubRecordHeader();
                    std::string data = loadLuaBinaryData(esm);
                    try
                    {
                        data = LuaUtil::serialize(LuaUtil::deserialize(lua, data, serializer), serializer);
                    }
                    catch (std::exception& e)
                    {
                        Log(Debug::Error) << "loadEvent: invalid event data: " << e.what();
                    }
                    if (dest.isSet())
                    {
                        auto it = contentFileMapping.find(dest.mContentFile);
                        if (it != contentFileMapping.end())
                            dest.mContentFile = it->second;
                        localEvents.push_back({dest, std::move(name), std::move(data)});
                    }
                    else
                        globalEvents.push_back({std::move(name), std::move(data)});

                    break;
                }
                default:
                    esm.skipSubRecordData(); // should really fail() here
                    return;
            }
        }
    }

    void saveEvents(ESM::ESMWriter& esm, const GlobalEventQueue& globalEvents, const LocalEventQueue& localEvents)
    {
        ObjectId globalId;
        globalId.unset();  // Used as a marker of a global event.

        for (const GlobalEvent& e : globalEvents)
            saveEvent(esm, globalId, e);
        for (const LocalEvent& e : localEvents)
            saveEvent(esm, e.mDest, e);
    }

}
