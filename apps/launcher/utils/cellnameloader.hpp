#ifndef OPENMW_CELLNAMELOADER_H
#define OPENMW_CELLNAMELOADER_H

#include <cstdint>

#include <QSet>
#include <QString>

#include <components/esm3/reader.hpp>

namespace ESM
{
    class Reader;
    struct Cell;
}

namespace ContentSelectorView
{
    class ContentSelector;
}

class CellNameLoader
{

public:

    /**
     * Returns the names of all cells contained within the given content files
     * @param contentPaths the file paths of each content file to be examined
     * @return the names of all cells
     */
    QSet<QString> getCellNames(QStringList &contentPaths);

private:
    /**
     * Returns whether or not the given record is of type "Cell"
     * @param name The name associated with the record
     * @return whether or not the given record is of type "Cell"
     */
    bool isCellRecord(std::uint32_t typeId);

    /**
     * Returns the name of the cell
     * @param esmReader the reader currently pointed to a loaded cell
     * @return the name of the cell
     */
    QString getCellName(ESM3::Reader& esm);
};


#endif //OPENMW_CELLNAMELOADER_H
