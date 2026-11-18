// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
[[gnu::noinline]] static void militaryDifficultyGameplayChecks(swgb::Assets &assets) {
    for(int difficulty:{2,3,4}) {
        swgb::Scenario scenario;scenario.map.width=scenario.map.height=64;
        scenario.map.tiles.resize(64*64);scenario.nextUnitId=1;
        scenario.playerSlotCount=2;scenario.civilizations.fill(1);
        for(int i=0;i<2;++i){scenario.players[i].active=true;scenario.players[i].human=i==0;scenario.players[i].civilization=1;}
        swgb::Game game(assets);std::string error;
        check(game.initScenario(scenario,&error,difficulty),error.c_str());
        check(game.spawnObjectForTesting(1,109,2,32,32)!=0,"Difficulty admission home fixture");
        const auto trooper=game.spawnObjectForTesting(1,329,2,27,28);
        const auto jedi=game.spawnObjectForTesting(1,180,2,28,28);
        check(trooper && jedi,"Difficulty admission soldier/Jedi fixture");
        check(game.loadAiSourceForTesting(2,"difficulty-admission","(defrule (true) => (disable-self))",{},&error),error.c_str());
        check(game.formAiMilitaryGroupsForTesting(2),"Original attack-pool grouping succeeds");
        bool containsTrooper=false,containsJedi=false;
        for(const auto &group:game.militaryGroupsForTesting(2))for(const auto member:group.members) {
            containsTrooper|=member==int32_t(trooper);containsJedi|=member==int32_t(jedi);
        }
        check(containsTrooper,"Ordinary soldier stays eligible at every tested difficulty");
        check(containsJedi==(difficulty==2),"Class50 enters Moderate attack pool and is excluded on Easy/Easiest");
    }
}
