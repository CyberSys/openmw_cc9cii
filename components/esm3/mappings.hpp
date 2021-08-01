#ifndef ESM3_MAPPINGS_H
#define ESM3_MAPPINGS_H

#include <string>

#include <components/esm3/armo.hpp>
#include <components/esm3/body.hpp>

namespace ESM3
{
    ESM3::BodyPart::MeshPart getMeshPart(ESM3::PartReferenceType type);
    std::string getBoneName(ESM3::PartReferenceType type);
    std::string getMeshFilter(ESM3::PartReferenceType type);
}

#endif
