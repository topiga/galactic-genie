// SPDX-License-Identifier: GPL-3.0-or-later
#include "../src/engine/ai_builder_selection.h"
#include <array>
#include <cassert>
#include <iostream>
// Actual5a7c10 +551c60 controls: build-worker-selection-controls-2026-11-18.json.
int main() {
    using swgb::AiBuilderCandidate;
    auto candidates=[] {return std::array<AiBuilderCandidate,2>{{{1,83},{2,83}}};};
    auto pick=[](const auto &list,double second=2.) {
        return swgb::aiSelectBuilder(list,[&](const auto &worker){return worker.id==1?1.:second;});
    };
    assert(pick(candidates())==1);assert(pick(candidates(),1.)==1);assert(pick(candidates(),0.)==2);
    auto list=candidates();list[0].reservedInCurrentPass=true;assert(pick(list)==2);
    list=candidates();list[0].carriedAmount=7;assert(pick(list)==1);
    list[0].carriedAmount=7.1f;assert(pick(list)==2);
    for(int prototype:{259,214}) {list=candidates();list[0].prototype=prototype;assert(pick(list)==2);}
    for(int action:{702,718,700,717}) {list=candidates();list[0].controllerType=action;assert(pick(list)==2);}
    list=candidates();list[0].hasController=false;assert(pick(list)==2);
    list=candidates();list[0].resolved=false;assert(pick(list)==2);
    list=candidates();for(auto &worker:list)worker.reservedInCurrentPass=true;assert(pick(list)==-1);
    std::cout<<"Original builder selection checks passed\n";
}
