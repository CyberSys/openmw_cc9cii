#include "regn.hpp"

#include "common.hpp"
#include "reader.hpp"
#include "../esm/esmwriter.hpp"

namespace ESM3
{
    unsigned int Region::sRecordId = REC_REGN;

    void Region::load(Reader& reader, bool& isDeleted)
    {
        isDeleted = false;

        bool hasName = false;
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
                case ESM3::SUB_BNAM: reader.getZString(mSleepList); break;
                case ESM3::SUB_CNAM: reader.get(mMapColor); break;
                case ESM3::SUB_WEAT:
                {
                    if (reader.esmVersion() == ESM::VER_120)
                    {
                        mData.mA = 0;
                        mData.mB = 0;
                        reader.get(mData, sizeof(mData) - 2);
                    }
                    else if (reader.esmVersion() == ESM::VER_130)
                    {
                        // May include the additional two bytes (but not necessarily)
                        if (subHdr.dataSize == sizeof(mData))
                        {
                            reader.get(mData);
                        }
                        else
                        {
                            mData.mA = 0;
                            mData.mB = 0;
                            reader.get(mData, sizeof(mData)-2);
                        }
                    }
                    else
                    {
                        reader.fail("Don't know what to do in this version");
                    }
                    break;
                }
                case ESM3::SUB_SNAM:
                {
                    SoundRef sr;
                    char tmp[32];
                    reader.get(tmp[0], 32);  // NOTE: fixed size string
                    sr.mSound = std::string(&tmp[0]); // remove junk

                    reader.get(sr.mChance);
                    mSoundList.push_back(sr);
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
    }

    void Region::save(ESM::ESMWriter& esm, bool isDeleted) const
    {
        esm.writeHNCString("NAME", mId);

        if (isDeleted)
        {
            esm.writeHNString("DELE", "", 3);
            return;
        }

        esm.writeHNOCString("FNAM", mName);

        if (esm.getVersion() == ESM::VER_12)
            esm.writeHNT("WEAT", mData, sizeof(mData) - 2);
        else
            esm.writeHNT("WEAT", mData);

        esm.writeHNOCString("BNAM", mSleepList);

        esm.writeHNT("CNAM", mMapColor);
        for (std::vector<SoundRef>::const_iterator it = mSoundList.begin(); it != mSoundList.end(); ++it)
        {
            esm.startSubRecord("SNAM");
            esm.writeFixedSizeString(it->mSound, 32);
            esm.writeT(it->mChance);
            esm.endRecord("NPCO");
        }
    }

    void Region::blank()
    {
        mData.mClear = mData.mCloudy = mData.mFoggy = mData.mOvercast = mData.mRain =
            mData.mThunder = mData.mAsh = mData.mBlight = mData.mA = mData.mB = 0;

        mMapColor = 0;

        mName.clear();
        mSleepList.clear();
        mSoundList.clear();
    }
}
