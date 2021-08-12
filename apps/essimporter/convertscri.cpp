#include "convertscri.hpp"

#include <components/esm3/variant.hpp>

namespace
{

    template <typename T, ESM::VarType VariantType>
    void storeVariables(const std::vector<T>& variables, ESM3::Locals& locals, const std::string& scriptname)
    {
        for (const auto& variable : variables)
        {
            ESM3::Variant val(variable);
            val.setType(VariantType);
            locals.mVariables.emplace_back(std::string(), val);
        }
    }

}

namespace ESSImport
{

    void convertSCRI(const SCRI &scri, ESM3::Locals &locals)
    {
        // order *is* important, as we do not have variable names available in this format
        storeVariables<short, ESM::VT_Short> (scri.mShorts, locals, scri.mScript);
        storeVariables<int, ESM::VT_Int> (scri.mLongs, locals, scri.mScript);
        storeVariables<float, ESM::VT_Float> (scri.mFloats, locals, scri.mScript);
    }

}
