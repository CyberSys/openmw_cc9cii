#include "tes3.hpp"

#include <stdexcept>
#include <cstring>

//#include <iostream> // FIXME: debugging only

#include "common.hpp"
#include "reader.hpp"

//#include "../esm/esmcommon.hpp"
#include "../esm/esmwriter.hpp"
//#include "../esm/defs.hpp"

void ESM3::Header::load(ESM3::Reader& reader)
{
    //mFlags = reader.hdr().flags;

    while (reader.getSubRecordHeader())
    {
        const ESM3::SubRecordHeader& subHdr = reader.subRecordHeader();
        switch (subHdr.typeId)
        {
            case ESM3::SUB_FORM:
            {
                if (subHdr.dataSize != sizeof(mFormat))
                    //reader.reportSubSizeMismatch(sizeof(mFormat), subHdr.dataSize);
                    throw std::runtime_error("TES3 FORM data size mismatch");

                reader.get(mFormat);

                break;
            }
            case ESM3::SUB_HEDR:
            {
                char tmp[256];
                reader.get(mData.version);
                reader.get(mData.type);
                // NOTE: OpenMW/ESSImporter/etc expect trailing nulls removed
                reader.get(tmp[0], 32);  // NOTE: fixed size string
                mData.author = std::string(&tmp[0]);
                memset(&tmp[0], 0, 32);
                reader.get(tmp[0], 256);  // NOTE: fixed size string
                mData.desc = std::string(&tmp[0]);
                reader.get(mData.records);

                break;
            }
            case ESM3::SUB_MAST: // multiple
            {
                ESM::MasterData m;
                if (!reader.getZString(m.name))
                    throw std::runtime_error("TES3 MAST data read error");

                m.size = 0;
                mMaster.push_back (m);

                break;
            }
            case ESM3::SUB_DATA:
            {
                // WARNING: assumes DATA always follows MAST
                reader.get(mMaster.back().size);

                break;
            }
            case ESM3::SUB_GMDT:
            {
                if (subHdr.dataSize != sizeof(mGameData))
                    //reader.reportSubSizeMismatch(sizeof(mGameData), subHdr.dataSize);
                    throw std::runtime_error("TES3 GMDT data size mismatch");

                reader.get(mGameData);

                break;
            }
            case ESM3::SUB_SCRD:
            {
                mSCRD.resize(subHdr.dataSize);
                if (!mSCRD.empty())
                    reader.get(*mSCRD.data(), subHdr.dataSize);

                break;
            }
            case ESM3::SUB_SCRS:
            {
                mSCRS.resize(subHdr.dataSize);
                if (!mSCRS.empty())
                    reader.get(*mSCRS.data(), subHdr.dataSize);

                break;
            }
            default:
                //std::cout << ESM::printName(subHdr.typeId) << " skipping..." << std::endl;
                //reader.skipSubRecordData();
                throw std::runtime_error("ESM3::Header::load - Unknown subrecord " + ESM::printName(subHdr.typeId));
        }
    }
}

void ESM3::Header::blank()
{
    mData.version.ui = ESM::VER_13;
    mData.type = 0;
    mData.author.clear();
    mData.author.resize(32); // NOTE: fixed size
    mData.desc.clear();
    mData.desc.resize(256);  // NOTE: fixed size
    mData.records = 0;

    mFormat = CurrentFormat;
    mMaster.clear();
}

void ESM3::Header::save (ESM::ESMWriter &esm)
{
    if (mFormat>0)
        esm.writeHNT ("FORM", mFormat);

    esm.startSubRecord("HEDR");
    esm.writeT(mData.version);
    esm.writeT(mData.type);
    esm.writeFixedSizeString(mData.author, 32);
    esm.writeFixedSizeString(mData.desc, 256);
    esm.writeT(mData.records);
    esm.endRecord("HEDR");

    for (const ESM::MasterData& data : mMaster)
    {
        esm.writeHNCString ("MAST", data.name);
        esm.writeHNT ("DATA", data.size);
    }
}
