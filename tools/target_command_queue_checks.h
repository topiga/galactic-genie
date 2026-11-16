// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

static void targetCommandQueueChecks(swgb::Assets &assets) {
    swgb::Game game(assets);std::string error;
    const auto reset=[&] {
        check(game.initSkirmish(0x5A17,&error),error.c_str());
        game.setClassicHud(true);game.setKeyboardMouseControls(true);
        game.updateLocal(1000,0,{});
    };
    const auto hudClick=[&](float x,float y) {
        swgb::InputState input;input.screenW=960;input.screenH=544;
        input.pointerX=input.boxStartX=x*1.2f;input.pointerY=input.boxStartY=340+y*1.2f;
        input.pointerPressed=input.pointerHeld=true;
        game.updateLocal(1010,.001f,input);
        input.pointerPressed=input.pointerHeld=false;input.selectPressed=true;
        game.updateLocal(1011,.001f,input);
    };
    const auto targetClick=[&](uint32_t target) {
        check(game.lookAtObject(target),"Center camera on command target");
        swgb::InputState input;input.screenW=960;input.screenH=544;
        check(game.objectScreenPosition(target,960,544,input.pointerX,input.pointerY),"Resolve actual command target screen position");
        input.pointerY-=12;input.selectPressed=true;input.cursorVisible=true;
        game.updateLocal(1020,.009f,input);
    };
    const auto changeViewAndSelection=[&](uint32_t other) {
        check(game.selectObjectForTesting(other),"Change selection after emitting target command");
        game.lookAt(45,45);
    };

    reset();
    check(game.selectObjectForTesting(11),"Select patrol actor");
    check(game.classicHudIconForTesting(0)==6,"Patrol uses original top-left command slot");
    hudClick(26,23);
    const auto patrolDestination=game.objectPosition(12);
    targetClick(12);
    check(!game.patrolActiveForTesting(11),"Patrol endpoint click remains pending between ticks");
    changeViewAndSelection(13);
    game.updateLocal(1050,.03f,{});
    check(game.patrolActiveForTesting(11) && !game.patrolActiveForTesting(13),
          "Admitted patrol belongs to captured actor after selection and camera changes");
    const auto patrolAnchor=game.moveAnchorForTesting(11);
    const float patrolDx=patrolAnchor[0]-patrolDestination[0],patrolDy=patrolAnchor[1]-patrolDestination[1];
    check(patrolDx*patrolDx+patrolDy*patrolDy<16.f,
          "Patrol retains the world endpoint captured before camera movement");

    reset();
    check(game.selectObjectForTesting(11),"Select guard actor");
    check(game.classicHudIconForTesting(1)==7,"Guard uses original second command slot");
    hudClick(67,23);targetClick(12);
    check(game.guardTargetForTesting(11)==0,"Guard target click remains pending between ticks");
    changeViewAndSelection(13);
    game.updateLocal(1050,.03f,{});
    check(game.guardTargetForTesting(11)==12 && game.guardTargetForTesting(13)==0,
          "Guard dispatch retains clicked actor and target IDs instead of repicking at the new camera");

    reset();
    check(game.selectObjectForTesting(11),"Select stale guard actor");
    hudClick(67,23);targetClick(12);
    check(game.guardTargetForTesting(11)==0,"Stale-target control starts with pending guard only");
    check(game.damageObjectForTesting(12,100000) && !game.objectActive(12),"Kill captured target before dispatch");
    changeViewAndSelection(13);
    game.updateLocal(1050,.03f,{});
    check(game.guardTargetForTesting(11)==0 && game.guardTargetForTesting(13)==0,
          "Dead captured guard target is rejected without retargeting or affecting the new selection");

    reset();
    check(game.damageObjectForTesting(4,100),"Damage actual repair target");
    check(game.selectObjectForTesting(6),"Select repair worker");
    check(game.classicHudIconForTesting(7)==28,"Repair uses original worker slot7");
    hudClick(149,64);targetClick(4);
    check(game.repairTargetForTesting(6)==0,"Repair target click does not assign work between ticks");
    changeViewAndSelection(7);
    game.updateLocal(1050,.03f,{});
    check(game.repairTargetForTesting(6)==4 && game.repairTargetForTesting(7)==0,
          "Repair dispatch retains the clicked building and worker through camera and selection changes");
    std::puts("PASS: deferred patrol, guard and repair targets, captured actors/camera independence and stale target rejection");
}
