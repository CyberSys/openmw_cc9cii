#include "fact.hpp"

#include <cassert>
#include <stdexcept>

#include "common.hpp"
#include "reader.hpp"
#include "../esm/esmwriter.hpp"

namespace ESM3
{
    unsigned int Faction::sRecordId = REC_FACT;

    int& Faction::FADTstruct::getSkill (int index, bool ignored)
    {
        if (index<0 || index>=7)
            throw std::logic_error ("skill index out of range");

        return mSkills[index];
    }

    int Faction::FADTstruct::getSkill (int index, bool ignored) const
    {
        if (index<0 || index>=7)
            throw std::logic_error ("skill index out of range");

        return mSkills[index];
    }

    void Faction::load(Reader& reader, bool& isDeleted)
    {
        isDeleted = false;

        mReactions.clear();
        for (int i=0;i<10;++i)
            mRanks[i].clear();

        int rankCounter = 0;
        bool hasName = false;
        bool hasData = false;
        while (reader.getSubRecordHeader())
        {
            const ESM3::SubRecordHeader& subHdr = reader.subRecordHeader();
            switch (subHdr.typeId)
            {
                case ESM3::SUB_NAME:
                {
                    reader.getZString(mId);
                    hasName = true;
                    break;
                }
                case ESM3::SUB_FNAM: reader.getZString(mName); break;
                case ESM3::SUB_RNAM:
                {
                    if (rankCounter >= 10)
                        reader.fail("Rank out of range");

                    std::string rank;
                    reader.getZString(rank); // NOTE: string has trailing nulls
                    mRanks[rankCounter++] = rank;
                    break;
                }
                case ESM3::SUB_FADT:
                {
                    assert (subHdr.dataSize == 240 && "FACT incorrect data size");
                    assert (subHdr.dataSize == sizeof(mData) && "FACT incorrect data size");
                    reader.get(mData);
                    if (mData.mIsHidden > 1)
                        reader.fail("Unknown flag!");

                    hasData = true;
                    break;
                }
                case ESM3::SUB_ANAM:
                {
                    std::string faction;
                    reader.getString(faction); // NOTE: string not null terminated

                    reader.getSubRecordHeader(); // WARN: assumes INTV follows immediatly after
                    if (subHdr.typeId != ESM3::SUB_INTV)
                        reader.fail("INTV was expected but a different sub-record found");

                    std::int32_t reaction;
                    reader.get(reaction);
                    mReactions[faction] = reaction;
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
            reader.fail("Missing FADT subrecord");
    }

    void Faction::save(ESM::ESMWriter& esm, bool isDeleted) const
    {
        esm.writeHNCString("NAME", mId);

        if (isDeleted)
        {
            esm.writeHNString("DELE", "", 3);
            return;
        }

        esm.writeHNOCString("FNAM", mName);

        for (int i = 0; i < 10; i++)
        {
            if (mRanks[i].empty())
                break;

            esm.writeHNString("RNAM", mRanks[i], 32);
        }

        esm.writeHNT("FADT", mData, 240);

        for (std::map<std::string, int>::const_iterator it = mReactions.begin(); it != mReactions.end(); ++it)
        {
            esm.writeHNString("ANAM", it->first);
            esm.writeHNT("INTV", it->second);
        }
    }

    void Faction::blank()
    {
        mName.clear();
        mData.mAttribute[0] = mData.mAttribute[1] = 0;
        mData.mIsHidden = 0;

        for (int i=0; i<10; ++i)
        {
            mData.mRankData[i].mAttribute1 = mData.mRankData[i].mAttribute2 = 0;
            mData.mRankData[i].mPrimarySkill = mData.mRankData[i].mFavouredSkill = 0;
            mData.mRankData[i].mFactReaction = 0;

            mRanks[i].clear();
        }

        for (int i=0; i<7; ++i)
            mData.mSkills[i] = 0;

        mReactions.clear();
    }
}
