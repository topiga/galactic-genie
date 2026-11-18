// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "../src/core/genie_random.h"

static void explorationReissueGameplayChecks(swgb::Assets &assets) {
    swgb::Scenario scenario;scenario.map.width=scenario.map.height=64;scenario.map.tiles.resize(64*64);
    scenario.nextUnitId=1;scenario.playerSlotCount=2;scenario.civilizations.fill(1);
    for(int i=0;i<2;++i){scenario.players[i].active=true;scenario.players[i].human=i==0;scenario.players[i].civilization=1;}
    swgb::Game game(assets);std::string error;
    check(game.initScenario(scenario,&error),error.c_str());
    const auto home=game.spawnObjectForTesting(1,109,2,30,30);
    const auto scout=game.spawnObjectForTesting(1,550,2,31,31);
    check(home && scout,"Exploration notification fixtures");
    check(game.loadAiSourceForTesting(2,"explore-reissue",R"(
(defrule (true) => (set-strategic-number sn-minimum-explore-group-size 1)
 (set-strategic-number sn-number-explore-groups 1) (disable-self))
)",{},&error),error.c_str());
    for(int i=0;i<8 && game.explorationStateForTesting(scout)!=11;++i) {
        game.update(0,{});
    }
    check(game.explorationStateForTesting(scout)==11,"Initial concrete exploration child starts");
    const auto first=game.explorationDestinationForTesting(scout);
    bool requested=false;
    for(int i=0;i<2400;++i) {
        game.update(.05f,{});
        if(game.explorationStateForTesting(scout)==3){requested=true;break;}
    }
    check(requested,"Reaching a waypoint requests another destination through the retained exploration parent");
    const auto position=game.objectPosition(scout);
    game.updatePaused(1,{});
    check(game.objectPosition(scout)==position && game.explorationStateForTesting(scout)==3 &&
          game.explorationDestinationForTesting(scout)==first,"Pause holds the queued exploration reissue");
    game.update(0,{});
    const auto second=game.explorationDestinationForTesting(scout);
    check(game.explorationStateForTesting(scout)==11 && second!=first,
          "Notification spiral and ring retry dispatch a new concrete705 child");
    bool third=false;
    for(int i=0;i<2400;++i) {
        game.update(.05f,{});
        if(game.explorationStateForTesting(scout)==11 && game.explorationDestinationForTesting(scout)!=second){third=true;break;}
    }
    check(third,"An initial singleton scout continues beyond two waypoints");
    uint32_t seed=12345;const uint32_t expected=swgb::genieRandomDraw(seed);
    game.setAmbientRandomForTesting(12345);
    check(game.reissueExploreForTesting(scout),"Controlled live notification can enqueue a destination");
    check(game.legacyRandomForTesting()==expected,"Player514 callback restores the saved rand value after all nested search draws");
    std::puts("PASS: singleton exploration arrival, retained parent, pause, successive destinations and notification RNG");
}
