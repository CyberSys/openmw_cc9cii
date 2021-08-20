#include "soun.hpp"

#include "common.hpp"
#include "reader.hpp"
#include "../esm/esmwriter.hpp"

namespace ESM3
{
    unsigned int Sound::sRecordId = REC_SOUN;

    void Sound::load(Reader& reader, bool& isDeleted)
    {
        isDeleted = false;

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
                case ESM3::SUB_DATA:
                {
                    if (subHdr.dataSize != sizeof(mData) || subHdr.dataSize != 3)
                        reader.fail("SOUN incorrect data size");
                    reader.get(mData);
                    hasData = true;
                    break;
                }
                case ESM3::SUB_FNAM: reader.getZString(mSound); break;
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
            reader.fail("Missing DATA subrecord");
    }

    void Sound::save(ESM::ESMWriter& esm, bool isDeleted) const
    {
        esm.writeHNCString("NAME", mId);

        if (isDeleted)
        {
            esm.writeHNString("DELE", "", 3);
            return;
        }

        esm.writeHNOCString("FNAM", mSound);
        esm.writeHNT("DATA", mData, 3);
    }

    void Sound::blank()
    {
        mSound.clear();

        mData.mVolume = 128;
        mData.mMinRange = 0;
        mData.mMaxRange = 255;
    }
}
