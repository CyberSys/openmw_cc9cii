#include "info.hpp"

//#include <cassert>

#include "common.hpp"
#include "reader.hpp"
#include "../esm/esmwriter.hpp"

namespace ESM3
{
    unsigned int DialInfo::sRecordId = REC_INFO;

    void DialInfo::load(Reader& reader, bool& isDeleted)
    {
        isDeleted = false;

        mQuestStatus = QS_None;
        mFactionLess = false;

        while (reader.getSubRecordHeader())
        {
            const ESM3::SubRecordHeader& subHdr = reader.subRecordHeader();
            switch (subHdr.typeId)
            {
                case ESM3::SUB_INAM: reader.getZString(mId); break;
                case ESM3::SUB_PNAM: reader.getZString(mPrev); break;
                case ESM3::SUB_NNAM: reader.getZString(mNext); break;
                case ESM3::SUB_ONAM: reader.getZString(mActor); break;
                case ESM3::SUB_RNAM: reader.getZString(mRace); break;
                case ESM3::SUB_CNAM: reader.getZString(mClass); break;
                case ESM3::SUB_ANAM: reader.getZString(mCell); break;
                case ESM3::SUB_DNAM: reader.getZString(mPcFaction); break;
                case ESM3::SUB_SNAM: reader.getZString(mSound); break;
                case ESM3::SUB_NAME: reader.getString(mResponse); break; // NOTE: not null terminated
                case ESM3::SUB_BNAM: reader.getString(mResultScript); break; // NOTE: not null terminated
                case ESM3::SUB_FNAM:
                {
                    reader.getZString(mFaction);
                    if (mFaction == "FFFF")
                    {
                        mFactionLess = true;
                    }
                    break;
                }
                case ESM3::SUB_DATA:
                {
                    //assert (subHdr.dataSize == 12 && "INFO incorrect data size");
                    reader.get(mData);
                    break;
                }
                case ESM3::SUB_SCVR:
                {
                    SelectStruct ss;
                    ss.mSelectRule.resize(subHdr.dataSize);
                    reader.get(*ss.mSelectRule.data(), subHdr.dataSize);
                    mSelects.push_back(ss);
                    break;
                }
                case ESM3::SUB_INTV: // NOTE: assumes follows SUB_SCVR
                {
                    std::uint32_t value;
                    reader.get(value);
                    mSelects.back().mValue = Variant(static_cast<int>(value));
                    mSelects.back().mValue.setType(ESM::VT_Int);
                    break;
                }
                case ESM3::SUB_FLTV: // NOTE: assumes follows SUB_SCVR
                {
                    float value;
                    reader.get(value);
                    mSelects.back().mValue = Variant(value);
                    mSelects.back().mValue.setType(ESM::VT_Float);
                    break;
                }
                case ESM3::SUB_QSTN:
                {
                    mQuestStatus = QS_Name;
                    reader.skipSubRecordData(); // FIXME: skipRecord?
                    break;
                }
                case ESM3::SUB_QSTF:
                {
                    mQuestStatus = QS_Finished;
                    reader.skipSubRecordData(); // FIXME: skipRecord?
                    break;
                }
                case ESM3::SUB_QSTR:
                {
                    mQuestStatus = QS_Restart;
                    reader.skipSubRecordData(); // FIXME: skipRecord?
                    break;
                }
                case ESM3::SUB_DELE:
                    reader.skipSubRecordData();
                    isDeleted = true;
                    break;
                default:
                    reader.fail("Unknown subrecord");
                    break;
            }
        }
    }

    void DialInfo::save(ESM::ESMWriter& esm, bool isDeleted) const
    {
        esm.writeHNCString("INAM", mId);
        esm.writeHNCString("PNAM", mPrev);
        esm.writeHNCString("NNAM", mNext);

        if (isDeleted)
        {
            esm.writeHNString("DELE", "", 3);
            return;
        }

        esm.writeHNT("DATA", mData, 12);
        esm.writeHNOCString("ONAM", mActor);
        esm.writeHNOCString("RNAM", mRace);
        esm.writeHNOCString("CNAM", mClass);
        esm.writeHNOCString("FNAM", mFaction);
        esm.writeHNOCString("ANAM", mCell);
        esm.writeHNOCString("DNAM", mPcFaction);
        esm.writeHNOCString("SNAM", mSound);
        esm.writeHNOString("NAME", mResponse);

        for (std::vector<SelectStruct>::const_iterator it = mSelects.begin(); it != mSelects.end(); ++it)
        {
            esm.writeHNString("SCVR", it->mSelectRule);
            it->mValue.write (esm, Variant::Format_Info);
        }

        esm.writeHNOString("BNAM", mResultScript);

        switch(mQuestStatus)
        {
        case QS_Name: esm.writeHNT("QSTN",'\1'); break;
        case QS_Finished: esm.writeHNT("QSTF", '\1'); break;
        case QS_Restart: esm.writeHNT("QSTR", '\1'); break;
        default: break;
        }
    }

    void DialInfo::blank()
    {
        mData.mUnknown1 = 0;
        mData.mDisposition = 0;
        mData.mRank = 0;
        mData.mGender = 0;
        mData.mPCrank = 0;
        mData.mUnknown2 = 0;

        mSelects.clear();
        mPrev.clear();
        mNext.clear();
        mActor.clear();
        mRace.clear();
        mClass.clear();
        mFaction.clear();
        mPcFaction.clear();
        mCell.clear();
        mSound.clear();
        mResponse.clear();
        mResultScript.clear();
        mFactionLess = false;
        mQuestStatus = QS_None;
    }
}
