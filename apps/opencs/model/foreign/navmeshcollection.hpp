#ifndef CSM_FOREIGN_NAVMESHCOLLECTION_H
#define CSM_FOREIGN_NAVMESHCOLLECTION_H

#include "../world/collection.hpp"
//#include "../world/nestedcollection.hpp"
//#include "../world/record.hpp"
#include "navmesh.hpp"

namespace ESM4
{
    class Reader;
}

namespace CSMWorld
{
    struct Cell;

    template<typename AT>
    struct IdAccessor;

    template<typename T, typename AT>
    class IdCollection;
}

namespace CSMForeign
{
    class NavMeshCollection : public CSMWorld::Collection<NavMesh, CSMWorld::IdAccessor<NavMesh> >//, public NestedCollection
    {
        const CSMWorld::IdCollection<CSMWorld::Cell, CSMWorld::IdAccessor<CSMWorld::Cell> >& mCells;
        //NavMesh mNavMesh;

    public:
        NavMeshCollection (const CSMWorld::IdCollection<CSMWorld::Cell, CSMWorld::IdAccessor<CSMWorld::Cell> >& cells);
        ~NavMeshCollection ();

        // similar to IdCollection but with ESM4::Reader
        int load (ESM4::Reader& reader, bool base);

        // similar to IdCollection but with ESM4::Reader
        int load (const NavMesh& record, bool base, int index = -2);

        virtual void loadRecord (NavMesh& record, ESM4::Reader& reader);
#if 0
        virtual void addNestedRow (int row, int col, int position) = 0;

        virtual void removeNestedRows (int row, int column, int subRow) = 0;

        virtual QVariant getNestedData (int row, int column, int subRow, int subColumn) const = 0;

        virtual void setNestedData (int row, int column, const QVariant& data, int subRow, int subColumn) = 0;

        virtual NestedTableWrapperBase* nestedTable (int row, int column) const = 0;

        virtual void setNestedTable (int row, int column, const NestedTableWrapperBase& nestedTable) = 0;

        virtual int getNestedRowsCount (int row, int column) const;

        virtual int getNestedColumnsCount (int row, int column) const;

        virtual NestableColumn *getNestableColumn (int column) = 0;
#endif
    private:
        NavMeshCollection ();
        NavMeshCollection (const NavMeshCollection& other);
        NavMeshCollection& operator= (const NavMeshCollection& other);
    };
}
#endif // CSM_FOREIGN_NAVMESHCOLLECTION_H
