#include "compilercontext.hpp"

#include "../mwworld/esmstore.hpp"

#include <components/esm3/dial.hpp>

#include <components/compiler/locals.hpp>

#include "../mwbase/environment.hpp"
#include "../mwbase/world.hpp"
#include "../mwbase/scriptmanager.hpp"

#include "../mwworld/ptr.hpp"
#include "../mwworld/class.hpp"
#include "../mwworld/manualref.hpp"

namespace MWScript
{
    CompilerContext::CompilerContext (Type type)
    : mType (type)
    {}

    bool CompilerContext::canDeclareLocals() const
    {
        return mType==Type_Full;
    }

    char CompilerContext::getGlobalType (const std::string& name) const
    {
        return MWBase::Environment::get().getWorld()->getGlobalVariableType (name);
    }

    std::pair<char, bool> CompilerContext::getMemberType (const std::string& name,
        const std::string& id) const
    {
        std::string script;
        bool reference = false;

        if (const ESM3::Script *scriptRecord =
            MWBase::Environment::get().getWorld()->getStore().get<ESM3::Script>().search (id))
        {
            script = scriptRecord->mId;
        }
        else
        {
            MWWorld::ManualRef ref (MWBase::Environment::get().getWorld()->getStore(), id);

            script = ref.getPtr().getClass().getScript (ref.getPtr());
            reference = true;
        }

        char type = ' ';

        if (!script.empty())
            type = MWBase::Environment::get().getScriptManager()->getLocals (script).getType (
                Misc::StringUtils::lowerCase (name));

        return std::make_pair (type, reference);
    }

    bool CompilerContext::isId (const std::string& name) const
    {
        const MWWorld::ESMStore &store =
            MWBase::Environment::get().getWorld()->getStore();

        return
            store.get<ESM3::Activator>().search (name) ||
            store.get<ESM3::Potion>().search (name) ||
            store.get<ESM3::Apparatus>().search (name) ||
            store.get<ESM3::Armor>().search (name) ||
            store.get<ESM3::Book>().search (name) ||
            store.get<ESM3::Clothing>().search (name) ||
            store.get<ESM3::Container>().search (name) ||
            store.get<ESM3::Creature>().search (name) ||
            store.get<ESM3::Door>().search (name) ||
            store.get<ESM3::Ingredient>().search (name) ||
            store.get<ESM3::CreatureLevList>().search (name) ||
            store.get<ESM3::ItemLevList>().search (name) ||
            store.get<ESM3::Light>().search (name) ||
            store.get<ESM3::Lockpick>().search (name) ||
            store.get<ESM3::Miscellaneous>().search (name) ||
            store.get<ESM3::NPC>().search (name) ||
            store.get<ESM3::Probe>().search (name) ||
            store.get<ESM3::Repair>().search (name) ||
            store.get<ESM3::Static>().search (name) ||
            store.get<ESM3::Weapon>().search (name) ||
            store.get<ESM3::Script>().search (name);
    }

    bool CompilerContext::isJournalId (const std::string& name) const
    {
        const MWWorld::ESMStore &store =
            MWBase::Environment::get().getWorld()->getStore();

        const ESM3::Dialogue *topic = store.get<ESM3::Dialogue>().search (name);

        return topic && topic->mType==ESM3::Dialogue::Journal;
    }
}
