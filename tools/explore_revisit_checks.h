// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "../src/core/genie_explore_revisit.h"
static void exploreRevisitChecks() {
    struct Case {uint32_t now,previous,candidates;bool sweep;uint32_t after;};
    // Original585ba8 instruction controls on10x10 information grids.
    const Case cases[]={{120000,0,0,false,0},{120001,0,0,true,120001},
        {120001,0,4,true,120001},{120001,0,5,true,120001},{120001,0,6,false,120001},
        {0,33815,0,true,0},{0,33815,100,false,0},{33815,33815,0,false,33815},{0,0,0,false,0}};
    for(const auto &c:cases) {
        uint32_t timestamp=c.previous;
        check(swgb::genieExploreRevisitDue(c.now,timestamp,c.candidates,100)==c.sweep && timestamp==c.after,
              "Original periodic exploration threshold and timestamp control");
    }
}
