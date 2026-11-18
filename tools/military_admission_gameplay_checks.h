// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "military_admission_checks.h"
[[gnu::noinline]] static void militaryAdmissionGameplayChecks(swgb::Assets &assets) {
    militaryAdmissionChecks();
    swgb::Scenario scenario;scenario.map.width=scenario.map.height=64;
    scenario.map.tiles.resize(64*64);scenario.nextUnitId=1;
    scenario.playerSlotCount=2;scenario.civilizations.fill(1);
    for(int i=0;i<2;++i){scenario.players[i].active=true;scenario.players[i].human=i==0;scenario.players[i].civilization=1;}
    const auto setup=[&](swgb::Game &game,int quota) {
        std::string error;check(game.initScenario(scenario,&error),error.c_str());
        check(game.spawnObjectForTesting(1,109,2,32,32)!=0,"Military admission home fixture");
        for(int i=0;i<5;++i)check(game.spawnObjectForTesting(1,329,2,27.f+i*.5f,28)!=0,"Unassigned soldier fixture");
        const std::string source="(defrule (true) => "
            "(set-strategic-number sn-number-explore-groups 0) "
            "(set-strategic-number sn-minimum-attack-group-size 2) "
            "(set-strategic-number sn-maximum-attack-group-size 3) "
            "(set-strategic-number sn-attack-group-size-randomness 0) "
            "(set-strategic-number sn-number-attack-groups "+std::to_string(quota)+") (disable-self))";
        check(game.loadAiSourceForTesting(2,"ordinary-group-quota",source,{},&error),error.c_str());
    };
    swgb::Game ordinary(assets);setup(ordinary,2);
    bool populated=false;
    for(int i=0;i<100 && !populated;++i) {
        ordinary.update(0,{});
        int count=0,members=0;
        for(const auto &group:ordinary.militaryGroupsForTesting(2))if(group.type==100){++count;members+=int(group.members.size());}
        populated=count==2 && members==4;
    }
    check(populated,"PER group quotas form and populate normal attack groups without attack-now");
    for(const auto &group:ordinary.militaryGroupsForTesting(2))if(group.type==100)
        check(group.desiredCount==2 && !(group.flags&8u),"Ordinary group size and unforced flag follow original admission");

    swgb::Game disabled(assets);setup(disabled,0);
    for(int i=0;i<100;++i)disabled.update(0,{});
    check(disabled.militaryGroupsForTesting(2).empty(),"Zero attack-group quota does not invent aggression");
    check(disabled.formAiMilitaryGroupsForTesting(2),"Create explicit attack-now groups with the original protection flag");
    std::vector<int32_t> protectedIds;
    for(const auto &group:disabled.militaryGroupsForTesting(2))if(group.type==100 && (group.flags&8u))protectedIds.push_back(group.id);
    check(!protectedIds.empty(),"Explicit attack-now created a protected group");
    for(int i=0;i<100;++i)disabled.update(0,{});
    for(int32_t id:protectedIds)
        check(std::any_of(disabled.militaryGroupsForTesting(2).begin(),disabled.militaryGroupsForTesting(2).end(),
            [&](const auto &group){return group.id==id && (group.flags&8u);}),
            "Ordinary zero-quota trimming preserves an explicit attack-now group");
}
