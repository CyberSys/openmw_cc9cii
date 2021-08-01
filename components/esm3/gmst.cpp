#include "gmst.hpp"

#include "common.hpp"
#include "reader.hpp"
#include "../esm/esmwriter.hpp"

namespace ESM3
{
    unsigned int GameSetting::sRecordId = REC_GMST;

    void GameSetting::load (Reader &reader, bool &isDeleted)
    {
        isDeleted = false; // GameSetting record can't be deleted now (may be changed in the future)

        while (reader.getSubRecordHeader())
        {
            const ESM3::SubRecordHeader& subHdr = reader.subRecordHeader();
            switch (subHdr.typeId)
            {
                case ESM3::SUB_NAME:
                {
                    reader.getString(mId); // NOTE: string not null terminated
#if 0
                    mValue.read (reader, ESM3::Variant::Format_Gmst);
#else
                    break;
                }
                case ESM3::SUB_FLTV:
                {
                    float value;
                    reader.get(value);
                    mValue = Variant(value);
                    mValue.setType(ESM::VT_Float);
                    break;
                }
                case ESM3::SUB_INTV:
                {
                    std::int32_t value;
                    reader.get(value);
                    mValue = Variant(value);
                    mValue.setType(ESM::VT_Int);
                    break;
                }
                case ESM3::SUB_STRV:
                {
                    std::string value;
                    reader.getString(value); // NOTE: string not null terminated
                    mValue = Variant(value);
                    mValue.setType(ESM::VT_String);
#endif
                    break;
                }
                case ESM3::SUB_DELE: reader.skipSubRecordData(); break;
                default:
                    reader.fail("Unknown subrecord");
                    break;
            }
        }
    }

    void GameSetting::save (ESM::ESMWriter &esm, bool /*isDeleted*/) const
    {
        esm.writeHNCString("NAME", mId);
        mValue.write (esm, ESM3::Variant::Format_Gmst);
    }

    void GameSetting::blank()
    {
        mValue.setType (ESM::VT_None);
    }

    bool operator== (const GameSetting& left, const GameSetting& right)
    {
        return left.mValue==right.mValue;
    }
}
