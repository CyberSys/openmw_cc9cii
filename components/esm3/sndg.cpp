#include "sndg.hpp"

//#ifdef NDEBUG
//#undef NDEBUG
//#endif

#include "common.hpp"
#include "reader.hpp"
#include "../esm/esmwriter.hpp"

namespace ESM3
{
    unsigned int SoundGenerator::sRecordId = REC_SNDG;

    void SoundGenerator::load(Reader& reader, bool& isDeleted)
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
                    assert (sizeof(mType) == 4);
                    assert (subHdr.dataSize == sizeof(mType));
                    reader.get(mType);
                    hasData = true;
                    break;
                }
                case ESM3::SUB_CNAM: reader.getZString(mCreature); break;
                case ESM3::SUB_SNAM: reader.getZString(mSound); break;
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

    void SoundGenerator::save(ESM::ESMWriter& esm, bool isDeleted) const
    {
        esm.writeHNCString("NAME", mId);

        if (isDeleted)
        {
            esm.writeHNString("DELE", "", 3);
            return;
        }

        esm.writeHNT("DATA", mType, 4);
        esm.writeHNOCString("CNAM", mCreature);
        esm.writeHNOCString("SNAM", mSound);
    }

    void SoundGenerator::blank()
    {
        mType = LeftFoot;
        mCreature.clear();
        mSound.clear();
    }
}
