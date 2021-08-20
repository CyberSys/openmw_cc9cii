#include "globalscript.hpp"

#include "reader.hpp"
#include "../esm/esmwriter.hpp"

namespace ESM3
{
    // NOTE: equivalent to REC_SCPT where "NAME" is SUB_SCHD "TARG" is related to SUB_RNAM
    // NOTE: OpenMW extension: "RUN_" which is implied by the existance of SUB_RNAM
    // (called from StateManager::loadGame() via ScriptManager::getGlobalScripts() and
    //  GlobalScripts::readRecord())
    void ESM3::GlobalScript::load (Reader& esm)
    {
        mRunning = 0;
        mTargetRef.unset();

        while (esm.getSubRecordHeader())
        {
            const ESM3::SubRecordHeader& subHdr = esm.subRecordHeader();
            switch (subHdr.typeId)
            {
                case ESM3::SUB_NAME:
                {
                    esm.getString(mId); // NOTE: string not null terminated

                    mLocals.load (esm);
                    break;
                }
                case ESM3::SUB_RUN_:
                {
                    esm.get(mRunning);
                    break;
                }
                case ESM3::SUB_TARG: // optional
                {
                    esm.getString(mTargetId); // NOTE: string not null terminated
                    break;
                }
                case ESM3::SUB_FRMR: // optional
                {
                    mTargetRef.load(esm, true/*wide*/, ESM3::SUB_FRMR);
                    break;
                }
                default:
                    esm.skipSubRecordData();
                    break;
            }
        }
    }

    void ESM3::GlobalScript::save (ESM::ESMWriter& esm) const
    {
        esm.writeHNString ("NAME", mId);

        mLocals.save (esm);

        if (mRunning)
            esm.writeHNT ("RUN_", mRunning);

        if (!mTargetId.empty())
        {
            esm.writeHNOString ("TARG", mTargetId);
            if (mTargetRef.isSet())
                mTargetRef.save (esm, true, "FRMR");
        }
    }
}
