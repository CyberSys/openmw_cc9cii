#include "alch.hpp"

#include <cassert>

#include "common.hpp"
#include "reader.hpp"
#include "../esm/esmwriter.hpp"

namespace ESM3
{
    unsigned int Potion::sRecordId = REC_ALCH;

    void Potion::load(Reader& reader, bool &isDeleted)
    {
        isDeleted = false;
        mRecordFlags = reader.getRecordFlags();

        mEffects.mList.clear();

        bool hasName = false;
        bool hasData = false;
        while (reader.getSubRecordHeader())
        {
            const ESM3::SubRecordHeader& subHdr = reader.subRecordHeader();
            switch (subHdr.typeId)
            {
                case ESM3::SUB_NAME:
                {
                    reader.getZString(mId); // FIXME: fixed size?
                    hasName = true;
                    break;
                }
                case ESM3::SUB_MODL: reader.getZString(mModel); break;
                case ESM3::SUB_FNAM: reader.getZString(mName); break;
                case ESM3::SUB_SCRI: reader.getZString(mScript); break;
                case ESM3::SUB_TEXT: reader.getZString(mIcon); break; // not ITEX here for some reason
                case ESM3::SUB_ALDT:
                {
                    assert (subHdr.dataSize == 12 && "ALCH data size mismatch");
                    reader.get(mData);
                    hasData = true;
                    break;
                }
                case ESM3::SUB_ENAM:
                {
                    //mEffects.add(esm);
                    ENAMstruct s;
                    assert (subHdr.dataSize == 24 && "ALCH effect size mismatch");
                    reader.get(s);
                    mEffects.mList.push_back(s);
                    break;
                }
                case ESM3::SUB_DELE:
                {
                    reader.skipSubRecordData();
                    isDeleted = true;
                    break;
                }
                default:
                    reader.fail("Unknown subrecord");
                    break;
            }
        }

        if (!hasName)
            reader.fail("Missing NAME subrecord");

        if (!hasData && !isDeleted)
            reader.fail("Missing ALDT subrecord");
    }

    void Potion::save(ESM::ESMWriter& esm, bool isDeleted) const
    {
        esm.writeHNCString("NAME", mId);

        if (isDeleted)
        {
            esm.writeHNString("DELE", "", 3);
            return;
        }

        esm.writeHNCString("MODL", mModel);
        esm.writeHNOCString("TEXT", mIcon);
        esm.writeHNOCString("SCRI", mScript);
        esm.writeHNOCString("FNAM", mName);
        esm.writeHNT("ALDT", mData, 12);
        mEffects.save(esm);
    }

    void Potion::blank()
    {
        mData.mWeight = 0;
        mData.mValue = 0;
        mData.mAutoCalc = 0;
        mName.clear();
        mModel.clear();
        mIcon.clear();
        mScript.clear();
        mEffects.mList.clear();
    }
}
