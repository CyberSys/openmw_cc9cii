#include "locals.hpp"

#include <cassert>
#include <stdexcept>
#include <algorithm>
#include <ostream>
#include <iterator>

#include <components/misc/stringops.hpp>

namespace Compiler
{
    const std::vector<std::string>& Locals::get (char type) const
    {
        switch (type)
        {
            case 's': return mShorts;
            case 'l': return mLongs;
            case 'f': return mFloats;
            case 'r': return mRefs; // TES4 scripts only
        }

        throw std::logic_error ("unknown variable type");
    }

    int Locals::searchIndex (char type, const std::string& name) const
    {
        const std::vector<std::string>& collection = get (type);

        std::vector<std::string>::const_iterator iter =
            std::find (collection.begin(), collection.end(), name);

        if (iter==collection.end())
            return -1;

        return iter-collection.begin();
    }

    bool Locals::search (char type, const std::string& name) const
    {
        return searchIndex (type, name)!=-1;
    }

    std::vector<std::string>& Locals::get (char type)
    {
        switch (type)
        {
            case 's': return mShorts;
            case 'l': return mLongs;
            case 'f': return mFloats;
            case 'r': return mRefs; // TES4 scripts only
        }

        throw std::logic_error ("unknown variable type");
    }

    char Locals::getType (const std::string& name) const
    {
        if (search ('s', name))
            return 's';

        if (search ('l', name))
            return 'l';

        if (search ('f', name))
            return 'f';

        if (search ('r', name)) // TES4 scripts only
            return 'r';

        return ' ';
    }

    int Locals::getIndex (const std::string& name) const
    {
        int index = searchIndex ('s', name);

        if (index!=-1)
            return index;

        index = searchIndex ('l', name);

        if (index!=-1)
            return index;

        index = searchIndex ('f', name);

        if (index!=-1)
            return index;

        return searchIndex ('r', name); // TES4 scripts only
    }

    void Locals::write (std::ostream& localFile) const
    {
        localFile
            << get ('s').size() << ' '
            << get ('l').size() << ' '
            << get ('f').size() << ' '
            << get ('r').size() << std::endl; // TES4 scripts only

        std::copy (get ('s').begin(), get ('s').end(),
            std::ostream_iterator<std::string> (localFile, " "));
        std::copy (get ('l').begin(), get ('l').end(),
            std::ostream_iterator<std::string> (localFile, " "));
        std::copy (get ('f').begin(), get ('f').end(),
            std::ostream_iterator<std::string> (localFile, " "));
        std::copy (get ('r').begin(), get ('r').end(),
            std::ostream_iterator<std::string> (localFile, " ")); // TES4 scripts only
    }

    void Locals::declare (char type, const std::string& name)
    {
        get (type).push_back (Misc::StringUtils::lowerCase (name));
    }

    void Locals::clear()
    {
        get ('s').clear();
        get ('l').clear();
        get ('f').clear();
        get ('r').clear(); // TES4 scripts only
    }
}

