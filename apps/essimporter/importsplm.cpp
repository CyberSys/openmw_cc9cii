#include "importsplm.h"

//#ifdef NDEBUG
//#undef NDEBUG
//#endif

#include <cassert>
#include <cstring> // memset
#include <iostream>

#include <components/esm3/reader.hpp>

namespace ESSImport
{
    void SPLM::load(ESM3::Reader& esm)
    {
        assert(esm.hdr().typeId == ESM3::REC_SPLM);

        ActiveSpell spell;
        spell.blank();

        while (esm.getSubRecordHeader())
        {
            const ESM3::SubRecordHeader& subHdr = esm.subRecordHeader();
            switch (subHdr.typeId)
            {
                case ESM3::SUB_NAME: esm.get(spell.mIndex); break;
                case ESM3::SUB_SPDT: esm.get(spell.mSPDT);
#if 0
                     std::cout << "type   " << spell.mSPDT.mType << std::endl;
                     std::cout << "effect " << spell.mSPDT.mId.toString() << std::endl;
                     std::cout << "caster " << spell.mSPDT.mCasterId.toString() << std::endl;
                     std::cout << "source " << spell.mSPDT.mSourceId.toString() << std::endl;
#endif
                     break;
                case ESM3::SUB_TNAM: esm.getZString(spell.mTarget); break;
                case ESM3::SUB_NPDT:
                {
                    assert(subHdr.dataSize == sizeof(ActiveEffect::mNPDT));
                    ActiveEffect effect;
                    esm.get(effect.mNPDT);

                    // Effect-specific subrecords can follow:
                    // - INAM for disintegration and bound effects
                    // - CNAM for summoning and command effects
                    // - VNAM for vampirism
                    // NOTE: There can be multiple INAMs per effect.
                    // TODO: Needs more research.

                    spell.mActiveEffects.push_back(effect);
                    break;
                }
                case ESM3::SUB_NAM0: // sentinel
                {
                    esm.skipSubRecordData();
                    break;
                }
                case ESM3::SUB_XNAM:
                {
                    unsigned char xnam; // sentinel
                    esm.get(xnam);
                    mActiveSpells.push_back(spell);
                    spell.blank();

                    break;
                }
                default:
                    esm.skipSubRecordData();
                    break;
            }
        }
    }
}
