#ifndef OPENMW_ESSIMPORT_IMPORTDIAL_H
#define OPENMW_ESSIMPORT_IMPORTDIAL_H
namespace ESM3
{
    class Reader;
}

namespace ESSImport
{

    struct DIAL
    {
        int mIndex; // Journal index

        void load(ESM3::Reader& esm);
    };

}

#endif
