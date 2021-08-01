#include "clas.hpp"

//#ifdef NDEBUG
//#undef NDEBUG
//#endif

#include <stdexcept>
#include <cassert>

#include "common.hpp"
#include "reader.hpp"
#include "../esm/esmwriter.hpp"

namespace ESM3
{
    unsigned int Class::sRecordId = REC_CLAS;

    const Class::Specialization Class::sSpecializationIds[3] = {
      Class::Combat,
      Class::Magic,
      Class::Stealth
    };

    const char *Class::sGmstSpecializationIds[3] = {
      "sSpecializationCombat",
      "sSpecializationMagic",
      "sSpecializationStealth"
    };

    std::uint32_t& Class::CLDTstruct::getSkill (int index, bool major)
    {
        if (index<0 || index>=5)
            throw std::logic_error ("skill index out of range");

        return mSkills[index][major ? 1 : 0];
    }

    std::uint32_t Class::CLDTstruct::getSkill (int index, bool major) const
    {
        if (index<0 || index>=5)
            throw std::logic_error ("skill index out of range");

        return mSkills[index][major ? 1 : 0];
    }

    void Class::load(Reader& reader, bool& isDeleted)
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
                case ESM3::SUB_FNAM: reader.getZString(mName); break;
                case ESM3::SUB_DESC: reader.getString(mDescription); break; // NOTE: not null terminated
                case ESM3::SUB_CLDT:
                {
                    assert (sizeof(mData) == 60);
                    assert (subHdr.dataSize == sizeof(mData));
                    reader.get(mData);
                    if (mData.mIsPlayable > 1)
                        reader.fail("Unknown bool value");

                    hasData = true;
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
            reader.fail("Missing CLDT subrecord");
    }

    void Class::save(ESM::ESMWriter& esm, bool isDeleted) const
    {
        esm.writeHNCString("NAME", mId);

        if (isDeleted)
        {
            esm.writeHNString("DELE", "", 3);
            return;
        }

        esm.writeHNOCString("FNAM", mName);
        esm.writeHNT("CLDT", mData, 60);
        esm.writeHNOString("DESC", mDescription);
    }

    void Class::blank()
    {
        mName.clear();
        mDescription.clear();

        mData.mAttribute[0] = mData.mAttribute[1] = 0;
        mData.mSpecialization = 0;
        mData.mIsPlayable = 0;
        mData.mCalc = 0;

        for (int i=0; i<5; ++i)
            for (int i2=0; i2<2; ++i2)
                mData.mSkills[i][i2] = 0;
    }
}
