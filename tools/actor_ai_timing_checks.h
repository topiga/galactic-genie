// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "../src/core/genie_ai_timing.h"
static void actorAiTimingChecks(swgb::Assets &assets) {
    uint32_t rng=1;swgb::GenieTroopIdleClock timer;timer.initialize(rng);
    check(timer.initialOffset==41&&timer.secondaryThreshold==1084&&timer.threshold==1375&&rng==3884216597u,
          "Generic AI constructor preserves four original CRT draws including overwritten base threshold");
    check(!timer.advance(1375,true)&&timer.elapsed==1375,"Idle comparison is strict at threshold equality");
    check(timer.advance(1,true)&&timer.elapsed==0&&timer.threshold==1375,"Idle scan resets elapsed without resampling");
    check(!timer.advance(2000,false)&&timer.elapsed==2000&&timer.advance(0,true),"Active task defers scan without losing elapsed time");
    swgb::GenieOwnerSchedule owners;
    const uint32_t expected[]={75,150,225,225,225,225};
    for(uint32_t step=0;step<6;++step){
        check(owners.selected==step%3,"Owner rotation includes Gaia");
        for(uint32_t owner=0;owner<3;++owner){owners.beginOwner(owner,75);check(owners.delta(owner)==(owner==step%3?expected[step]:0),"Owner receives accumulated step milliseconds only on its turn");}
        owners.finish(3);
    }
    owners.elapsed[1]=0xfffffff0u;owners.beginOwner(1,33);check(owners.elapsed[1]==17,"Owner accumulator uses original unsigned wrap");
    owners.finish(3);check(owners.selected==1,"Admitted zero-duration step still rotates owner");
    swgb::Scenario scenario;scenario.map.width=scenario.map.height=32;scenario.map.tiles.resize(32*32);
    scenario.playerSlotCount=2;scenario.civilizations.fill(1);
    for(int i=0;i<2;++i){scenario.players[i].active=true;scenario.players[i].human=i==0;scenario.players[i].civilization=1;}
    swgb::ScenarioUnit troop;troop.unitId=329;troop.player=1;troop.spawnId=1;troop.state=2;troop.x=10;troop.y=10;
    scenario.units={troop};scenario.nextUnitId=2;
    swgb::Game game(assets);std::string error;check(game.initScenario(scenario,&error),error.c_str());
    const auto initial=game.idleClockForTesting(1);check(initial[0]==0&&initial[1]>=1125&&initial[1]<=1874,"Imported generic troop starts with original randomized idle threshold");
    uint32_t elapsed=0;bool scanned=false;
    for(int step=0;step<40;++step){
        const auto selected=game.selectedActorOwnerForTesting();game.update(.075f,{});
        if(selected==1){elapsed+=step==1?150:225;if(elapsed>initial[1]){elapsed=0;scanned=true;}}
        const auto clock=game.idleClockForTesting(1);check(clock[0]==elapsed&&clock[1]==initial[1],"Native idle clock follows selected-owner cadence and retains threshold after empty scans");
    }
    check(scanned,"Integration crosses at least one idle scan");
    const auto before=game.idleClockForTesting(1);const auto owner=game.selectedActorOwnerForTesting();game.updatePaused(3,{});
    check(game.idleClockForTesting(1)==before&&game.selectedActorOwnerForTesting()==owner,"Pause freezes actor clock and owner scheduling");
    std::puts("PASS: original constructor RNG, strict idle scans, retained phase, owner rotation/elapsed and pause");
}
