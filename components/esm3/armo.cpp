#include "armo.hpp"

#include <cassert>

#include "common.hpp"
#include "reader.hpp"
#include "../esm/esmwriter.hpp"

namespace ESM3
{
#if 0
    void PartReferenceList::add(Reader& reader)
    {
        PartReference pr;
        reader.getHT(pr.mPart); // The INDX byte
        pr.mMale = reader.getHNOString("BNAM");
        pr.mFemale = reader.getHNOString("CNAM");
        mParts.push_back(pr);
    }

    void PartReferenceList::load(Reader& reader)
    {
        mParts.clear();
        while (esm.isNextSub("INDX"))
        {
            add(reader);
        }
    }
#endif
    void PartReferenceList::save(ESM::ESMWriter& esm) const
    {
        for (std::vector<PartReference>::const_iterator it = mParts.begin(); it != mParts.end(); ++it)
        {
            esm.writeHNT("INDX", it->mPart);
            esm.writeHNOString("BNAM", it->mMale);
            esm.writeHNOString("CNAM", it->mFemale);
        }
    }

    unsigned int Armor::sRecordId = REC_ARMO;

    void Armor::load(Reader& reader, bool& isDeleted)
    {
        isDeleted = false;
        mRecordFlags = reader.getRecordFlags();

        mParts.mParts.clear();

        bool hasName = false;
        bool hasData = false;
        std::size_t partIndex = 0;
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
                case ESM3::SUB_ITEX: reader.getZString(mIcon); break;
                case ESM3::SUB_ENAM: reader.getZString(mEnchant); break;
                case ESM3::SUB_AODT:
                {
                    assert (subHdr.dataSize == 24 && "APMO data size mismatch");
                    reader.get(mData);
                    hasData = true;
                    break;
                }
                case ESM3::SUB_INDX:
                {
                    PartReference pr;
                    reader.get(pr.mPart); // The INDX byte
                    mParts.mParts.push_back(pr);
                    partIndex = mParts.mParts.size();
                    break;
                }
                case ESM3::SUB_BNAM:
                {
                    assert (partIndex <= mParts.mParts.size() && "Male ARMO before INDX");
                    reader.getString(mParts.mParts.back().mMale); // NOTE: string not null terminated
                    break;
                }
                case ESM3::SUB_CNAM:
                {
                    assert (partIndex <= mParts.mParts.size() && "Female ARMO before INDX");
                    reader.getString(mParts.mParts.back().mFemale); // NOTE: string not null terminated
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
            reader.fail("Missing AODT subrecord");
    }

    void Armor::save(ESM::ESMWriter& esm, bool isDeleted) const
    {
        esm.writeHNCString("NAME", mId);

        if (isDeleted)
        {
            esm.writeHNString("DELE", "", 3);
            return;
        }

        esm.writeHNCString("MODL", mModel);
        esm.writeHNOCString("FNAM", mName);
        esm.writeHNOCString("SCRI", mScript);
        esm.writeHNT("AODT", mData, 24);
        esm.writeHNOCString("ITEX", mIcon);
        mParts.save(esm);
        esm.writeHNOCString("ENAM", mEnchant);
    }

    void Armor::blank()
    {
        mData.mType = 0;
        mData.mWeight = 0;
        mData.mValue = 0;
        mData.mHealth = 0;
        mData.mEnchant = 0;
        mData.mArmor = 0;
        mParts.mParts.clear();
        mName.clear();
        mModel.clear();
        mIcon.clear();
        mScript.clear();
        mEnchant.clear();
    }
}
