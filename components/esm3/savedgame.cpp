#include "savedgame.hpp"

//#ifdef NDEBUG
//#undef NDEBUG
//#endif

#include <cassert>

#include "common.hpp"
#include "reader.hpp"
#include "../esm/esmwriter.hpp"

unsigned int ESM3::SavedGame::sRecordId = ESM::REC_SAVE;
int ESM3::SavedGame::sCurrentFormat = 16;

void ESM3::SavedGame::load (Reader& esm)
{
    esm.getSubRecordHeader();
    assert(esm.subRecordHeader().typeId == ESM3::SUB_PLNA);
    esm.getString(mPlayerName); // NOTE: not null terminated

    if (esm.getNextSubRecordType() == ESM3::SUB_PLLE && esm.getSubRecordHeader())
        esm.get(mPlayerLevel);

    if (esm.getNextSubRecordType() == ESM3::SUB_PLCL && esm.getSubRecordHeader())
        esm.getString(mPlayerClassId); // NOTE: not null terminated
    if (esm.getNextSubRecordType() == ESM3::SUB_PLCN && esm.getSubRecordHeader())
        esm.getString(mPlayerClassName); // NOTE: not null terminated

    esm.getSubRecordHeader();
    assert(esm.subRecordHeader().typeId == ESM3::SUB_PLCE);
    esm.getString(mPlayerCell); // NOTE: not null terminated
    esm.getSubRecordHeader();
    assert(esm.subRecordHeader().typeId == ESM3::SUB_TSTM);
    esm.get(mInGameTime, 16);
    esm.getSubRecordHeader();
    assert(esm.subRecordHeader().typeId == ESM3::SUB_TIME);
    esm.get(mTimePlayed);
    esm.getSubRecordHeader();
    assert(esm.subRecordHeader().typeId == ESM3::SUB_DESC);
    esm.getString(mDescription); // NOT: not null terminated

    while (esm.getNextSubRecordType() == ESM3::SUB_DEPE && esm.getSubRecordHeader())
    {
        std::string contentFile;
        esm.getString(contentFile); // NOTE: not null terminated
        mContentFiles.push_back(contentFile);
    }

    if (esm.getNextSubRecordType() == ESM3::SUB_SCRN && esm.getSubRecordHeader())
    {
        mScreenshot.resize(esm.subRecordHeader().dataSize);
        esm.get(*mScreenshot.data(), mScreenshot.size());
    }
}

void ESM3::SavedGame::save (ESM::ESMWriter& esm) const
{
    esm.writeHNString ("PLNA", mPlayerName);
    esm.writeHNT ("PLLE", mPlayerLevel);

    if (!mPlayerClassId.empty())
        esm.writeHNString ("PLCL", mPlayerClassId);
    else
        esm.writeHNString ("PLCN", mPlayerClassName);

    esm.writeHNString ("PLCE", mPlayerCell);
    esm.writeHNT ("TSTM", mInGameTime, 16);
    esm.writeHNT ("TIME", mTimePlayed);
    esm.writeHNString ("DESC", mDescription);

    for (std::vector<std::string>::const_iterator iter (mContentFiles.begin());
         iter!=mContentFiles.end(); ++iter)
         esm.writeHNString ("DEPE", *iter);

    esm.startSubRecord("SCRN");
    esm.write(&mScreenshot[0], mScreenshot.size());
    esm.endRecord("SCRN");
}
