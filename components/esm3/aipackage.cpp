#include "aipackage.hpp"

//#ifdef NDEBUG
//#undef NDEBUG
//#endif

#include <cassert>

#include "common.hpp"
#include "reader.hpp"
#include "../esm/esmwriter.hpp"

namespace ESM3
{
    void AIData::blank()
    {
        mHello = mFight = mFlee = mAlarm = mU1 = mU2 = mU3 = 0;
        mServices = 0;
    }

    // NOTE: assumes sub-record header was just read
    void AIPackageList::add(Reader& reader)
    {
        AIPackage pack;
        const ESM3::SubRecordHeader& subHdr = reader.subRecordHeader();
        if (subHdr.typeId == ESM3::SUB_CNDT)
        {
            if (mList.empty())
            {
                reader.fail("AIPackge with an AI_CNDT applying to no cell.");
            } else {
                reader.getZString(mList.back().mCellName);
            }
        }
        else if (subHdr.typeId == ESM3::SUB_AI_W)
        {
            pack.mType = AI_Wander;
            assert (subHdr.dataSize == 14 && "AIPackage wander size mismatch");
            reader.get(pack.mWander);
            mList.push_back(pack);
        }
        else if (subHdr.typeId == ESM3::SUB_AI_T)
        {
            pack.mType = AI_Travel;
            assert (subHdr.dataSize == 16 && "AIPackage travel size mismatch");
            reader.get(pack.mTravel);
            mList.push_back(pack);
        }
        else if (subHdr.typeId == ESM3::SUB_AI_E || subHdr.typeId == ESM3::SUB_AI_F)
        {
            pack.mType = (subHdr.typeId == ESM3::SUB_AI_E) ? AI_Escort : AI_Follow;
            assert (subHdr.dataSize == 48 && "AIPackage escort/follow size mismatch");
            reader.get(pack.mTarget);
            mList.push_back(pack);
        }
        else if (subHdr.typeId == ESM3::SUB_AI_A)
        {
            pack.mType = AI_Activate;
            assert (subHdr.dataSize == 33 && "AIPackage activate size mismatch");
            reader.get(pack.mActivate);
            mList.push_back(pack);
        }
        else
             return; // not AI package related data, so leave
    }

    void AIPackageList::save(ESM::ESMWriter& esm) const
    {
        typedef std::vector<AIPackage>::const_iterator PackageIter;
        for (PackageIter it = mList.begin(); it != mList.end(); ++it) {
            switch (it->mType) {
            case AI_Wander:
                esm.writeHNT("AI_W", it->mWander, sizeof(it->mWander));
                break;

            case AI_Travel:
                esm.writeHNT("AI_T", it->mTravel, sizeof(it->mTravel));
                break;

            case AI_Activate:
                esm.writeHNT("AI_A", it->mActivate, sizeof(it->mActivate));
                break;

            case AI_Escort:
            case AI_Follow: {
                const char *name = (it->mType == AI_Escort) ? "AI_E" : "AI_F";
                esm.writeHNT(name, it->mTarget, sizeof(it->mTarget));
                esm.writeHNOCString("CNDT", it->mCellName);
                break;
            }

            default:
                break;
            }
        }
    }
}
