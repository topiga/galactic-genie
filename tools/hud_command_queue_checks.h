// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

static void hudCommandQueueChecks(swgb::Assets &assets) {
    swgb::Game game(assets);std::string error;
    const auto reset=[&] {
        check(game.initSkirmish(0x5A17,&error),error.c_str());
        game.setClassicHud(true);game.setKeyboardMouseControls(true);
        game.updateLocal(1000,0,{});
    };
    // Original800x170 panel scaled to the existing960x544 test viewport.
    const auto click=[&](uint32_t timestamp,float x,float y) {
        swgb::InputState input;
        input.screenW=960;input.screenH=544;
        input.pointerX=input.boxStartX=x*1.2f;
        input.pointerY=input.boxStartY=340+y*1.2f;
        input.pointerPressed=input.pointerHeld=true;
        game.updateLocal(timestamp,.001f,input);
        input.pointerPressed=input.pointerHeld=false;input.selectPressed=true;
        game.updateLocal(timestamp+1,.001f,input);
    };
    const auto moving=[&](uint32_t id) {
        const auto units=game.movingObjects();
        return std::any_of(units.begin(),units.end(),[&](const swgb::MovingObjectInfo &unit){return unit.spawnId==id;});
    };

    reset();
    auto position=game.objectPosition(11);
    game.groupMoveForTesting({11,12},position[0]+8,position[1]+4);
    check(moving(11) && moving(12),"HUD stop fixture has two actual movement orders");
    check(game.selectObjectForTesting(11),"Select stop command actor");
    check(game.classicHudIconForTesting(3)==3,"Original Stop button is in slot3");
    click(1010,149,23);
    check(moving(11) && moving(12),"HUD Stop does not change orders between admitted ticks");
    check(game.selectObjectForTesting(12),"Change selection after queued Stop");
    game.updateLocal(1050,.04f,{});
    check(!moving(11) && moving(12),"Queued Stop retains clicked actor despite later selection changes");

    reset();
    check(game.selectObjectForTesting(11),"Select stance command actor");
    const int oldStanceIcon=game.classicHudIconForTesting(7);
    click(1010,149,64); // Passive stance, original slot7.
    check(game.classicHudIconForTesting(7)==oldStanceIcon,"HUD stance remains unchanged before command dispatch");
    check(game.selectObjectForTesting(12),"Change selection after queued stance");
    game.updateLocal(1050,.04f,{});
    check(game.classicHudIconForTesting(7)==oldStanceIcon,"Queued stance does not affect the new selection");
    check(game.selectObjectForTesting(11),"Inspect captured stance actor");
    check(game.classicHudIconForTesting(7)!=oldStanceIcon,"Captured actor receives the queued stance at the admitted tick");

    reset();
    const std::vector<uint32_t> formationActors={11,12,13,14};
    position=game.objectPosition(11);
    game.groupMoveForTesting(formationActors,position[0]+8,position[1]+4,0);
    std::vector<std::array<float,2>> oldAnchors;
    for(uint32_t id:formationActors) oldAnchors.push_back(game.moveAnchorForTesting(id));
    check(game.selectObjectsForTesting(formationActors),"Select captured military formation group");
    const auto unrelatedAnchor=game.moveAnchorForTesting(6);
    click(1010,67,105); // Box formation, original slot9.
    check(game.formationForTesting()==1,"Formation preference updates immediately in the HUD");
    for(size_t i=0;i<formationActors.size();++i)
        check(game.moveAnchorForTesting(formationActors[i])==oldAnchors[i],"Formation click does not reorganize movement before dispatch");
    check(game.selectObjectForTesting(6),"Change selection after pending formation");
    game.updateLocal(1050,.04f,{});
    bool anchorsChanged=false;
    for(size_t i=0;i<formationActors.size();++i)
        anchorsChanged|=game.moveAnchorForTesting(formationActors[i])!=oldAnchors[i];
    check(anchorsChanged && game.moveAnchorForTesting(6)==unrelatedAnchor,
          "Queued Box formation reorganizes captured troops rather than the newly selected worker");

    reset();
    const auto center=game.commandCenterId(1);
    check(game.selectObjectForTesting(center),"Select alarm command center");
    check(!game.alarmActiveForTesting(center),"Alarm starts off");
    click(1010,108,146);click(1020,108,146);
    check(!game.alarmActiveForTesting(center),"Repeated alarm clicks remain pending between ticks");
    check(game.selectObjectForTesting(11),"Change selection after queued alarm");
    game.updateLocal(1050,.03f,{});
    check(game.alarmActiveForTesting(center),"Queued alarm captures desired enabled state rather than toggling twice on execution");

    reset();
    check(game.selectObjectForTesting(center) && game.setGatherPointForTesting(center,20,20),"Set rally removal fixture");
    click(1010,67,146); // Enter targeting mode: UI state is immediate.
    check(game.gatherPointCursorActiveForTesting(),"Rally targeting opens between ticks");
    click(1020,26,23);
    check(game.gatherPointActiveForTesting(center),"Rally destination survives until queued removal dispatch");
    check(game.selectObjectForTesting(11),"Change selection after queued rally removal");
    game.updateLocal(1050,.03f,{});
    check(!game.gatherPointActiveForTesting(center),"Queued removal retains the original rally building");

    reset();
    check(game.selectObjectForTesting(center),"Select paid production fixture");
    const auto recruits=game.productionOptionIds(center);
    check(!recruits.empty(),"Command center has recruit option");
    const int slot=assets.dat().civs[1].units[size_t(recruits.front())].buttonId-1;
    check(slot>=0 && slot<12,"Recruit has original HUD slot");
    const float food=game.resource(1,0);
    click(1010,20+(slot%4)*42,20+(slot/4)*42);
    check(game.productionQueueSizeForTesting(center)==0 && game.resource(1,0)==food,"Paid training remains pending until admission");
    game.updateLocal(1050,.03f,{});
    check(game.productionQueueSizeForTesting(center)==1 && game.resource(1,0)<food,"Admitted HUD training charges and queues the recruit");
    click(1060,200,125);
    check(game.productionQueueSizeForTesting(center)==1 && game.resource(1,0)<food,"Queue cancellation and refund stay pending between ticks");
    check(game.selectObjectForTesting(11),"Change selection after pending cancellation");
    game.updateLocal(1100,.04f,{});
    check(game.productionQueueSizeForTesting(center)==0 && game.resource(1,0)==food,"Cancellation refunds the captured building after selection change");
    reset();
    const auto centerPosition=game.objectPosition(center);
    check(game.moveObjectForTesting(11,centerPosition[0]+2.5f,centerPosition[1]) &&
          game.garrisonForTesting(11,center),"Place actual passenger next to the command center and order boarding");
    for(int step=0;step<1800 && game.garrisonedCount(center)<1;++step)game.update(1.f/30,{});
    check(game.garrisonedCount(center)==1 && game.garrisonContainerForTesting(11)==int32_t(center),
          "Portrait fixture contains exactly the intended passenger");
    check(game.selectObjectForTesting(center),"Select occupied command center");
    click(1010,295,62); // Single portrait occupies275..315;333 is the second column.
    check(game.garrisonedCount(center)==1 && game.garrisonContainerForTesting(11)==int32_t(center),
          "Individual portrait click does not eject between admitted ticks");
    check(game.selectObjectForTesting(12),"Change selection after pending passenger ejection");
    game.updateLocal(1050,.04f,{});
    check(game.garrisonedCount(center)==0 && game.garrisonContainerForTesting(11)<0,
          "Individual ejection retains the captured passenger and container after selection changes");
    std::puts("PASS: between-tick classic HUD Stop, stance, formation, alarm, rally, production cancellation and individual ejection");
}
