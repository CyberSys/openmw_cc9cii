#ifndef ESM3_FOGSTATE_H
#define ESM3_FOGSTATE_H

#include <vector>

namespace ESM
{
    class ESMWriter;
}

namespace ESM3
{
    class Reader;

    struct FogTexture
    {
        int mX, mY; // Only used for interior cells
        std::vector<char> mImageData;
    };

    // format 0, saved games only
    // Fog of war state
    struct FogState
    {
        // Only used for interior cells
        float mNorthMarkerAngle;
        struct Bounds
        {
            float mMinX;
            float mMinY;
            float mMaxX;
            float mMaxY;
        } mBounds;

        std::vector<FogTexture> mFogTextures;

        void load (Reader& esm);
        void save (ESM::ESMWriter& esm, bool interiorCell) const;
    };
}

#endif
