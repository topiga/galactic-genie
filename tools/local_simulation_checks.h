// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "../src/engine/local_simulation_clock.h"

static void localSimulationChecks(swgb::Assets &assets) {
    swgb::LocalSimulationClock clock;
    auto step=clock.advance(1000);
    check(step.admitted && step.deltaMilliseconds==0,"Original first host call admits zero simulation time");
    check(!clock.advance(1033).admitted,"Sub-50ms host frame is not a simulation call");
    step=clock.advance(1066);
    check(step.admitted && step.deltaMilliseconds==66,"Skipped host frames accumulate from last accepted timestamp");
    step=clock.advance(5000);
    check(step.admitted && step.deltaMilliseconds==100,"Long local frame is capped at100ms");
    clock.suspend();
    check(!clock.advance(5033).admitted,"Suspended clock still waits for an eligible timestamp");
    check(!clock.advance(5066).admitted,"First eligible resumed call rebases without simulation");
    step=clock.advance(5116);
    check(step.admitted && step.deltaMilliseconds==50,"Resumed local clock advances from its new baseline");

    swgb::Scenario scenario;
    scenario.map.width=scenario.map.height=32;scenario.map.tiles.resize(32*32);
    scenario.civilizations.fill(1);
    for(int player=0;player<2;++player) {
        auto &p=scenario.players[size_t(player)];p.active=true;p.human=player==0;
        p.civilization=1;p.populationLimit=200;p.resources[0]=500;
        p.diplomacy[1]=player==0?0:3;p.diplomacy[2]=player==1?0:3;
    }
    swgb::ScenarioUnit center;center.unitId=109;center.spawnId=1;center.player=1;center.state=2;center.x=10;center.y=10;
    swgb::ScenarioUnit worker=center;worker.unitId=293;worker.spawnId=2;worker.x=14;
    scenario.units={center,worker};scenario.nextUnitId=3;
    swgb::Game game(assets);std::string error;
    const std::string script=
        "(defrule (goal 980 2) => (set-goal 980 3))\n"
        "(defrule (goal 980 1) => (set-goal 980 2))\n"
        "(defrule (goal 980 0) => (set-goal 980 1))\n";
    const auto initialize=[&] {
        check(game.initScenario(scenario,&error),error.c_str());
        check(game.loadAiSourceForTesting(2,"local-admission",script,{},&error),error.c_str());
    };
    bool trainedA=false,trainedB=false;
    for(uint32_t seed=1;seed<=32 && !(trainedA && trainedB);++seed) {
        initialize();
        check(game.queueUnitForTesting(1,83),"Queue standard Command Center worker");
        game.seedLegacyRandomForTesting(seed);
        game.update(.2f,{});
        const int trained=game.objectUnitId(3);
        check(trained==83 || trained==293,"Worker exit creates a valid civilization worker");
        trainedA|=trained==83;
        trainedB|=trained==293;
    }
    check(trainedA && trainedB,"Standard worker orders produce both original worker variants");
    initialize();
    check(game.queueUnitForTesting(1,83),"Queue actual worker production for local scheduling check");
    game.lookAtObject(2);
    game.updateLocal(1000,.016f,{});
    const auto position=game.objectPosition(2);
    const float remaining=game.productionRemainingForTesting(1);
    const auto camera=game.cameraForTesting();
    const uint32_t randomBeforeRejectedFrame=game.legacyRandomForTesting();
    swgb::InputState input;input.scrollX=1;
    game.updateLocal(1033,.033f,input);
    check(game.legacyRandomForTesting()==randomBeforeRejectedFrame,
          "Non-admitted camera frame neither reseeds nor advances simulation RNG");
    check(game.cameraForTesting()!=camera && game.objectPosition(2)==position &&
          game.productionRemainingForTesting(1)==remaining && game.aiGoalForTesting(2,980)==1,
          "Camera updates between admitted ticks without movement, production or AI advancement");
    input={};input.selectPressed=true;input.cursorVisible=true;
    check(game.objectScreenPosition(2,input.screenW,input.screenH,input.pointerX,input.pointerY),"Worker selection screen coordinate");
    game.updateLocal(1034,.001f,input);
    check(game.objectSelected(2),"Selection click is handled on a non-admitted host frame");
    input={};input.commandPressed=true;input.cursorVisible=true;
    const auto currentCamera=game.cameraForTesting();
    const float goalX=position[0]+4,goalY=position[1];
    input.pointerX=(goalX-goalY)*swgb::Game::kTileHalfW-currentCamera[0]+input.screenW*.5f;
    input.pointerY=(goalX+goalY)*swgb::Game::kTileHalfH-currentCamera[1]+input.screenH*.5f;
    game.updateLocal(1035,.001f,input);
    check(game.objectPosition(2)==position && game.productionRemainingForTesting(1)==remaining &&
          game.aiGoalForTesting(2,980)==1,"Order input does not run a world step between eligible timestamps");
    const auto moving=game.movingObjects();
    check(std::none_of(moving.begin(),moving.end(),[](const swgb::MovingObjectInfo &o){return o.spawnId==2;}),
          "Between-tick right click waits for admitted command dispatch");
    check(game.selectObjectForTesting(1),"Change selection while the worker command is pending");
    game.lookAtObject(1);
    game.updateLocal(1066,.031f,{});
    check(game.objectPosition(2)!=position && game.productionRemainingForTesting(1)<remaining,
          "Next admitted tick advances both ordered movement and production");
    const auto destination=game.moveAnchorForTesting(2);
    check(std::abs(destination[0]-goalX)<.01f && std::abs(destination[1]-goalY)<.01f,
          "Queued destination is captured in world coordinates before camera changes");
    swgb::Game reference(assets);
    check(reference.initScenario(scenario,&error),error.c_str());
    check(reference.queueUnitForTesting(1,83),"Queue reference worker for original normal-speed scaling");
    reference.update(0,{});reference.update(.099f,{});
    check(std::abs(game.productionRemainingForTesting(1)-reference.productionRemainingForTesting(1))<.00001f,
          "Normal local speed converts the accepted66 host milliseconds into99 simulation milliseconds");
    const auto pausedPosition=game.objectPosition(2);
    const float pausedRemaining=game.productionRemainingForTesting(1);
    game.updatePaused(10,{});
    game.updateLocal(9000,.016f,{});
    check(game.objectPosition(2)==pausedPosition && game.productionRemainingForTesting(1)==pausedRemaining &&
          game.aiGoalForTesting(2,980)==1,"Pause and first accepted resume rebase without catch-up or AI evaluation");
    game.updateLocal(9050,.05f,{});
    check(game.productionRemainingForTesting(1)<pausedRemaining,"Simulation resumes after a fresh admitted interval");

    initialize();
    game.lookAtObject(2);
    game.updateLocal(1000,0,{});
    check(game.selectObjectForTesting(2),"Select worker before pending-command pause");
    input={};input.commandPressed=true;input.cursorVisible=true;
    auto point=game.objectPosition(2);
    const auto view=game.cameraForTesting();
    input.pointerX=(point[0]+4-point[1])*swgb::Game::kTileHalfW-view[0]+input.screenW*.5f;
    input.pointerY=(point[0]+4+point[1])*swgb::Game::kTileHalfH-view[1]+input.screenH*.5f;
    game.updateLocal(1001,0,input);
    game.updatePaused(1,{});
    game.updateLocal(2000,0,{});
    check(game.objectPosition(2)==point,"Queued command survives pause without executing during resume rebase");
    game.updateLocal(2050,0,{});
    check(game.objectPosition(2)!=point,"Retained command executes on next admitted resumed step");
    initialize();
    game.updateLocal(1000,0,{});
    game.selectObjectForTesting(2);game.lookAtObject(2);
    game.updateLocal(1001,0,input);
    initialize();
    point=game.objectPosition(2);
    game.updateLocal(1000,0,{});game.updateLocal(1050,0,{});
    check(game.objectPosition(2)==point,"New session discards pending commands from the old world");

    initialize();
    game.updateLocal(1000,0,{});
    game.lookAtObject(2);game.selectObjectForTesting(2);
    game.updateLocal(1001,0,input);
    check(game.destroyLastSelectedForTesting(),"Delete captured actor before command dispatch");
    game.updateLocal(1050,0,{});
    const auto afterDeletion=game.movingObjects();
    check(std::none_of(afterDeletion.begin(),afterDeletion.end(),[](const swgb::MovingObjectInfo &o){return o.spawnId==2;}),
          "Deleted captured actor is not revived or replaced during dispatch");
    initialize();
    game.updateLocal(1000,0,{});
    game.lookAtObject(2);game.selectObjectForTesting(1);
    game.updateLocal(1001,0,input);
    check(!game.gatherPointActiveForTesting(1),"Building rally click waits for command dispatch");
    game.selectObjectForTesting(2);
    game.updateLocal(1050,0,{});
    check(game.gatherPointActiveForTesting(1),"Rally dispatch keeps captured building after selection changes");

    initialize();
    for(uint32_t call=1;call<=40;++call) {
        const uint32_t timestamp=1000+(call-1)*50;
        game.updateLocal(timestamp,.05f,{});
        const int expected=call<20?1:call<40?2:3;
        check(game.aiGoalForTesting(2,980)==expected,"AI executes on first, twentieth and fortieth admitted calls only");
        game.updateLocal(timestamp+1,.001f,{});
        check(game.aiGoalForTesting(2,980)==expected,"Skipped host frames do not increment the AI call cadence");
    }
    initialize();
    game.updateLocal(1000,.016f,{});
    check(game.aiGoalForTesting(2,980)==1,"New session resets local host baseline and first AI evaluation");
    swgb::Scenario empty;
    empty.map.width=empty.map.height=8;empty.map.tiles.resize(64);
    empty.playerSlotCount=1;empty.players[0].active=empty.players[0].human=true;
    empty.players[0].civilization=1;
    check(game.initScenario(empty,&error),error.c_str());
    game.updateLocal(1000,0,{});
    for(uint32_t i=1;i<1000;++i)game.updateLocal(1000+i*66,0,{});
    check(game.loadAiSourceForTesting(1,"integer-world-time",
          "(defrule (game-time == 99) => (set-goal 981 1))",{},&error),error.c_str());
    game.updateLocal(67000,0,{});
    check(game.aiGoalForTesting(1,981)==1,
          "One thousand99ms simulation steps reach original game-time99 without float accumulation drift");
    check(game.initScenario(empty,&error),error.c_str());
    game.update(4294967.f,{}); // 296ms before the original uint32 world clock wraps.
    game.updateLocal(1000,0,{});
    game.updateLocal(1100,0,{});
    check(game.loadAiSourceForTesting(1,"world-time-wrap",
          "(defrule (game-time == 0) => (set-goal 982 1))",{},&error),error.c_str());
    game.updateLocal(1200,0,{});
    check(game.aiGoalForTesting(1,982)==1,"Original uint32 world milliseconds wrap after two150ms admitted steps");
    std::puts("PASS: local host admission, between-tick input/camera, pause rebase and admitted-call AI cadence");
}
