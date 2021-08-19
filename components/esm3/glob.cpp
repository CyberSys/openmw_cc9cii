#include "glob.hpp"

//#include <cassert>
#include <cmath> // std::isnan

#include "common.hpp"
#include "reader.hpp"
#include "../esm/esmwriter.hpp"
#include "../esm/common.hpp" // VarType

namespace ESM3
{
    unsigned int Global::sRecordId = REC_GLOB;

    void Global::load (Reader& reader, bool& isDeleted)
    {
        isDeleted = false;

        char type = '\0';
        while (reader.getSubRecordHeader())
        {
            const ESM3::SubRecordHeader& subHdr = reader.subRecordHeader();
            switch (subHdr.typeId)
            {
                case ESM3::SUB_NAME: reader.getZString(mId); break;
                case ESM3::SUB_FNAM:
                {
                    //assert(subHdr.dataSize == 1);
#if 0
                    mValue.read (reader, ESM3::Variant::Format_Global);
#else
                    reader.get(type);
                    break;
                }
                case ESM3::SUB_FLTV:
                {
                    //assert (type != '\0' && "GLOB value before type");

                    float value;
                    reader.get(value);

                    if (type == 's')
                    {
                        mValue.setType(ESM::VT_Short);
                        if (std::isnan(value))
                            mValue.setInteger(0);
                        else
                            mValue.setInteger(static_cast<short> (value));
                    }
                    else if (type == 'l')
                    {
                        mValue.setType(ESM::VT_Long);
                        mValue.setInteger(static_cast<int> (value));
                    }
                    else if (type == 'f')
                        mValue = Variant(value);
                    else
                        reader.fail ("unsupported global variable type");
#endif
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
    }

    void Global::save (ESM::ESMWriter& esm, bool isDeleted) const
    {
        esm.writeHNCString ("NAME", mId);

        if (isDeleted)
        {
            esm.writeHNCString ("DELE", "");
        }
        else
        {
            mValue.write (esm, ESM3::Variant::Format_Global);
        }
    }

    void Global::blank()
    {
        mValue.setType (ESM::VT_None);
    }

    bool operator== (const Global& left, const Global& right)
    {
        return left.mId==right.mId && left.mValue==right.mValue;
    }
}
