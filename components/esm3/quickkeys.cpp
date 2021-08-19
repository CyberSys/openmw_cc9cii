#include "quickkeys.hpp"

#include "reader.hpp"
#include "../esm/esmwriter.hpp"

namespace ESM3
{
    // probably no equivalent records in vanilla savefile
    // (called from StateManager::loadGame() via WindowManager::readRecord())
    void QuickKeys::load(Reader& esm)
    {
        while (esm.getSubRecordHeader())
        {
            const ESM3::SubRecordHeader& subHdr = esm.subRecordHeader();
            switch (subHdr.typeId)
            {
                case ESM3::SUB_KEY_:
                {
                    // no longer used, because sub-record hierachies do not work properly in esmreader
                    esm.skipSubRecordData();
                    break;
                }
                case ESM3::SUB_TYPE:
                {
                    QuickKey key;
                    esm.get(key.mType);

                    esm.getSubRecordHeader(ESM3::SUB_ID__);
                    esm.getZString(key.mId);

                    mKeys.push_back(key);
                    break;
                }
                default:
                    esm.skipSubRecordData();
                    break;
            }
        }
    }

    void QuickKeys::save(ESM::ESMWriter& esm) const
    {
        for (std::vector<QuickKey>::const_iterator it = mKeys.begin(); it != mKeys.end(); ++it)
        {
            esm.writeHNT("TYPE", it->mType);
            esm.writeHNString("ID__", it->mId);
        }
    }
}
