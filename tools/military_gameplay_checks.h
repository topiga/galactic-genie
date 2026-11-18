// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "../src/core/genie_random.h"
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
    for(int count:{1,2}) {
        Game explorer(assets);check(explorer.initScenario(scenario,&error),error.c_str());
        const auto home=explorer.spawnObjectForTesting(1,109,2,30,30);
        const auto scout=explorer.spawnObjectForTesting(1,550,2,31,31);
        const auto secondScout=count==2?explorer.spawnObjectForTesting(1,550,2,32,31):0;
        check(home && scout && (count==1 || secondScout),"Initial exploration home and scout fixture");
        check(explorer.loadAiSourceForTesting(2,"initial-exploration",std::string("(defrule (true) => (set-strategic-number sn-minimum-explore-group-size ")+std::to_string(count)+R"()
 (set-strategic-number sn-number-explore-groups 1)
 (disable-self))
)",{},&error),error.c_str());
        for(int i=0;i<8 && (explorer.explorationStateForTesting(scout)!=11 ||
            (count==2 && explorer.explorationStateForTesting(secondScout)!=11));++i)explorer.update(0,{});
        const auto &groups=explorer.militaryGroupsForTesting(2);
        check(groups.size()==1 && groups[0].type==101 && groups[0].members==(count==1?std::vector<int32_t>{int32_t(scout)}:
                  std::vector<int32_t>{int32_t(scout),int32_t(secondScout)}) &&
              groups[0].state==6 && groups[0].initialized,
              "Original singleton preference and multiunit flag1 grouping enter exploration without a six-soldier wave");
        check(explorer.stagedTacticalCountForTesting()>0 || explorer.explorationStateForTesting(scout)>=0,
              "Initial exploration ring stages a concrete705 destination through the shared tactical queue");
        check(explorer.stopUnitForTesting(scout) && (count==1 || explorer.stopUnitForTesting(secondScout)),
              "Stop exploration actors without removing their original group");
        const auto beforeRetry=explorer.stagedTacticalCountForTesting();
        explorer.setGameTimeForTesting(15.999f);
        explorer.updateAiMilitaryForTesting(2,1000);
        check(explorer.stagedTacticalCountForTesting()==beforeRetry,"Default SN76 retains stopped exploration through15999ms");
        explorer.setGameTimeForTesting(16.f);
        explorer.updateAiMilitaryForTesting(2,1000);
        check(explorer.stagedTacticalCountForTesting()>beforeRetry &&
              explorer.militaryGroupsForTesting(2)[0].search.lastBusyTime==16000,
              "Initialized idle exploration retries with forced705 after sixteen whole seconds");
    }
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
    check(game.militaryPhaseForTesting(2)==AiMilitaryPhase::Refresh && game.stagedTacticalCountForTesting()==beforePlan,
          "Pending original planning waits for idle timeout without native waypoint commands");
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
        check(deficit.members.size()==2 && deficit.desiredCount==(homeCase==0?2:3) && deficit.state==1,
              "Disconnected home shrinks demand before no-target wait; same-region or absent home retains deficit");
        if(homeCase!=0) {
            const auto queued=game.stagedTacticalCountForTesting();
            game.updateSkirmishArmyForTesting(2);
            check(game.stagedTacticalCountForTesting()==queued,"Native fallback does not launch a waiting deficit group");
            game.updateAiMilitaryForTesting(2,UINT32_MAX);
            check(game.militaryGroupsForTesting(2)[0].desiredCount==2 && game.militaryGroupsForTesting(2)[0].state==1,
                  "Next phase7 shrinks attack-now demand before original no-target wait");
        }
    }
    for(int targetType:{329,109}) {
        scenario.players[0].diplomacy[2]=3;scenario.players[1].diplomacy[1]=3;
        check(game.initScenario(scenario,&error),error.c_str());
        const auto one=game.spawnObjectForTesting(1,329,2,20,20);
        const auto two=game.spawnObjectForTesting(1,329,2,21,20);
        const auto target=game.spawnObjectForTesting(1,targetType,1,24,20);
        check(one && two && target && game.formAiMilitaryGroupsForTesting(2),"Group target selection fixture");
        game.setAttackModeForTesting(one,3);game.setAttackModeForTesting(two,3);
        game.updateAiMilitaryForTesting(2,UINT32_MAX);
        check(game.militaryGroupsForTesting(2).size()==1 && game.militaryGroupsForTesting(2)[0].targetId==int32_t(target),
              "Ready group acquires an observed target through original metric/ranking stages");
        check(game.stagedTacticalCountForTesting()==0,"Group attack uses one local packet instead of per-unit tactical700 staging");
        game.updatePaused(1,{});
        check(!game.attackTargetForTesting(one) && !game.attackTargetForTesting(two),"Pause retains queued group attack without dispatch");
        game.update(0,{});
        check(game.attackTargetForTesting(one)==target && game.attackTargetForTesting(two)==target,
              "Local group packet dispatches the same target to both members");
        const auto closer=game.spawnObjectForTesting(1,329,1,21,21);
        check(closer!=0,"Introduce a closer target after acquisition");
        game.updateAiMilitaryForTesting(2,UINT32_MAX);
        check(game.militaryGroupsForTesting(2)[0].targetId==int32_t(target),"Valid group target is retained instead of nearest-target reselection");
        check(game.moveObjectForTesting(target,55,55),"Target leaves group sight");
        game.updateAiMilitaryForTesting(2,UINT32_MAX);
        check(game.militaryGroupsForTesting(2)[0].targetId==(targetType==109?int32_t(target):-1) &&
              game.militaryGroupsForTesting(2)[0].state==(targetType==109?2:1),
              "Original retention keeps hidden buildings and resets hidden mobile targets");
        if(targetType==109) {
            check(game.damageObjectForTesting(target,100000),"Destroy retained group target");
            game.updateAiMilitaryForTesting(2,UINT32_MAX);
            check(game.militaryGroupsForTesting(2)[0].targetId==-1 && game.militaryGroupsForTesting(2)[0].state==1,
                  "Destroyed target resets group planning and queues Stop");
            game.update(0,{});
            check(!game.attackTargetForTesting(one) && !game.attackTargetForTesting(two),"Deferred group Stop clears both attacks");
        }
    }
    check(game.initScenario(scenario,&error),error.c_str());
    const auto searcher=game.spawnObjectForTesting(1,329,2,20,20);
    check(searcher && game.formAiMilitaryGroupsForTesting(2),"No-target group search fixture");
    check(game.militaryGroupsForTesting(2)[0].flags==40,"Fresh runtime group preserves constructor32 and attack-now8");
    game.setGameTimeForTesting(4.999f);game.setAmbientRandomForTesting(12345);
    game.updateAiMilitaryForTesting(2,UINT32_MAX);
    check(game.militaryGroupsForTesting(2)[0].state==1 && game.legacyRandomForTesting()==12345,
          "No-target group waits4999ms without consuming search RNG");
    game.setGameTimeForTesting(5.f);game.updateAiMilitaryForTesting(2,UINT32_MAX);
    const auto searchOrigin=game.militaryGroupsForTesting(2)[0].search.origin;
    uint32_t expectedSearchRng=12345;
    const auto searchPoint=aiGroupSearchPoint(game.militaryGroupsForTesting(2)[0].search,64,64,[&] {
        return genieRandomDraw(expectedSearchRng);
    });
    check(searchPoint && game.militaryGroupsForTesting(2)[0].state==18 &&
          game.legacyRandomForTesting()==expectedSearchRng && searchOrigin==std::array<float,3>{44,0,20},
          "Exact5000ms search captures centroid and consumes exactly two shared original RNG draws");
    game.updatePaused(1,{});
    check(game.pointMovementStatusForTesting(searcher)==-1,"Pause retains targetless group packet without moving");
    game.update(0,{});
    const auto searchDestination=game.pointMovementDestinationForTesting(searcher);
    check(searchDestination==std::array<float,2>{float((*searchPoint)[1]),64.f-float((*searchPoint)[0])},
          "Targetless local group packet reaches shared native group movement with original selected point");
    game.setGameTimeForTesting(30.f);game.updateAiMilitaryForTesting(2,UINT32_MAX);
    check(game.militaryGroupsForTesting(2)[0].state==18,"Busy search retains state at exact25000ms");
    game.setGameTimeForTesting(30.001f);game.updateAiMilitaryForTesting(2,UINT32_MAX);
    check(game.militaryGroupsForTesting(2)[0].state==1 && !(game.militaryGroupsForTesting(2)[0].flags&32u),
          "Busy search expires after25000ms and clears origin capture only after queuing Stop");
    game.update(0,{});
    game.setGameTimeForTesting(35.001f);game.updateAiMilitaryForTesting(2,UINT32_MAX);
    check(game.militaryGroupsForTesting(2)[0].state==18 &&
          game.militaryGroupsForTesting(2)[0].search.origin==searchOrigin,
          "Repeated no-target search retains its prior origin until a successful target acquisition");
    std::puts("PASS: production PER military admission, group targeting, movement, pause, ownership and reset");
}
