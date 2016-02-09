/*
  Copyright (C) 2016 cc9cii

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  cc9cii cc9c@iinet.net.au

*/
#ifndef ESM4_GRUP_H
#define ESM4_GRUP_H

#include <vector>

#include "common.hpp" // GroupLabel

namespace ESM4
{
    // http://www.uesp.net/wiki/Tes4Mod:Mod_File_Format#Hierarchical_Top_Groups
    //
    //  Type | Info                                 |
    // ------+--------------------------------------+-------------------
    //   2   | Interior Cell Block                  |
    //   3   |   Interior Cell Sub-Block            |
    //     R |     CELL                             |
    //   6   |     Cell Childen                     |
    //   8   |       Persistent children            |
    //     R |         REFR, ACHR, ACRE             |
    //  10   |       Visible distant children       |
    //     R |         REFR, ACHR, ACRE             |
    //   9   |       Temp Children                  |
    //     R |         PGRD                         |
    //     R |         REFR, ACHR, ACRE             |
    //       |                                      |
    //   0   | Top (Type)                           |
    //     R |   WRLD                               |
    //   1   |   World Children                     |
    //     R |     ROAD                             |
    //     R |     CELL                             |
    //   6   |     Cell Childen                     |
    //   8   |       Persistent children            |
    //     R |         REFR, ACHR, ACRE             |
    //  10   |       Visible distant children       |
    //     R |         REFR, ACHR, ACRE             |
    //   9   |       Temp Children                  |
    //     R |         PGRD                         |
    //     R |         REFR, ACHR, ACRE             |
    //   4   |       Exterior World Block           |
    //   5   |         Exterior World Sub-block     |
    //     R |           CELL                       |
    //   6   |           Cell Childen               |
    //   8   |             Persistent children      |
    //     R |               REFR, ACHR, ACRE       |
    //  10   |             Visible distant children |
    //     R |               REFR, ACHR, ACRE       |
    //   9   |             Temp Children            |
    //     R |               LAND                   |
    //     R |               PGRD                   |
    //     R |               REFR, ACHR, ACRE       |
    //
    struct WorldGroup
    {
        FormId mWorld; // WRLD record for this group

        // occurs only after World Child (type 1)
        // since GRUP label may not be reliable, need to keep the formid of the current WRLD in
        // the reader's context
        FormId mRoad;

        std::vector<FormId> mCells;

        WorldGroup() : mWorld(0), mRoad(0) {}
    };

    // http://www.uesp.net/wiki/Tes4Mod:Mod_File_Format/CELL
    //
    // The block and subblock groups for an interior cell are determined by the last two decimal
    // digits of the lower 3 bytes of the cell form ID (the modindex is not included in the
    // calculation). For example, for form ID 0x000CF2=3314, the block is 4 and the subblock is 1.
    //
    // The block and subblock groups for an exterior cell are determined by the X-Y coordinates of
    // the cell. Each block contains 16 subblocks (4x4) and each subblock contains 64 cells (8x8).
    // So each block contains 1024 cells (32x32).
    //
    // NOTE: There may be many CELL records in one subblock
    struct CellGroup
    {
        FormId mCell; // CELL record for this group

        // for retrieving parent group size (for lazy loading or skipping) and sub-block number / grid
        GroupTypeHeader mHeader;

        // FIXME: should these be pairs?  i.e. <FormId, modindex> so that we know from which file
        //        the formid came (it may have been updated by a mod)
        //        but does it matter?  the record itself keeps track of whether it is base,
        //        added or modified anyway
        // FIXME: should these be maps? e.g. std::map<FormId, std::uint8_t>
        //        or vector for storage with a corresponding map of index?

        // cache (modindex adjusted) formId's of children
        GroupTypeHeader mHdrPersist;
        std::vector<FormId> mPersistent;     // REFR, ACHR, ACRE

        GroupTypeHeader mHdrVisDist;
        std::vector<FormId> mVisibleDistant; // REFR, ACHR, ACRE

        GroupTypeHeader mHdrTemp;
        FormId mLand; // if present, assume only one LAND per exterior CELL
        FormId mPgrd; // if present, seems to be the first record after LAND in Temp Cell Child GRUP
        std::vector<FormId> mTemporary;      // REFR, ACHR, ACRE

        // need to keep modindex and context for lazy loading (of all the files that contribute
        // to this group)

        CellGroup() : mCell(0), mLabel.value(0), mLand(0) {}
    };
}

#endif // ESM4_GRUP_H
