// SPDX-License-Identifier: GPL-3.0-or-later
#include "../src/engine/ai_resource_knowledge.h"
#include <cassert>
#include <iostream>
int main() {
    using swgb::AiResourceKnowledge;
    // Independent original590cf8 controls: resource-found-controls-2026-11-18.json.
    for(int cls:{27,30,29}) {
        const int type=cls==27?0:cls==30?2:3;
        AiResourceKnowledge k;k.admit(cls,true,10,10,10);assert(!k.found[type]);
        k.admit(cls,true,11,10,10);assert(!k.found[type]);
        k.admit(cls,true,9,10,10);assert(k.found[type]);
        k.admit(cls,true,100,0,0);assert(k.found[type]);
        AiResourceKnowledge noCenter;noCenter.admit(cls,false,100,0,0);assert(noCenter.found[type]);
    }
    AiResourceKnowledge animals;animals.admit(1,false,0,10,10);assert(!animals.found[0]);
    animals.admit(31,false,0,10,10);assert(!animals.found[1]);
    // Original5856da..585715 terrain branch, separate from object admission.
    for(int terrain=0;terrain<60;++terrain) {
        AiResourceKnowledge k;k.forestTile(terrain,false);assert(!k.found[1]);
        k.forestTile(terrain,true);
        const bool expected=terrain==49 || terrain==50 || terrain==10 || terrain==19 || terrain==18 ||
            terrain==13 || terrain==20 || terrain==21 || terrain==17;
        assert(k.found[1]==expected);
    }
    AiResourceKnowledge sticky;sticky.forestTile(10,true);sticky.forestTile(0,false);assert(sticky.found[1]);
    std::cout<<"AI resource knowledge checks passed\n";
}
