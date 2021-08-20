#include "activespells.hpp"

#include "reader.hpp"
#include "../esm/esmwriter.hpp"

namespace ESM3
{
    void ActiveSpells::load(Reader& esm)
    {
        int format = esm.getFormat();

        while (esm.getSubRecordHeader())
        {
            const ESM3::SubRecordHeader& subHdr = esm.subRecordHeader();
            if (subHdr.typeId == ESM3::SUB_ID__)
            {
                std::string spellId;
                esm.getString(spellId); // TODO: check string not null terminated

                ActiveSpellParams params;
                esm.getSubRecordHeader(ESM3::SUB_CAST);
                esm.get(params.mCasterActorId);

                esm.getSubRecordHeader(ESM3::SUB_DISP);
                esm.getString(params.mDisplayName); // TODO: check string not null terminated

                // spell casting timestamp, no longer used
                if (esm.getNextSubRecordHeader(ESM3::SUB_TIME))
                    esm.skipSubRecordData();

                while (esm.getNextSubRecordHeader(ESM3::SUB_MGEF))
                {
                    ActiveEffect effect;
                    esm.get(effect.mEffectId);
                    effect.mArg = -1;

                    if (esm.getNextSubRecordHeader(ESM3::SUB_ARG_))
                        esm.get(effect.mArg);

                    esm.getSubRecordHeader(ESM3::SUB_MAGN);
                    esm.get(effect.mMagnitude);

                    esm.getSubRecordHeader(ESM3::SUB_DURA);
                    esm.get(effect.mDuration);

                    effect.mEffectIndex = -1;
                    if (esm.getNextSubRecordHeader(ESM3::SUB_EIND))
                        esm.get(effect.mEffectIndex);

                    if (format < 9)
                        effect.mTimeLeft = effect.mDuration;
                    else if (esm.getSubRecordHeader())
                    {
                        if (esm.subRecordHeader().typeId == ESM3::SUB_LEFT)
                        {
                            esm.get(effect.mTimeLeft);
                        }
                    }

                    params.mEffects.push_back(effect);
                }

                mSpells.insert(std::make_pair(spellId, params));
                break;
            }
            else
            {
                esm.cacheSubRecordHeader();
                return;
            }
        }
    }

    void ActiveSpells::save(ESM::ESMWriter& esm) const
    {
        for (TContainer::const_iterator it = mSpells.begin(); it != mSpells.end(); ++it)
        {
            esm.writeHNString ("ID__", it->first);

            const ActiveSpellParams& params = it->second;

            esm.writeHNT ("CAST", params.mCasterActorId);
            esm.writeHNString ("DISP", params.mDisplayName);

            for (std::vector<ActiveEffect>::const_iterator effectIt = params.mEffects.begin(); effectIt != params.mEffects.end(); ++effectIt)
            {
                esm.writeHNT ("MGEF", effectIt->mEffectId);
                if (effectIt->mArg != -1)
                    esm.writeHNT ("ARG_", effectIt->mArg);
                esm.writeHNT ("MAGN", effectIt->mMagnitude);
                esm.writeHNT ("DURA", effectIt->mDuration);
                esm.writeHNT ("EIND", effectIt->mEffectIndex);
                esm.writeHNT ("LEFT", effectIt->mTimeLeft);
            }
        }
    }
}
