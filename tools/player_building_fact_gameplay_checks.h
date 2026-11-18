// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
[[gnu::noinline]] static void playerBuildingFactGameplayChecks(swgb::Assets &assets) {
    swgb::Scenario scenario;scenario.map.width=scenario.map.height=64;
    scenario.map.tiles.resize(64*64);scenario.nextUnitId=1;scenario.playerSlotCount=3;
    scenario.civilizations.fill(1);
    for(int i=0;i<3;++i) {
        scenario.players[i].active=true;scenario.players[i].human=i!=1;
        scenario.players[i].civilization=1;scenario.players[i].diplomacy.fill(3);
        scenario.players[i].diplomacy[size_t(i+1)]=0;
    }
    swgb::Game game(assets);std::string error;
    check(game.initScenario(scenario,&error),error.c_str());
    const auto observer=game.spawnObjectForTesting(1,83,2,4,4);
    const auto enemy=game.spawnObjectForTesting(1,70,3,50,50);
    const auto neutral=game.spawnObjectForTesting(1,70,0,55,55);
    check(observer && enemy && neutral,"Known building fact actors exist");
    const std::string source=
        "(defrule (game-time < 2) (players-building-type-count 3 70 == 0)"
        " (cc-players-building-type-count 3 70 == 1) => (set-goal 970 1))"
        "(defrule (game-time >= 2) (players-building-type-count any-enemy 70 == 1)"
        " => (set-goal 971 1))"
        "(defrule (game-time >= 4) (players-building-type-count 3 70 == 1)"
        " (cc-players-building-type-count 3 70 == 0) => (set-goal 972 1))"
        "(defrule (players-building-type-count 3 999 == -1) => (set-goal 973 1))"
        "(defrule (players-building-type-count 0 70 == -1)"
        " (cc-players-building-type-count 0 70 == 1) => (set-goal 974 1))"
        "(defrule (not (cc-players-building-type-count 3 -1 == 0)) => (set-goal 975 1))";
    check(game.loadAiSourceForTesting(2,"known-building-runtime",source,{},&error),error.c_str());
    const auto advance=[&](int ticks){for(int i=0;i<ticks;++i)game.update(.1f,{});};
    advance(10);
    check(game.aiGoalForTesting(2,970)==1 && game.aiGoalForTesting(2,973)==1,
          "Ordinary hostile count excludes unknown buildings; CC fact reads live and unknown category is -1");
    check(game.aiGoalForTesting(2,974)==1 && game.aiGoalForTesting(2,975)==0,
          "Gaia direct counts follow original cache/live split; unsafe negative prototype remains unknown under NOT");
    check(game.moveObjectForTesting(observer,49,50),"Reveal enemy building");
    advance(20);
    check(game.aiGoalForTesting(2,971)==1,
          "Production sight admission enables remembered building fact without a military group");
    check(game.moveObjectForTesting(observer,4,4),"Hide previously observed building");
    advance(2);
    check(game.damageObjectForTesting(enemy,100000),"Destroy the hidden known building");
    // Script replacement must preserve knowledge and its original refresh epoch.
    check(game.loadAiSourceForTesting(2,"known-building-runtime",source,{},&error),error.c_str());
    advance(20);
    check(game.aiGoalForTesting(2,972)==1,
          "Hidden destruction updates CC live count but remembered count survives until periodic refresh, including reload");
    std::puts("PASS: production ordinary/CC building facts, fog admission, selector and retained knowledge");
}
