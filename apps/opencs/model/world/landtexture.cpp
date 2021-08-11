#include "landtexture.hpp"

#include <sstream>
#include <stdexcept>

#include <components/esm3/reader.hpp>

namespace CSMWorld
{
    void LandTexture::load(ESM::Reader& reader, bool& isDeleted)
    {
        ESM3::LandTexture::load(static_cast<ESM3::Reader&>(reader), isDeleted);

        mPluginIndex = static_cast<ESM3::Reader&>(reader).getModIndex(); // FIXME: is there another way to get index?
    }

    std::string LandTexture::createUniqueRecordId(int plugin, int index)
    {
        std::stringstream ss;
        ss << 'L' << plugin << '#' << index;
        return ss.str();
    }

    void LandTexture::parseUniqueRecordId(const std::string& id, int& plugin, int& index)
    {
        size_t middle = id.find('#');

        if (middle == std::string::npos || id[0] != 'L')
            throw std::runtime_error("Invalid LandTexture ID");

        plugin = std::stoi(id.substr(1,middle-1));
        index = std::stoi(id.substr(middle+1));
    }
}
