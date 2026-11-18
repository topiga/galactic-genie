// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "../src/engine/ai_military_admission.h"
#include <array>
static void militaryAdmissionChecks() {
    using Group=std::array<int64_t,5>; // id,type,desired,state,flags
    struct Case {
        swgb::AiMilitaryAdmissionSettings settings;
        std::vector<swgb::AiMilitaryAdmissionGroup> initial;
        std::vector<Group> expected;
        uint32_t randomState;
    };
    // Original5aaa20 controls; original constructor and RNG executed.
    // CRTceil and heap allocation were modeled, no object/world callbacks.
    const std::vector<Case> cases={
        {{5,1,1,3,8,3,2},{},{{100,101,1,1,32},{101,100,5,1,32},{102,100,5,1,32}},3357800067u}, // five_soldiers
        {{0,1,1,3,8,3,2},{},{},1u}, // no_soldiers
        {{1,1,1,3,8,3,2},{},{{100,101,1,1,32}},1u}, // explore_reserves_all
        {{7,1,1,3,8,3,2},{},{{100,101,1,1,32},{101,100,5,1,32},{102,100,5,1,32}},3357800067u}, // attack_exact_division
        {{30,1,1,3,8,2,2},{},{{100,101,1,1,32},{101,100,5,1,32},{102,100,5,1,32}},3357800067u}, // attack_group_cap
        {{5,1,1,3,8,0,2},{},{{100,101,1,1,32}},1u}, // disabled_attack
        {{5,1,0,3,8,3,2},{},{{100,100,5,1,32},{101,100,5,1,32}},3357800067u}, // disabled_explore
        {{5,1,1,3,3,3,2},{},{{100,101,1,1,32},{101,100,3,1,32},{102,100,3,1,32}},3357800067u}, // size_cap
        {{5,1,1,3,8,3,0},{},{{100,101,1,1,32},{101,100,3,1,32},{102,100,3,1,32}},3357800067u}, // zero_randomness
        {{5,1,1,3,2,3,2},{},{{100,101,1,1,32},{101,100,2,1,32},{102,100,2,1,32}},3357800067u}, // max_below_min
        {{0,1,1,3,8,3,2},{{10,100,1,0,8}},{},1u}, // empty_forced_removed
        {{1,1,1,3,8,3,2},{{10,100,1,1,8},{11,100,1,1,32}},{{10,100,5,1,8},{11,100,5,1,32},{100,101,1,1,32}},1u}, // forced_first_stops_trim
        {{1,1,1,3,8,3,2},{{10,100,1,1,32},{11,100,1,1,8},{12,100,1,1,32}},{{11,100,5,1,8},{12,100,5,1,32},{100,101,1,1,32}},1u}, // trim_then_forced_stops
        {{1,1,1,3,8,3,2},{{10,100,2,1,32}},{{100,101,1,1,32}},1u}, // active_group_can_trim
        {{1,1,1,3,8,3,2},{{10,101,1,1,32},{11,101,0,1,32}},{{10,101,5,1,32}},1u}, // explore_trim_prefers_state0
        {{5,1,1,3,8,3,2},{{10,112,1,1,32}},{{10,112,5,1,32},{100,101,1,1,32},{101,100,5,1,32},{102,100,5,1,32}},3357800067u}, // other_types_not_normalquota
    };
    for(const auto &c:cases) {
        const auto plan=swgb::planAiMilitaryAdmission(c.initial,c.settings,100,1);
        check(bool(plan),"Valid original group-admission fixture");
        std::vector<Group> result;
        for(const auto &g:c.initial)result.push_back({g.id,g.type,5,g.state,g.flags});
        for(const auto &op:plan->operations) {
            if(op.create)result.push_back({op.id,op.type,op.desired,1,32});
            else result.erase(std::remove_if(result.begin(),result.end(),[&](const Group &g){return g[0]==op.id;}),result.end());
        }
        check(result==c.expected && plan->randomState==c.randomState,"Original ordinary military admission and RNG goldens");
    }
    swgb::AiMilitaryAdmissionSettings invalid;invalid.attackMinimum=0;
    check(!swgb::planAiMilitaryAdmission({},invalid,0,1),"Unclosed invalid divisor remains unsupported");
}
