#include "activespells.hpp"

//#ifdef NDEBUG
//#undef NDEBUG
//#endif

#include <cassert>

#include "reader.hpp"
#include "../esm/esmwriter.hpp"

namespace ESM3
{
    void ActiveSpells::load(Reader& esm)
    {
        int format = esm.getFormat();

        bool subDataRemaining = false;
        while (subDataRemaining || esm.getSubRecordHeader())
        {
            const ESM3::SubRecordHeader& subHdr = esm.subRecordHeader();
            if (subHdr.typeId == ESM3::SUB_ID__)
            {
                std::string spellId;
                esm.getZString(spellId);

                ActiveSpellParams params;
                esm.getSubRecordHeader();
                assert(esm.subRecordHeader().typeId == ESM3::SUB_CAST);
                esm.get(params.mCasterActorId);

                esm.getSubRecordHeader();
                assert(esm.subRecordHeader().typeId == ESM3::SUB_DISP);
                esm.getZString(params.mDisplayName);

                // spell casting timestamp, no longer used
                if (esm.getNextSubRecordType() == ESM3::SUB_TIME && esm.getSubRecordHeader())
                    esm.skipSubRecordData();

                while (esm.getSubRecordHeader())
                {
                    subDataRemaining = false;
                    const ESM3::SubRecordHeader& subHdr2 = esm.subRecordHeader();
                    if (subHdr2.typeId == ESM3::SUB_MGEF)
                    {
                        ActiveEffect effect;
                        esm.get(effect.mEffectId);
                        effect.mArg = -1;

                        if (esm.getNextSubRecordType() == ESM3::SUB_ARG_ && esm.getSubRecordHeader())
                            esm.get(effect.mArg);

                        esm.getSubRecordHeader();
                        assert(esm.subRecordHeader().typeId == ESM3::SUB_MAGN);
                        esm.get(effect.mMagnitude);

                        esm.getSubRecordHeader();
                        assert(esm.subRecordHeader().typeId == ESM3::SUB_DURA);
                        esm.get(effect.mDuration);

                        effect.mEffectIndex = -1;
                        esm.getSubRecordHeader();
                        if (esm.subRecordHeader().typeId == ESM3::SUB_EIND)
                        {
                            esm.get(effect.mEffectIndex);
                            esm.getSubRecordHeader();
                        }
                        else
                            subDataRemaining = true;

                        if (format < 9)
                            effect.mTimeLeft = effect.mDuration;
                        else if (subDataRemaining || esm.getSubRecordHeader())
                        {
                            if (esm.subRecordHeader().typeId == ESM3::SUB_LEFT)
                            {
                                esm.get(effect.mTimeLeft);
                                subDataRemaining = false;
                            }
                        }

                        params.mEffects.push_back(effect);
                    }
                    else
                    {
                        subDataRemaining = true;
                        break;
                    }
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
