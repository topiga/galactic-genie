// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
[[gnu::noinline]] static void playerMilitaryFactsGameplayChecks(swgb::Assets &assets) {
    swgb::Scenario scenario;scenario.map.width=scenario.map.height=64;scenario.map.tiles.resize(64*64);
    scenario.nextUnitId=1;scenario.playerSlotCount=2;scenario.civilizations.fill(1);
    for(int i=0;i<2;++i) {
        scenario.players[i].active=true;scenario.players[i].human=i==0;scenario.players[i].civilization=1;
        scenario.players[i].diplomacy.fill(3);scenario.players[i].diplomacy[size_t(i+1)]=0;
    }
    swgb::Game game(assets);std::string error;check(game.initScenario(scenario,&error),error.c_str());
    check(game.loadAiSourceForTesting(2,"military-facts",R"(
(defrule (true) => (set-strategic-number sn-percent-attack-soldiers 50) (disable-self))
(defrule (players-military-population any-enemy == 1) => (set-goal 94 1))
(defrule (players-military-population any-enemy == 0) => (set-goal 94 0))
(defrule (players-military-population every-enemy >= 0) => (set-goal 95 1))
(defrule (players-military-population 0 == 0) => (set-goal 97 1))
(defrule (not (players-military-population -99 != 100)) => (set-goal 98 1))
(defrule (not (players-military-population 99 == 0)) => (set-goal 99 1))
(defrule (soldier-count == 2) (attack-soldier-count == 0) => (set-goal 93 1))
(defrule (soldier-count == 2) (attack-soldier-count == 1) => (set-goal 93 2))
(defrule (defend-soldier-count == 2) => (set-goal 96 2))
(defrule (defend-soldier-count == 1) => (set-goal 96 1))
)",{},&error),error.c_str());
    const auto enemy=game.spawnObjectForTesting(1,307,1,45,45);
    const auto home=game.spawnObjectForTesting(1,109,1,44,45);
    const auto first=game.spawnObjectForTesting(1,307,2,15,15);
    const auto second=game.spawnObjectForTesting(1,307,2,17,15);
    check(enemy && home && first && second,"Military player fact fixtures spawn");
    game.updateAiMilitaryForTesting(2,0);
    const auto evaluate=[&] {for(int i=0;i<25;++i)game.update(0,{});};evaluate();
    check(game.aiGoalForTesting(2,94)==1 && game.aiGoalForTesting(2,95)==1,
          "Original player military fact sees enemy population without a fog gate");
    check(game.aiGoalForTesting(2,97)==1 && game.aiGoalForTesting(2,98)==1 && game.aiGoalForTesting(2,99)==1,
          "Military population accepts direct Gaia and rejects invalid numeric slots as false under negation");
    check(game.aiMatchedPlayerForTesting(2,-106)==1 && !game.aiMatchedPlayerForTesting(2,-203),
          "Any military selector retains matching player while every leaves its cache untouched");
    check(game.aiGoalForTesting(2,96)==2 && game.aiGoalForTesting(2,93)==1,"Ungrouped soldier registry contributes to defenders");
    check(game.formAiMilitaryGroupsForTesting(2),"Original attack-now admission creates an actual group");
    check(game.militaryGroupsForTesting(2).size()==1 && game.militaryGroupsForTesting(2)[0].members.size()==1,
          "Explicit half-strength attack percentage admits one of two soldiers");
    evaluate();check(game.aiGoalForTesting(2,96)==1 && game.aiGoalForTesting(2,93)==2,"Defender fact subtracts actual group members");
    check(game.garrisonForTesting(enemy,home),"Enemy military population fixture garrisons");
    for(int n=0;n<80;++n)game.update(.1f,{});
    check(game.garrisonContainerForTesting(enemy)==int(home),"Enemy troop actually enters its container");
    evaluate();check(game.aiGoalForTesting(2,94)==1,"Garrisoned military still contributes to player population");
    check(game.damageObjectForTesting(enemy,100000),"Remove enemy military population fixture");
    evaluate();check(game.aiGoalForTesting(2,94)==0,"Military population follows death rather than stale startup resource40");
    std::puts("PASS: production player military selectors, cache, lifecycle and defender group counts");
}
