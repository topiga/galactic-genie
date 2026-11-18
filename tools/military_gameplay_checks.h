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
          admitted[0].desiredCount==2 && admitted[0].state==1 && (admitted[0].flags&8u),
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
    check(game.initScenario(scenario,&error),error.c_str());
    const auto leader=game.spawnObjectForTesting(1,329,2,15,15);
    const auto later=game.spawnObjectForTesting(1,329,2,45,45);
    check(leader && later && game.formAiMilitaryGroupsForTesting(2),"Two independent military groups");
    check(game.militaryGroupsForTesting(2).size()==2,"Distant units form separate groups");
    game.updateAiMilitaryForTesting(2,0);
    check(game.militaryPhaseForTesting(2)==AiMilitaryPhase::Readiness,"Zero budget retains readiness after refresh");
    const auto destination=game.militaryGroupsForTesting(2)[1].destination;
    check(game.moveObjectForTesting(later,55,55),"Move later leader during retained readiness");
    game.updateAiMilitaryForTesting(2,0);
    check(game.militaryPhaseForTesting(2)==AiMilitaryPhase::Readiness && game.militaryGroupsForTesting(2)[0].state==2 &&
          game.militaryGroupsForTesting(2)[1].destination==destination,"Budget continuation preserves the unprocessed group's destination");
    const auto staged=game.stagedTacticalCountForTesting();
    game.updateSkirmishArmyForTesting(2);
    check(game.stagedTacticalCountForTesting()==staged,"Native fallback cannot bypass retained readiness to plan for ready members");
    game.updatePaused(1,{});
    check(game.militaryPhaseForTesting(2)==AiMilitaryPhase::Readiness,"Pause retains scheduler continuation");
    game.updateAiMilitaryForTesting(2,0);
    check(game.militaryPhaseForTesting(2)==AiMilitaryPhase::Planning && game.militaryGroupsForTesting(2)[1].destination==destination,
          "Finished readiness retains pending planning at the deadline");
    const auto beforePlan=game.stagedTacticalCountForTesting();
    game.updateAiMilitaryForTesting(2,0);
    check(game.militaryPhaseForTesting(2)==AiMilitaryPhase::Refresh && game.stagedTacticalCountForTesting()>beforePlan,
          "Pending native planning emits commands without another refresh");
    game.updateAiMilitaryForTesting(2,0);
    check(game.militaryGroupsForTesting(2)[1].destination!=destination,"Only the following cycle refreshes the moving leader");
    // Home lookup follows admission order, not proximity; both region samples use the member's restriction.
    for(int homeCase=0;homeCase<3;++homeCase) {
        check(game.initScenario(scenario,&error),error.c_str());
        for(int y=0;y<64;++y)check(game.setTerrainForTesting(32,y,1),"Separate home and army land regions");
        if(homeCase!=2) {
            const float firstHome=homeCase==0?15.f:48.f,secondHome=homeCase==0?48.f:15.f;
            check(game.spawnObjectForTesting(1,109,2,firstHome,15)!=0 &&
                  game.spawnObjectForTesting(1,109,2,secondHome,15)!=0,"Ordered Command Center admission fixture");
        }
        const auto one=game.spawnObjectForTesting(1,329,2,45,45);
        const auto two=game.spawnObjectForTesting(1,329,2,46,45);
        const auto lost=game.spawnObjectForTesting(1,329,2,47,45);
        check(one && two && lost && game.formAiMilitaryGroupsForTesting(2),"Admit complete group before casualty");
        check(game.militaryGroupsForTesting(2).size()==1 && game.militaryGroupsForTesting(2)[0].desiredCount==3,
              "Original requested size retained before casualty");
        game.updateAiMilitaryForTesting(2,0); // Lose the member after phase7, while readiness is retained.
        check(game.transferObjectForTesting(lost,1),"Remove one member through ownership change");
        game.updateAiMilitaryForTesting(2,UINT32_MAX);
        const auto &deficit=game.militaryGroupsForTesting(2)[0];
        check(deficit.members.size()==2 && deficit.desiredCount==(homeCase==0?2:3) && deficit.state==(homeCase==0?2:1),
              "Disconnected first home releases a smaller group; same-region or absent home waits");
        if(homeCase!=0) {
            const auto queued=game.stagedTacticalCountForTesting();
            game.updateSkirmishArmyForTesting(2);
            check(game.stagedTacticalCountForTesting()==queued,"Native fallback does not launch a waiting deficit group");
            game.updateAiMilitaryForTesting(2,UINT32_MAX);
            check(game.militaryGroupsForTesting(2)[0].desiredCount==2 && game.militaryGroupsForTesting(2)[0].state==2,
                  "Next phase7 honors original attack-now flag8 and releases the reduced group");
        }
    }
    std::puts("PASS: production PER military admission, gather movement, pause, ownership and reset");
}
