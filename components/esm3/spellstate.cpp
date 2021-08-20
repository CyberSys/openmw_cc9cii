#include "spellstate.hpp"

#include "reader.hpp"
#include "../esm/esmwriter.hpp"

namespace ESM3
{
    void SpellState::load(Reader& esm)
    {
        mSelectedSpell = "";

        bool finished = false;
        while (!finished && esm.getSubRecordHeader())
        {
            const ESM3::SubRecordHeader& subHdr = esm.subRecordHeader();
            switch (subHdr.typeId)
            {
                case ESM3::SUB_SPEL:
                {
                    std::string id;
                    esm.getString(id); // NOTE: string not null terminated

                    SpellParams state;

                    while (esm.getNextSubRecordHeader(ESM3::SUB_INDX))
                    {
                        int index;
                        esm.get(index);

                        float magnitude;
                        esm.getSubRecordHeader(ESM3::SUB_RAND);
                        esm.get(magnitude);

                        state.mEffectRands[index] = magnitude;
                    }

                    while (esm.getNextSubRecordHeader(ESM3::SUB_PURG))
                    {
                        int index;
                        esm.get(index);
                        state.mPurgedEffects.insert(index);
                    }

                    mSpells[id] = state;
                    break;
                }
                case ESM3::SUB_PERM: // Obsolete
                {
                    std::string spellId;
                    esm.getString(spellId); // TODO: check string not null terminated
                    std::vector<PermanentSpellEffectInfo> permEffectList;

                    while (true)
                    {
                        // FIXME: how to test? Prob need an old version save file
                        ReaderContext restorePoint = esm.getContext();

                        if (esm.getNextSubRecordType() != ESM3::SUB_EFID)
                            break;

                        PermanentSpellEffectInfo info;
                        esm.getSubRecordHeader();
                        esm.get(info.mId);
                        if (esm.getNextSubRecordType() == ESM3::SUB_BASE)
                        {
                            esm.restoreContext(restorePoint);
                            return;
                        }
                        else
                        {
                            esm.getSubRecordHeader(ESM3::SUB_ARG_);
                            esm.get(info.mArg);
                        }

                        esm.getSubRecordHeader(ESM3::SUB_MAGN);
                        esm.get(info.mMagnitude);

                        permEffectList.push_back(info);
                    }

                    mPermanentSpellEffects[spellId] = permEffectList;
                    break;
                }
                case ESM3::SUB_CORP: // Obsolete
                {
                    std::string id;
                    esm.getString(id); // TODO: check string not null terminated

                    CorprusStats stats;
                    esm.getSubRecordHeader(ESM3::SUB_WORS);
                    esm.get(stats.mWorsenings);

                    esm.getSubRecordHeader(ESM3::SUB_TIME);
                    esm.get(stats.mNextWorsening);

                    mCorprusSpells[id] = stats;
                    break;
                }
                case ESM3::SUB_USED:
                {
                    std::string id;
                    esm.getString(id); // TODO: check string not null terminated
                    ESM::TimeStamp time;

                    esm.getSubRecordHeader(ESM3::SUB_TIME);
                    esm.get(time);

                    mUsedPowers[id] = time;
                    break;
                }
                case ESM3::SUB_SLCT:
                {
                    esm.getString(mSelectedSpell); // TODO: check string not null terminated
                    finished = true;
                    break;
                }
                default:
                    finished = true;
                    esm.cacheSubRecordHeader();
                    break;
            }
        }
    }

    void SpellState::save(ESM::ESMWriter& esm) const
    {
        for (TContainer::const_iterator it = mSpells.begin(); it != mSpells.end(); ++it)
        {
            esm.writeHNString("SPEL", it->first);

            const std::map<int, float>& random = it->second.mEffectRands;
            for (std::map<int, float>::const_iterator rIt = random.begin(); rIt != random.end(); ++rIt)
            {
                esm.writeHNT("INDX", rIt->first);
                esm.writeHNT("RAND", rIt->second);
            }

            const std::set<int>& purges = it->second.mPurgedEffects;
            for (std::set<int>::const_iterator pIt = purges.begin(); pIt != purges.end(); ++pIt)
                esm.writeHNT("PURG", *pIt);
        }

        for (std::map<std::string, CorprusStats>::const_iterator it = mCorprusSpells.begin(); it != mCorprusSpells.end(); ++it)
        {
            esm.writeHNString("CORP", it->first);

            const CorprusStats & stats = it->second;
            esm.writeHNT("WORS", stats.mWorsenings);
            esm.writeHNT("TIME", stats.mNextWorsening);
        }

        for (std::map<std::string, ESM::TimeStamp>::const_iterator it = mUsedPowers.begin(); it != mUsedPowers.end(); ++it)
        {
            esm.writeHNString("USED", it->first);
            esm.writeHNT("TIME", it->second);
        }

        if (!mSelectedSpell.empty())
            esm.writeHNString("SLCT", mSelectedSpell);
    }
}
