#include "cellnameloader.hpp"

#include <components/esm3/cell.hpp>
#include <components/contentselector/view/contentselector.hpp>

QSet<QString> CellNameLoader::getCellNames(QStringList &contentPaths)
{
    QSet<QString> cellNames;
    ESM3::Reader esm;

    // Loop through all content files
    for (auto &contentPath : contentPaths) {
        esm.open(contentPath.toStdString());

        // Loop through all records
        while(esm.hasMoreRecs())
        {
            esm.getRecordHeader();

            if (isCellRecord(esm.hdr().typeId)) {
                QString cellName = getCellName(esm);
                if (!cellName.isEmpty()) {
                    cellNames.insert(cellName);
                }
            }

            // Stop loading content for this record and continue to the next
            esm.skipRecordData();
        }
    }

    return cellNames;
}

bool CellNameLoader::isCellRecord(std::uint32_t typeId)
{
    return typeId == ESM::REC_CELL;
}

QString CellNameLoader::getCellName(ESM3::Reader& esm)
{
    ESM3::Cell cell;
    bool isDeleted = false;
    cell.loadNameAndData(esm, isDeleted);

    return QString::fromStdString(cell.mName);
}

