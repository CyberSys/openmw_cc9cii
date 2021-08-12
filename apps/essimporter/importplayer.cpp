#include "importplayer.hpp"

#ifdef NDEBUG
#undef NDEBUG
#endif

#include <cassert>

#include <components/esm3/reader.hpp>

namespace ESSImport
{

    void REFR::load(ESM3::Reader& esm)
    {
        assert(esm.hdr().typeId == ESM3::REC_REFR);
        esm.getSubRecordHeader();
        assert(esm.subRecordHeader().typeId == ESM3::SUB_FRMR);
        esm.get(mRefNum.mIndex);

        esm.getSubRecordHeader();
        assert(esm.subRecordHeader().typeId == ESM3::SUB_NAME);
        esm.getZString(mRefID);

        mActorData.load(esm);

        if (esm.hasMoreSubs())
        {
            esm.getSubRecordHeader();
            assert(esm.subRecordHeader().typeId == ESM3::SUB_DATA);
            esm.get(mPos,24);
        }
    }

    void PCDT::load(ESM3::Reader& esm)
    {
        assert(esm.hdr().typeId == ESM3::REC_PCDT);
        mHasMark = false;
        mHasENAM = false;
        mHasAADT = false;
        mBounty = 0;

        while (esm.getSubRecordHeader())
        {
            const ESM3::SubRecordHeader& subHdr = esm.subRecordHeader();
            switch (subHdr.typeId)
            {
                case ESM3::SUB_DNAM:
                {
                    std::string topic;
                    esm.getZString(topic);
                    mKnownDialogueTopics.push_back(topic);
                    break;
                }
                case ESM3::SUB_MNAM:
                {
                    mHasMark = true;
                    esm.getZString(mMNAM);
                    break;
                }
                case ESM3::SUB_PNAM:
                {
                    esm.get(mPNAM);
                    break;
                }
                case ESM3::SUB_SNAM:
                case ESM3::SUB_NAM9:
                case ESM3::SUB_LNAM:
                case ESM3::SUB_KNAM: // assigned Quick Keys, I think
                case ESM3::SUB_ANIS: // 16 bytes
                {
                    esm.skipSubRecordData();
                    break;
                }
                case ESM3::SUB_RNAM:
                {
                    // Rest state. You shouldn't even be able to save during rest, but skip just in case.
                    /*
                        int hoursLeft;
                        float x, y, z; // resting position
                    */
                    assert(subHdr.dataSize == 16);
                    esm.skipSubRecordData();
                    break;
                }
                case ESM3::SUB_CNAM:
                {
                    esm.get(mBounty);
                    break;
                }
                case ESM3::SUB_BNAM:
                {
                    esm.getZString(mBirthsign);
                    break;
                }
                // Holds the names of the last used Alchemy apparatus. Don't need to import this ATM,
                // because our GUI auto-selects the best apparatus.
                case ESM3::SUB_NAM0:
                case ESM3::SUB_NAM1:
                case ESM3::SUB_NAM2:
                case ESM3::SUB_NAM3:
                {
                    esm.skipSubRecordData();
                    break;
                }
                case ESM3::SUB_ENAM:
                {
                    mHasENAM = true;
                    esm.get(mENAM);
                    break;
                }
                case ESM3::SUB_FNAM:
                {
                    FNAM fnam;
                    esm.get(fnam);
                    mFactions.push_back(fnam);
                    break;
                }
                case ESM3::SUB_AADT: // Attack animation data?
                {
                    mHasAADT = true;
                    esm.get(mAADT);
                    break;
                }
                case ESM3::SUB_WERE:
                {
                    // some werewolf data, 152 bytes
                    // maybe current skills and attributes for werewolf form
                    assert(subHdr.dataSize == 152 && "WERE incorrect data size");
                    esm.skipSubRecordData();
                    break;
                }
                default:
                    esm.fail("Unknown subrecord");
                    break;
            }
        }
    }
}
