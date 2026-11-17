// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "genie_random.h"

namespace swgb {
// 61e71c..61e7c6: callback includes the owner's object update and AI save/reseed wrapper.
template<class UpdateOwner>
void genieRmsPrepasses(uint32_t &randomState,uint32_t normalPlayers,UpdateOwner update) {
    genieRandomDraw(randomState);
    genieRandomDraw(randomState);
    for(uint32_t pass=0;pass<3;++pass)
        for(uint32_t owner=0;owner<=normalPlayers;++owner)update(owner,pass);
    genieRandomDraw(randomState);
}
} // namespace swgb
