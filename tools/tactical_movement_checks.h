// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "../src/core/genie_tactical_movement.h"
#include <vector>
#include <limits>

static void tacticalMovementChecks() {
    using swgb::GenieTacticalMovementAction;
    check(swgb::geniePointFootprintDistance(0,0,4,5,1,1)==5,
          "551c60 measures distance beyond rectangular footprint, not center or clearance size");
    check(swgb::geniePointFootprintDistance(0,0,.25f,.25f,1,1)==0,
          "551c60 clamps both inside-footprint axes");
    int preparations=0,pathCalls=0;std::vector<int> notifications;
    const auto prepare=[&](){++preparations;return true;};
    const auto path=[&](){++pathCalls;return 0;};
    const auto notify=[&](int event){notifications.push_back(event);};
    GenieTacticalMovementAction a;a.present=true;a.range=1;
    check(a.update(1,prepare,path,notify)==0 && a.state==1 && !preparations && notifications.empty(),
          "408648 initial equality schedules completion without preparing a path");
    check(a.update(9,prepare,path,notify)==1 && notifications==std::vector<int>{506},
          "40882d reports610 completion only on the following owner update");
    a={};a.present=true;a.range=1;notifications.clear();
    a.update(std::nextafter(1.,2.),prepare,path,notify);
    check(a.state==4 && preparations==1 && pathCalls==0,"Initial outside-spacing prepares once");
    a.update(0,prepare,path,notify);
    check(a.state==4 && pathCalls==1 && preparations==1,
          "State4 delegates path status even when inside initial spacing");
    check(a.update(0,prepare,[]{return 2;},notify)==0 && a.state==1 && notifications.empty(),
          "Original path return2 defers completion notification");
    check(a.update(0,prepare,path,notify)==1 && notifications==std::vector<int>{506},"Deferred path completion");
    a={};a.present=true;notifications.clear();
    a.update(5,[]{return false;},path,notify);
    check(a.state==13 && notifications.empty(),"Failed preparation schedules state13");
    check(a.update(0,prepare,path,notify)==3 && a.present && notifications==std::vector<int>{505},
          "408857 failure returns3 and retains the action head");
    a={};a.present=true;a.state=4;notifications.clear();
    a.update(0,prepare,[]{return 1;},notify);
    check(a.state==13 && notifications.empty(),"Original ongoing path return1 fails, not completes");
    using swgb::geniePointPathSpacingArrival;
    check(!geniePointPathSpacingArrival(false,true,0,4,.1f,0),"4a5db8 excludes spacing exactly0.1");
    check(geniePointPathSpacingArrival(false,true,0,4,std::nextafter(.1f,1.f),.1),"Positive spacing branch above0.1");
    check(!geniePointPathSpacingArrival(true,true,0,4,1,0) &&
          !geniePointPathSpacingArrival(false,false,0,4,1,0) &&
          !geniePointPathSpacingArrival(false,true,4,4,1,0),"Target, absent pathdata and masked flags exclude ongoing spacing completion");
    const double epsilon=double(1.0000000116860974e-7f);
    check(!geniePointPathSpacingArrival(false,true,0,4,1,1+epsilon) &&
          geniePointPathSpacingArrival(false,true,0,4,1,std::nextafter(1+epsilon,1.)),
          "4a5e13 uses strict distance-minus-spacing epsilon");
}


static void tacticalMovementGameplayChecks(swgb::Assets &assets) {
    swgb::Scenario scenario;scenario.map.width=scenario.map.height=64;scenario.map.tiles.resize(64*64);
    scenario.nextUnitId=1;scenario.playerSlotCount=1;scenario.civilizations.fill(1);
    scenario.players[0].active=scenario.players[0].human=true;scenario.players[0].civilization=1;
    swgb::Game game(assets);std::string error;check(game.initScenario(scenario,&error),error.c_str());
    const auto worker=game.spawnObjectForTesting(1,83,1,20.5f,20.5f);
    check(game.queueTacticalMoveForTesting(worker,30.5f,20.5f,4),"Stage original724 point move");
    game.updatePaused(1,{});
    check(game.tacticalMovementStateForTesting(worker)==-1,"Pause does not dispatch military movement");
    bool walking=false,completed=false;
    for(int i=0;i<300;++i) {
        game.update(.05f,{});
        const int state=game.tacticalMovementStateForTesting(worker);
        walking|=state==4;completed|=state==1;
        if(walking && completed && state==-1)break;
    }
    const auto position=game.objectPosition(worker);
    check(walking && completed && game.tacticalMovementStateForTesting(worker)==-1,
          "Tactical move prepares, walks, retains completion and retires on admitted updates");
    check(position[0]>26 && position[0]<27 && std::abs(position[1]-20.5f)<.01f,
          "Military movement stops at its requested footprint spacing, not the target center");
    check(game.queueTacticalMoveForTesting(worker,30.5f,20.5f,4),"Already-in-range724 accepted");
    game.update(.05f,{});game.update(.05f,{});game.update(.05f,{});
    check(game.objectPosition(worker)==position,"Already-arrived military move does not walk closer");
    check(game.queueTacticalMoveForTesting(worker,40.5f,20.5f,1),"Replacement military move accepted");
    game.update(.05f,{});game.update(.05f,{});
    game.stopUnitForTesting(worker);
    check(game.tacticalMovementStateForTesting(worker)==-1,"Stop clears tactical movement without resurrection");
    const swgb::dat::Unit *mobileTransform=nullptr;
    for(const auto &unit:assets.dat().civs[1].units)if(unit.exists && unit.cls==34 && unit.speed>0) {mobileTransform=&unit;break;}
    check(mobileTransform!=nullptr,"Class34 mobile transformation fixture exists");
    const auto transform=game.spawnObjectForTesting(1,mobileTransform->id,1,40,40);
    check(transform && game.queueTacticalMoveForTesting(transform,40,40,1),"Mobile class34 accepts ordinary724 without transformation");
    game.update(0,{});game.update(0,{});
    check(game.tacticalMovementStateForTesting(transform)==1,"Class34 dispatch reaches initial spacing completion");
    std::puts("PASS: queued724 movement, pause, range arrival, deferred retirement and Stop");
}
