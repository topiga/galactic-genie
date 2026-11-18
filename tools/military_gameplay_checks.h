// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
static void militaryGameplayChecks(swgb::Assets &assets) {
    using namespace swgb;
    check(aiMilitarySoldierClass(32) && aiMilitarySoldierClass(51) && !aiMilitarySoldierClass(43),
          "Original soldier admission precedes master-trait classification");
    check(aiMilitaryAttackCount(3,50)==2 && aiMilitaryAttackCount(40,100)==40 &&
          aiMilitaryAttackCount(100,0)==0,"Original attack percentage rounds upward after float32 percent scaling");
    std::vector<AiMilitaryGatherGroup> groups{{1,1,2,{10,20}},{2,1,1,{30}}};
    check(removeAiMilitaryMember(groups,10) && groups[0].desiredCount==2 && groups[0].members==std::vector<int32_t>{20},
          "Casualty removal preserves original desired count and remaining member order");
    check(removeAiMilitaryMember(groups,20) && groups.size()==1 && groups[0].id==2,
          "Removing the last member destroys only its group");
    Scenario scenario;scenario.map.width=scenario.map.height=64;scenario.map.tiles.resize(64*64);
    scenario.nextUnitId=1;scenario.playerSlotCount=2;scenario.civilizations.fill(1);
    for(int i=0;i<2;++i){scenario.players[i].active=true;scenario.players[i].human=i==0;scenario.players[i].civilization=1;}
    Game game(assets);std::string error;
    check(game.initScenario(scenario,&error),error.c_str());
    const uint32_t first=game.spawnObjectForTesting(1,329,2,20,20);
    const uint32_t second=game.spawnObjectForTesting(1,329,2,29,20);
    check(first && second,"Military admission fixture");
    game.setAttackModeForTesting(first,3);game.setAttackModeForTesting(second,3);
    check(game.loadAiSourceForTesting(2,"military-controller",R"(
(defrule (true) =>
 (set-strategic-number sn-percent-attack-soldiers 100)
 (set-strategic-number sn-group-form-distance 32)
 (set-strategic-number sn-attack-group-gather-spacing 1)
 (set-strategic-number sn-zero-priority-distance 200)
 (attack-now)
 (disable-self))
)",{},&error),error.c_str());
    game.update(0,{});
    auto admitted=game.militaryGroupsForTesting(2);
    check(admitted.size()==1 && admitted[0].members==std::vector<int32_t>{int32_t(first),int32_t(second)} &&
          admitted[0].desiredCount==2 && admitted[0].state==1,
          "Actual PER attack-now admits one ordered group and retains gather state");
    game.updatePaused(1,{});
    check(game.militaryGroupsForTesting(2)[0].state==1,"Pause retains military readiness state");
    game.update(0,{});
    check(game.tacticalMovementStateForTesting(second)==3 || game.tacticalMovementStateForTesting(second)==4,
          "Recovered gather dispatch reaches the production724 movement controller");
    check(game.transferObjectForTesting(second,1),"Transfer admitted group member");
    game.update(0,{});
    const auto remaining=game.militaryGroupsForTesting(2);
    check(remaining.size()==1 && remaining[0].members==std::vector<int32_t>{int32_t(first)} && remaining[0].desiredCount==2,
          "Live ownership reconciliation retains original casualty deficit rather than silently shrinking demand");
    check(game.initScenario(scenario,&error) && game.militaryGroupsForTesting(2).empty(),
          "Session reset discards military membership and controller state");
    scenario.playerSlotCount=3;scenario.players[2].active=true;scenario.players[2].civilization=1;
    check(game.initScenario(scenario,&error),error.c_str());
    check(game.spawnObjectForTesting(1,329,3,20,20)!=0,"Third-owner military fixture");
    check(game.loadAiSourceForTesting(3,"third-owner", "(defrule (true) => (attack-now) (disable-self))",{},&error),error.c_str());
    game.update(0,{});
    check(game.militaryGroupsForTesting(3).size()==1,"Default-profile attack-now supports owners beyond skirmish center slots");
    std::puts("PASS: production PER military admission, gather movement, pause, ownership and reset");
}
