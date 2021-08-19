#include "cont.hpp"

//#include <cassert>
#include <iostream>

#include "common.hpp"
#include "reader.hpp"
#include "../esm/esmwriter.hpp"

namespace ESM3
{
    // NOTE: assumes sub-record header was just read
    void InventoryList::add(Reader& reader)
    {
        ContItem ci;
        reader.get(ci.mCount);
#if 0
        ci.mItem.resize(32);
        reader.get(*ci.mItem.data(), 32); // NOTE: fixed size string
#else
        char tmp[32];
        reader.get(tmp[0], 32); // NOTE: fixed size string
        ci.mItem = std::string(&tmp[0]); // remove junk
#endif
        mList.push_back(ci);
    }

    void InventoryList::save(ESM::ESMWriter& esm) const
    {
        for (std::vector<ContItem>::const_iterator it = mList.begin(); it != mList.end(); ++it)
        {
            esm.startSubRecord("NPCO");
            esm.writeT(it->mCount);
            esm.writeFixedSizeString(it->mItem, 32);
            esm.endRecord("NPCO");
        }
    }

    unsigned int Container::sRecordId = REC_CONT;

    void Container::load(Reader& reader, bool& isDeleted)
    {
        isDeleted = false;
        mRecordFlags = reader.getRecordFlags();

        mInventory.mList.clear();

        bool hasName = false;
        bool hasWeight = false;
        bool hasFlags = false;
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
                case ESM3::SUB_MODL: reader.getZString(mModel); break;
                case ESM3::SUB_FNAM: reader.getZString(mName); break;
                case ESM3::SUB_SCRI: reader.getZString(mScript); break;
                case ESM3::SUB_CNDT:
                {
                    //assert (subHdr.dataSize == 4 && "CONT weight size mismatch");
                    reader.get(mWeight);
                    hasWeight = true;
                    break;
                }
                case ESM3::SUB_FLAG:
                {
                    //assert (subHdr.dataSize == 4 && "CONT flag size mismatch");
                    reader.get(mFlags);
                    if (mFlags & 0xf4)
                        reader.fail("Unknown flags");
                    if (!(mFlags & 0x8))
                        reader.fail("Flag 8 not set");
                    hasFlags = true;
                    break;
                }
                case ESM3::SUB_NPCO:
                {
                    mInventory.add(reader);
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

        if (!hasWeight && !isDeleted)
            reader.fail("Missing CNDT subrecord");

        if (!hasFlags && !isDeleted)
            reader.fail("Missing FLAG subrecord");
    }

    void Container::save(ESM::ESMWriter& esm, bool isDeleted) const
    {
        esm.writeHNCString("NAME", mId);

        if (isDeleted)
        {
            esm.writeHNString("DELE", "", 3);
            return;
        }

        esm.writeHNCString("MODL", mModel);
        esm.writeHNOCString("FNAM", mName);
        esm.writeHNT("CNDT", mWeight, 4);
        esm.writeHNT("FLAG", mFlags, 4);

        esm.writeHNOCString("SCRI", mScript);

        mInventory.save(esm);
    }

    void Container::blank()
    {
        mName.clear();
        mModel.clear();
        mScript.clear();
        mWeight = 0;
        mFlags = 0x8; // set default flag value
        mInventory.mList.clear();
    }
}
