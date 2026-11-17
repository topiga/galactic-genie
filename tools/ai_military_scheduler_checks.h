// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "../src/engine/ai_military_scheduler.h"

static void aiMilitarySchedulerChecks() {
    using namespace swgb;
    const auto fresh=[] {
        return std::vector<AiMilitaryGatherGroup>{{10,1,1,{1}},{20,1,1,{2}},{30,1,1,{3}}};
    };
    auto groups=fresh();
    groups[0].members={-1,1,2};groups[1].destination={9,8,7};groups[2].state=2;
    std::vector<int> lookups;
    refreshAiGatherDestinations(groups,[&](int32_t id)->std::optional<std::array<float,3>> {
        lookups.push_back(id);
        if(id==2)return std::nullopt;
        return std::array<float,3>{1,2,3};
    });
    check(lookups==std::vector<int>{1,2} && groups[0].destination==std::array<float,3>{1,2,3} &&
          groups[1].destination==std::array<float,3>{9,8,7},
          "Original phase7 refresh uses the first nonnegative member; missing lookup preserves destination");

    // Original tactical-readiness-pass-controls-2026-11-18.json, modeled readiness/clock boundaries.
    for(int test=0;test<8;++test) {
        groups=fresh();AiMilitaryReadinessPass pass;
        if(test==2)pass.nextGroupId=20;
        if(test==3)pass.nextGroupId=99;
        if(test==4)groups[0].state=2;
        if(test==5)groups[0].retries=11;
        if(test==6)groups[0].retries=10;
        std::vector<int> readyIds,stopped,gathered;
        int remembered=0,clocks=0;
        const auto result=pass.advance(groups,0,5,[&] {return uint32_t(test==1?5:++clocks);},
            [&](const auto &group) {readyIds.push_back(group.id);return group.id!=10 || (test!=5 && test!=6);},
            [&](auto &group) {gathered.push_back(group.id);},
            [&](int32_t id) {if(test!=7 || id!=1)stopped.push_back(id);},
            [&] {++remembered;});
        if(test==1) {
            check(result==AiMilitaryReadinessResult::BudgetExhausted && pass.nextGroupId==20 &&
                  stopped==std::vector<int>{1} && groups[1].state==1,
                  "Original exact host budget retains the next ordered group ID");
        } else {
            check(result==AiMilitaryReadinessResult::Complete && pass.nextGroupId==-1,
                  "Original completed readiness pass clears its continuation");
            check(groups[0].state==((test==2 || test==6)?1:2) && groups[1].state==2 && groups[2].state==2,
                  "Original readiness continuation and retry branches preserve group states");
        }
        if(test==2 || test==4 || test==5)
            check(std::find(readyIds.begin(),readyIds.end(),10)==readyIds.end(),
                  "Resume, completed group and retry override skip unnecessary readiness callbacks");
        if(test==6)check(gathered==std::vector<int>{10},"Retry ten still gathers without advancing to state2");
        if(test==7)check(stopped==std::vector<int>({2,3}) && remembered==3,
                        "Missing members do not block type100 readiness transition or remembered-count callback");
    }
    groups=fresh();groups[0].desiredCount=2;
    AiMilitaryReadinessPass pass;int calls=0;
    const auto unsupported=pass.advance(groups,0,5,[&]{++calls;return 0u;},
        [&](const auto &){++calls;return true;},[&](auto &){++calls;},[&](int32_t){++calls;},[&]{++calls;});
    check(unsupported==AiMilitaryReadinessResult::Unsupported && calls==0 && pass.nextGroupId==10 && groups[0].state==1,
          "Unrecovered count-deficit branches are explicit and do not issue approximate commands");
    groups=fresh();pass.nextGroupId=-1;
    const auto wrapped=pass.advance(groups,0xfffffffeu,3,[]{return 1u;},
        [](const auto &){return true;},[](auto &){},[](int32_t){},[]{});
    check(wrapped==AiMilitaryReadinessResult::BudgetExhausted && pass.nextGroupId==20,
          "Original unsigned host-clock subtraction survives wraparound");
}
