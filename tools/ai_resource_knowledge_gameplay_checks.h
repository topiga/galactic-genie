// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

static void aiResourceKnowledgeGameplayChecks(swgb::Assets &assets) {
    swgb::Scenario scenario;scenario.map.width=scenario.map.height=64;scenario.map.tiles.resize(64*64);
    scenario.nextUnitId=1;scenario.playerSlotCount=2;scenario.civilizations.fill(1);
    for(int i=0;i<2;++i) {
        scenario.players[i].active=true;scenario.players[i].human=i==0;scenario.players[i].civilization=1;
        scenario.players[i].diplomacy.fill(3);scenario.players[i].diplomacy[size_t(i+1)]=0;
    }
    auto unitForClass=[&](int cls) {
        for(const auto &unit:assets.dat().civs[0].units)
            if(unit.cls==cls && unit.id>=0 && unit.standingGraphic[0]>=0)return int(unit.id);
        return -1;
    };
    const int berry=unitForClass(27),ore=unitForClass(30),nova=unitForClass(29),animal=unitForClass(1);
    check(berry>=0 && ore>=0 && nova>=0 && animal>=0,"Original resource classes exist in loaded Clone Campaigns data");
    swgb::Game game(assets);std::string error;
    auto reset=[&] {check(game.initScenario(scenario,&error),error.c_str());};
    auto load=[&](const std::string &source) {
        check(game.loadAiSourceForTesting(2,"resource-found-runtime",source,{},&error),error.c_str());
    };
    auto advance=[&](int ticks) {for(int n=0;n<ticks;++n)game.update(.1f,{});};
    reset();
    const auto observer=game.spawnObjectForTesting(1,83,2,20,20);
    check(observer && game.spawnObjectForTesting(0,berry,0,21,20) &&
          game.spawnObjectForTesting(0,ore,0,20,21) && game.spawnObjectForTesting(0,nova,0,19,20),
          "Visible neutral berry and mineral fixtures");
    load("(defrule (resource-found food) (resource-found metal) (resource-found nova) => (set-goal 930 1))"
         "(defrule (game-time >= 2) (resource-found food) (resource-found metal) (resource-found nova) => (set-goal 931 1))");
    advance(10);
    check(game.aiGoalForTesting(2,930)==1,"Production visible index admits berries and both minerals to PER facts");
    check(game.moveObjectForTesting(observer,3,3),"Remove resource sight source");
    advance(25);
    check(game.aiGoalForTesting(2,931)==1,"Production resource-found stays true after losing sight");

    reset();
    check(game.spawnObjectForTesting(1,83,2,20,20) && game.spawnObjectForTesting(0,animal,0,21,20) &&
          game.spawnObjectForTesting(0,ore,0,20,21),
          "Visible animal food fixture");
    load("(defconst FOOD 2) (defconst goal-alias FOOD) (defconst resource-alias food)"
         "(defrule (resource-found metal) (resource-found goal-alias) (not (resource-found resource-alias))"
         " => (set-goal 940 FOOD))"
         "(defrule (resource-found food) => (set-goal 932 1))"
         "(defrule (not (resource-found food)) => (set-goal 933 1))");
    advance(10);
    check(game.aiGoalForTesting(2,932)==0 && game.aiGoalForTesting(2,933)==1 && game.aiGoalForTesting(2,940)==2,
          "Visible food animal is not the original berries-found fact");

    reset();
    check(game.spawnObjectForTesting(1,83,2,4,4) && game.spawnObjectForTesting(0,berry,0,50,50) &&
          game.spawnObjectForTesting(0,ore,0,51,50) && game.spawnObjectForTesting(0,nova,0,49,50),
          "Unseen resource fixtures");
    load("(defrule (resource-found food) => (set-goal 934 1))"
         "(defrule (resource-found metal) => (set-goal 935 1))"
         "(defrule (resource-found nova) => (set-goal 936 1))"
         "(defrule (true) => (set-goal 937 1))");
    advance(10);
    check(game.aiGoalForTesting(2,937)==1 && game.aiGoalForTesting(2,934)==0 &&
          game.aiGoalForTesting(2,935)==0 && game.aiGoalForTesting(2,936)==0,
          "Running PER cannot discover hidden unknown resources");

    reset();
    check(game.spawnObjectForTesting(1,109,2,10,20) && game.spawnObjectForTesting(0,ore,0,50,20),
          "Distant mineral and original command center threshold fixtures");
    const auto distantObserver=game.spawnObjectForTesting(1,83,2,49,20);check(distantObserver!=0,"Observe distant mineral");
    load("(defrule (game-time < 2) (not (resource-found metal)) => (set-goal 938 1))"
         "(defrule (game-time >= 2) => (set-strategic-number sn-camp-max-distance 60) (disable-self))"
         "(defrule (game-time >= 2) (resource-found metal) => (set-goal 939 1))");
    advance(10);
    check(game.aiGoalForTesting(2,938)==1 && game.aiGoalForTesting(2,939)==0,
          "Observed mineral beyond default25 edge-distance remains unfound");
    check(game.moveObjectForTesting(distantObserver,3,3),"Hide registered distant mineral before SN change");
    advance(25);
    check(game.aiGoalForTesting(2,939)==1,
          "PER SN86 change reevaluates remembered mineral without requiring current sight");
    std::puts("PASS: production resource-found discovery, animal exclusion, unknown fog, persistence and SN reevaluation");
}
