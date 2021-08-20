#include "queststate.hpp"

#include "common.hpp"
#include "reader.hpp"
#include "../esm/esmwriter.hpp"

void ESM3::QuestState::load (Reader& esm)
{
    esm.getSubRecordHeader(ESM3::SUB_YETO);
    esm.getString(mTopic); // NOTE: not null terminated

    if (esm.getNextSubRecordHeader(ESM3::SUB_QSTA))
        esm.get(mState);
    if (esm.getNextSubRecordHeader(ESM3::SUB_QFIN))
        esm.get(mFinished);
}

void ESM3::QuestState::save (ESM::ESMWriter& esm) const
{
    esm.writeHNString ("YETO", mTopic);
    esm.writeHNT ("QSTA", mState);
    esm.writeHNT ("QFIN", mFinished);
}
