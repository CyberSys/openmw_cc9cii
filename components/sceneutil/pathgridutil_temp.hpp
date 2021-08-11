#ifndef OPENMW_COMPONENTS_PATHGRIDUTIL_TEMP_H
#define OPENMW_COMPONENTS_PATHGRIDUTIL_TEMP_H

#include <osg/ref_ptr>
#include <osg/Geometry>

namespace ESM3
{
    struct Pathgrid;
}

namespace SceneUtil3
{
    const float DiamondHalfHeight = 40.f;
    const float DiamondHalfWidth = 16.f;

    osg::ref_ptr<osg::Geometry> createPathgridGeometry(const ESM3::Pathgrid& pathgrid);

    osg::ref_ptr<osg::Geometry> createPathgridSelectedWireframe(const ESM3::Pathgrid& pathgrid,
        const std::vector<unsigned short>& selected);

    unsigned short getPathgridNode(unsigned short vertexIndex);
}

#endif
