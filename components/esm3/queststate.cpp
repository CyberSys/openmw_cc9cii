#include "queststate.hpp"

#include "reader.hpp"
#include "../esm/esmwriter.hpp"

void ESM3::QuestState::load (Reader& esm)
{
    mTopic = esm.getHNString ("YETO");
    esm.getHNOT (mState, "QSTA");
    esm.getHNOT (mFinished, "QFIN");
}

void ESM3::QuestState::save (ESM::ESMWriter& esm) const
{
    esm.writeHNString ("YETO", mTopic);
    esm.writeHNT ("QSTA", mState);
    esm.writeHNT ("QFIN", mFinished);
}
