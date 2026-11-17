// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "genie_random.h"
namespace swgb {
// Leaf standing graphic before generic AI creation; saved scenario frame/facing replace the results.
inline void genieScenarioGraphicConstructorRandom(uint32_t &state,uint8_t flags,int angles) {
    if((flags&4) && angles>0)genieRandomDraw(state); // 54c44f base facet
    if(flags&4) {
        genieRandomDraw(state); // 423742 sprite base frame
        if(flags&1)genieRandomDraw(state); // 423c09/423c63 animated phase
    }
    if((flags&2) && angles>1)genieRandomDraw(state); // 4a099a heading
}
}
