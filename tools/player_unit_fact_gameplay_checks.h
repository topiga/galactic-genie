// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
[[gnu::noinline]] static void playerUnitFactGameplayChecks(swgb::Assets &assets) {
    const swgb::dat::UnitLine *laser=nullptr;
    for(const auto &line:assets.dat().unitLines)
        if(swgb::normalizeAiSymbol(line.name)=="unit-laser-line")laser=&line;
    check(laser && laser->id>=10 && laser->id<=42 && laser->unitIds.size()>=2,
          "Original unit-count fact has a supported DAT laser line");
    swgb::Scenario scenario;scenario.map.width=scenario.map.height=64;
    scenario.map.tiles.resize(64*64);scenario.nextUnitId=1;scenario.playerSlotCount=2;
    scenario.civilizations.fill(1);
    for(int i=0;i<2;++i) {
        scenario.players[i].active=true;scenario.players[i].human=i==0;
        scenario.players[i].civilization=1;scenario.players[i].populationLimit=75;
        scenario.players[i].diplomacy.fill(3);scenario.players[i].diplomacy[size_t(i+1)]=0;
    }
    swgb::Game game(assets);std::string error;
    check(game.initScenario(scenario,&error),error.c_str());
    const auto center=game.spawnObjectForTesting(1,109,1,44,45);
    const auto male=game.spawnObjectForTesting(1,83,1,46,45);
    check(center && male && game.spawnObjectForTesting(1,70,1,40,40) &&
          game.spawnObjectForTesting(1,293,1,47,45) &&
          game.spawnObjectForTesting(1,259,1,48,45) && game.spawnObjectForTesting(1,83,0,50,50),
          "Male, female, farmer-role and Gaia worker fixtures");
    for(size_t i=0;i<2;++i)
        check(game.spawnObjectForTesting(1,laser->unitIds[i],1,45+float(i),48)!=0,
              "Two physical upgrade members belong to the same original line");
    check(game.loadAiSourceForTesting(1,"own-worker-roles",R"(
(defrule (unit-type-count 83 == 3) (unit-type-count-total 83 == 3)
         (unit-type-count 293 == 1) (unit-type-count-total 293 == 1)
         (unit-type-count 259 == 1) (unit-type-count-total 259 == 1)
         => (set-goal 984 1))
)",{},&error),error.c_str());
    check(game.loadAiSourceForTesting(2,"cc-unit-count",R"(
(defconst laser-alias UNIT-LASER-LINE)
(defrule (cc-players-unit-type-count 1 83 == 3)
         (cc-players-unit-type-count 1 293 == 1)
         (cc-players-unit-type-count 1 259 == 1) => (set-goal 980 3))
(defrule (cc-players-unit-type-count 1 83 == 2) => (set-goal 980 2))
(defrule (cc-players-unit-type-count 1 83 > 3) => (set-goal 980 99))
(defrule (cc-players-unit-type-count any-enemy laser-alias == 2)
         (cc-players-unit-type-count every-enemy laser-alias >= 2) => (set-goal 981 1))
(defrule (cc-players-unit-type-count 0 83 == 1)
         (not (cc-players-unit-type-count 99 83 == 0)) => (set-goal 982 1))
(defrule (not (cc-players-unit-type-count 1 -2 == 0)) => (set-goal 983 1))
)",{},&error),error.c_str());
    const auto evaluate=[&]{for(int i=0;i<25;++i)game.update(0,{});};
    evaluate();
    check(game.aiGoalForTesting(1,984)==1,
          "Own current/total facts aggregate only operand83; female and farmer roles stay individual");
    check(game.aiGoalForTesting(2,980)==3 && game.aiGoalForTesting(2,981)==1 &&
          game.aiGoalForTesting(2,982)==1 && game.aiGoalForTesting(2,983)==0,
          "CC unit fact sees hidden workers/roles, mixed upgrades, Gaia and invalid selectors without turning unsafe NOT true");
    for(int type=0;type<4;++type)game.setResourceForTesting(1,type,1000);
    check(game.loadAiSourceForTesting(1,"queued-worker",
        R"((defrule (can-train 83) => (train 83) (disable-self))
(defrule (unit-type-count 83 == 3) (unit-type-count-total 83 == 4)
         (unit-type-count 293 == 1) (unit-type-count-total 293 == 1)
         (unit-type-count 259 == 1) (unit-type-count-total 259 == 1)
         => (set-goal 984 2)))",{},&error),error.c_str());
    evaluate();
    check(game.aiQueuedUnitCountForTesting(1,83)>0 && game.aiGoalForTesting(2,980)==3 &&
          game.aiGoalForTesting(1,984)==2,
          "Real pending worker production is excluded from CC current worker aggregate");
    check(game.cancelProductionForTesting(center),"Cancel queued fixture before garrison timing");
    check(game.loadAiSourceForTesting(1,"passive-enemy","",{},&error),error.c_str());
    evaluate();
    check(game.garrisonForTesting(male,center),"Counted worker receives an actual garrison order");
    for(int i=0;i<80 && game.garrisonContainerForTesting(male)<0;++i)game.update(.1f,{});
    check(game.garrisonContainerForTesting(male)==int(center),"Counted worker actually enters Command Center");
    evaluate();check(game.aiGoalForTesting(2,980)==3,"Garrison retains worker aggregate membership");
    check(game.damageObjectForTesting(male,100000),"Remove counted garrisoned worker");
    evaluate();check(game.aiGoalForTesting(2,980)==2,"CC current worker count follows native death adapter");
    std::puts("PASS: production CC unit facts, worker aggregate, lines, queue exclusion, garrison and death");
}
