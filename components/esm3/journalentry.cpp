#include "journalentry.hpp"

#include "reader.hpp"
#include "../esm/esmwriter.hpp"

void ESM3::JournalEntry::load (Reader& esm)
{
    if (esm.getNextSubRecordHeader(ESM3::SUB_JETY))
        esm.get(mType);

    esm.getSubRecordHeader(ESM3::SUB_YETO);
    esm.getString (mTopic); // FIXME: check if string null terminated

    esm.getSubRecordHeader(ESM3::SUB_YEIN);
    esm.getString (mInfo); // FIXME: check if string null terminated

    esm.getSubRecordHeader(ESM3::SUB_TEXT);
    esm.getString (mText); // FIXME: check if string null terminated

    if (mType==Type_Journal)
    {
        esm.getSubRecordHeader(ESM3::SUB_JEDA);
        esm.get(mDay);

        esm.getSubRecordHeader(ESM3::SUB_JEMO);
        esm.get(mMonth);

        esm.getSubRecordHeader(ESM3::SUB_JEDM);
        esm.get(mDayOfMonth);
    }
    else if (mType == Type_Topic)
    {
        if (esm.getNextSubRecordHeader(ESM3::SUB_ACT_))
            esm.getString(mActorName); // FIXME: check if string null terminated
    }
}

void ESM3::JournalEntry::save (ESM::ESMWriter& esm) const
{
    esm.writeHNT ("JETY", mType);
    esm.writeHNString ("YETO", mTopic);
    esm.writeHNString ("YEIN", mInfo);
    esm.writeHNString ("TEXT", mText);

    if (mType==Type_Journal)
    {
        esm.writeHNT ("JEDA", mDay);
        esm.writeHNT ("JEMO", mMonth);
        esm.writeHNT ("JEDM", mDayOfMonth);
    }
    else if (mType==Type_Topic)
        esm.writeHNString ("ACT_", mActorName);
}
