#include "importscri.hpp"

#include <components/esm3/reader.hpp>

namespace ESSImport
{
    // called from SCPT::load(), Inventory::load(), etc, and sub-record header was already read
    bool SCRI::load(ESM3::Reader& esm)
    {
        int numShorts = 0, numLongs = 0, numFloats = 0;

        bool subDataRemaining = true;
        while (subDataRemaining || esm.getSubRecordHeader())
        {
            subDataRemaining = false;
            const ESM3::SubRecordHeader& subHdr = esm.subRecordHeader();
            switch (subHdr.typeId)
            {
                case ESM3::SUB_SCRI:
                {
                    esm.getZString(mScript);
                    break;
                }
                case ESM3::SUB_SLCS:
                {
                    esm.get(numShorts);
                    esm.get(numLongs);
                    esm.get(numFloats);
                    break;
                }
                case ESM3::SUB_SLSD:
                {
                    for (int i = 0; i < numShorts; ++i)
                    {
                        short val;
                        esm.get(val);
                        mShorts.push_back(val);
                    }
                    break;
                }
                case ESM3::SUB_SLLD:
                {
                    // I haven't seen Longs in a save file yet, but SLLD would make sense for the name
                    // TODO: test this
                    for (int i = 0; i < numLongs; ++i)
                    {
                        int val;
                        esm.get(val);
                        mLongs.push_back(val);
                    }
                    break;
                }
                case ESM3::SUB_SLFD:
                {
                    for (int i = 0; i < numFloats; ++i)
                    {
                        float val;
                        esm.get(val);
                        mFloats.push_back(val);
                    }
                    break;
                }
                default:
                    // continue loading in SCPT::load(), Inventory::load(), etc
                    return true;
            }
        }

        return false;
    }
 }
