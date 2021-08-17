#include "queststate.hpp"

//#ifdef NDEBUG
//#undef NDEBUG
//#endif

#include <cassert>

#include "common.hpp"
#include "reader.hpp"
#include "../esm/esmwriter.hpp"

void ESM3::QuestState::load (Reader& esm)
{
    esm.getSubRecordHeader();
    assert(esm.subRecordHeader().typeId == ESM3::SUB_YETO);
    esm.getString(mTopic); // NOTE: not null terminated

    if (esm.getNextSubRecordType() == ESM3::SUB_QSTA && esm.getSubRecordHeader())
        esm.get(mState);
    if (esm.getNextSubRecordType() == ESM3::SUB_QFIN && esm.getSubRecordHeader())
        esm.get(mFinished);
}

void ESM3::QuestState::save (ESM::ESMWriter& esm) const
{
    esm.writeHNString ("YETO", mTopic);
    esm.writeHNT ("QSTA", mState);
    esm.writeHNT ("QFIN", mFinished);
}
