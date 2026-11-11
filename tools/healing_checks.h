// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

static void healingChecks(swgb::Assets &assets, swgb::SoftRenderer &renderer) {
    swgb::Game game(assets); std::string error;
    check(game.init(0x5A17,64,&error,1,1),error.c_str());
    const auto medic=game.spawnObjectForTesting(1,939,1,10.5f,10.5f);
    const auto troop=game.spawnObjectForTesting(1,307,1,11.5f,10.5f);
    const auto building=game.spawnObjectForTesting(1,70,1,14.5f,10.5f);
    const auto droid=game.spawnObjectForTesting(5,309,1,12.5f,10.5f);
    check(medic && troop && building && droid,"Healing fixtures");
    const float full=game.objectHitPoints(troop);
    check(game.damageObjectForTesting(troop,10),"Wound friendly infantry");
    game.damageObjectForTesting(building,10); game.damageObjectForTesting(droid,10);
    check(!game.issueHealForTesting(medic,building),"Medic cannot repair buildings");
    check(!game.issueHealForTesting(medic,droid),"Converted droid infantry still require repair");
    check(!game.issueHealForTesting(medic,medic),"Medic cannot heal itself");
    // Matrix classes; hero regeneration and some rare targets still need isolated runtime checks.
    for (int id : {170,252,552,383,403,4,2,640,941}) {
        const auto patient=game.spawnObjectForTesting(2,id,1,10.5f,11.5f);
        check(patient && game.damageObjectForTesting(patient,10),"Wound reference-matrix patient");
        const float before=game.objectHitPoints(patient);
        check(game.issueHealForTesting(medic,patient),"Accept reference-matrix healing class");
        for(int tick=0;tick<10;++tick) game.update(.1f,{});
        check(game.objectHitPoints(patient)>before,"Reference-matrix patient actually gains health");
        game.damageObjectForTesting(patient,100000);
    }
    const auto power=game.spawnObjectForTesting(7,1007,1,10.5f,11.5f);
    game.damageObjectForTesting(power,10);
    check(!game.issueHealForTesting(medic,power),"Power droids do not sustain healing in reference");
    game.damageObjectForTesting(power,100000);
    game.damageObjectForTesting(troop,10);
    const float wounded=game.objectHitPoints(troop);
    check(game.issueHealForTesting(medic,troop),"Medic accepts wounded friendly infantry");
    game.updatePaused(10,{});
    check(game.objectHitPoints(troop)==wounded,"Pause freezes healing");
    for(int i=0;i<20;++i) game.update(.1f,{});
    check(std::abs(game.objectHitPoints(troop)-wounded-4.f)<.02f,
          "Original medic timing: two HP per simulated second");
    for(int i=0;i<100;++i) game.update(.1f,{});
    check(game.objectHitPoints(troop)==full,"Healing stops at maximum health");
    game.damageObjectForTesting(troop,10);
    for(int i=0;i<100;++i) game.update(.1f,{});
    check(game.objectHitPoints(troop)==full,"Idle medic automatically heals nearby wounded units");
    // Exercise the real mouse command path, with the medic outside automatic search range.
    game.setKeyboardMouseControls(true); game.setClassicHud(true);
    game.moveObjectForTesting(medic,8.5f,10.5f);
    game.moveObjectForTesting(troop,18.5f,10.5f);
    game.damageObjectForTesting(troop,10);
    game.lookAt(14.5f,10.5f); game.render(renderer,960,544);
    check(game.selectObjectForTesting(medic),"Select medic for context order");
    check(game.classicHudIconForTesting(0)==15,"Original Heal icon occupies top-left slot");
    check(game.classicHudIconForTesting(1)<0 && game.classicHudIconForTesting(2)<0,
          "Medic has no Guard or Follow combat controls");
    swgb::InputState hotkey; hotkey.healPressed=true;
    game.updatePaused(0,hotkey);
    check(!game.healingCursorActive(),"Pause blocks Heal hotkey");
    game.update(0,hotkey);
    check(game.healingCursorActive() && game.classicHudIconForTesting(14)==0 &&
          game.classicHudIconForTesting(0)<0,"Heal hotkey opens original cancel-only command grid");
    swgb::InputState cancel; cancel.menuBack=true; game.update(0,cancel);
    check(!game.healingCursorActive(),"Escape cancels healing target selection");
    game.update(0,hotkey); game.updatePaused(0,cancel);
    check(!game.healingCursorActive(),"Paused Escape cancels targeting without issuing an order");
    float x,y; check(game.objectScreenPosition(troop,960,544,x,y),"Locate healing target");
    swgb::InputState input; input.commandPressed=true; input.pointerX=x; input.pointerY=y-12;
    game.update(0,input);
    const float distantHealth=game.objectHitPoints(troop);
    game.update(.1f,{});
    check(game.objectHitPoints(troop)==distantHealth,"Medic cannot heal outside range");
    for(int i=0;i<350;++i) game.update(.1f,{});
    check(game.objectHitPoints(troop)==full,"Right-click medic approaches and heals target");
    game.damageObjectForTesting(troop,10);
    game.moveObjectForTesting(medic,8.5f,10.5f);
    game.lookAt(14.5f,10.5f); game.render(renderer,960,544);
    swgb::InputState healButton; healButton.selectPressed=true;
    healButton.pointerX=25; healButton.pointerY=364;
    game.update(0,healButton);
    check(game.healingCursorActive(),"Original Heal button starts targeting");
    check(game.objectScreenPosition(troop,960,544,x,y),"Locate explicit heal target");
    swgb::InputState healTarget; healTarget.selectPressed=true; healTarget.pointerX=x; healTarget.pointerY=y-12;
    game.update(0,healTarget);
    check(!game.healingCursorActive(),"Left click assigns explicit healing target");
    for(int i=0;i<350;++i) game.update(.1f,{});
    check(game.objectHitPoints(troop)==full,"Heal button and target click restore health");
    game.render(renderer,960,544);
    check(renderer.savePng("medic-healing.png"),"Render medic fixture with original assets");
    game.moveObjectForTesting(medic,10.5f,10.5f);
    game.setDiplomacyForTesting(1,2,0);
    const auto ally=game.spawnObjectForTesting(1,307,2,11.5f,12.5f);
    game.damageObjectForTesting(ally,10);
    check(game.issueHealForTesting(medic,ally),"Medic accepts allied infantry");
    game.setDiplomacyForTesting(1,2,3);
    check(!game.issueHealForTesting(medic,ally),"Medic rejects enemy infantry");
    check(!game.canAttackTargetForTesting(medic,ally) && !game.issueAttackForTesting(medic,ally),
          "Medics cannot attack even when original DAT contains attack fields");
    check(!game.issueAttackGroundForTesting(medic,11.5f,12.5f),"Medic cannot attack ground");
    const float enemyHealth=game.objectHitPoints(ally);
    game.update(.1f,{});
    check(game.objectHitPoints(ally)<=enemyHealth,"Diplomacy change cancels healing");
    check(game.attackTargetForTesting(medic)==0,"Medic never acquires an enemy attack target");
    for (int civ : {1,7}) {
        swgb::Game rates(assets);
        check(rates.init(0x5A17,64,&error,civ,7),error.c_str());
        const auto healer=rates.spawnObjectForTesting(civ,civ==7?996:939,1,10.5f,10.5f);
        for (int patientId : {418,4}) {
            const auto patient=rates.spawnObjectForTesting(civ,patientId,1,11.5f,10.5f);
            rates.damageObjectForTesting(patient,25);
            const float before=rates.objectHitPoints(patient);
            check(rates.issueHealForTesting(healer,patient),"Rate fixture accepts patient");
            for(int i=0;i<20;++i) rates.update(.1f,{});
            check(std::abs(rates.objectHitPoints(patient)-before-(civ==7?8.f:4.f))<.02f,
                  "Healing is HP per second, with Republic bonus independent of target maximum HP");
            rates.damageObjectForTesting(patient,100000);
        }
        rates.setDiplomacyForTesting(1,2,0); rates.setDiplomacyForTesting(2,1,0);
        const auto patient=rates.spawnObjectForTesting(civ,4,1,11.5f,10.5f);
        rates.damageObjectForTesting(patient,100);
        float before=rates.objectHitPoints(patient);
        check(rates.issueHealForTesting(healer,patient),"Allied Republic rate fixture");
        for(int i=0;i<20;++i) rates.update(.1f,{});
        check(std::abs(rates.objectHitPoints(patient)-before-8.f)<.02f,
              "Republic team bonus applies once, including to allied medics");
        if(civ==7) {
            check(rates.researchTechnologyForTesting(1,578),"Research Upgraded Med Droids");
            before=rates.objectHitPoints(patient);
            for(int i=0;i<20;++i) rates.update(.1f,{});
            check(std::abs(rates.objectHitPoints(patient)-before-9.6f)<.02f,
                  "Upgraded Med Droids heals at 4.8 HP per second");
        }
    }
    swgb::Game garrison(assets);
    check(garrison.init(0x5A17,64,&error,1,1),error.c_str());
    const auto center=garrison.spawnObjectForTesting(1,109,1,10.5f,10.5f);
    const auto passenger=garrison.spawnObjectForTesting(1,307,1,14.5f,10.5f);
    check(garrison.garrisonForTesting(passenger,center),"Order patient into Command Center");
    for(int i=0;i<300 && garrison.garrisonContainerForTesting(passenger)<0;++i)
        garrison.update(.1f,{});
    check(garrison.garrisonContainerForTesting(passenger)==int(center),"Patient reaches garrison");
    garrison.damageObjectForTesting(passenger,20);
    float before=garrison.objectHitPoints(passenger);
    garrison.updatePaused(10,{});
    check(garrison.objectHitPoints(passenger)==before,"Pause freezes garrison healing");
    for(int i=0;i<20;++i) garrison.update(.1f,{});
    check(std::abs(garrison.objectHitPoints(passenger)-before-.6f)<.02f,
          "Original Command Center heals 0.3 HP per second");
    check(garrison.researchTechnologyForTesting(1,501),"Research Bacta Tanks");
    before=garrison.objectHitPoints(passenger);
    for(int i=0;i<20;++i) garrison.update(.1f,{});
    check(std::abs(garrison.objectHitPoints(passenger)-before-2.4f)<.02f,
          "Bacta Tanks multiplies building healing by four");
    check(garrison.ejectGarrisonedUnitForTesting(center,passenger),"Eject healed passenger");
    before=garrison.objectHitPoints(passenger);
    for(int i=0;i<20;++i) garrison.update(.1f,{});
    check(garrison.objectHitPoints(passenger)==before,"Garrison healing ends on leaving building");
    swgb::Game heroes(assets);
    check(heroes.init(0x5A17,64,&error,1,1),error.c_str());
    const auto hero=heroes.spawnObjectForTesting(1,252,1,10.5f,10.5f);
    const auto royal=heroes.spawnObjectForTesting(1,4,1,12.5f,10.5f);
    heroes.damageObjectForTesting(hero,10); heroes.damageObjectForTesting(royal,10);
    before=heroes.objectHitPoints(hero);
    const float royalBefore=heroes.objectHitPoints(royal);
    for(int i=0;i<7;++i) heroes.update(.25f,{});
    check(heroes.objectHitPoints(hero)==before,"Hero recovery is discrete, not continuous");
    heroes.updatePaused(10,{});
    check(heroes.objectHitPoints(hero)==before,"Pause freezes hero recovery timer");
    heroes.update(.25f,{});
    check(heroes.objectHitPoints(hero)==before+1.f && heroes.objectHitPoints(royal)==royalBefore,
          "Hero flag grants one HP per two seconds; special units without flag do not regenerate");
    const auto capped=heroes.spawnObjectForTesting(1,252,1,14.5f,10.5f);
    const float cap=heroes.objectHitPoints(capped);
    heroes.damageObjectForTesting(capped,1);
    heroes.update(2.5f,{});
    check(heroes.objectHitPoints(capped)==cap,"Hero recovery stops at maximum health");
    heroes.update(10.f,{});
    heroes.damageObjectForTesting(capped,5.f);
    heroes.update(1.25f,{});
    check(heroes.objectHitPoints(capped)==cap-5.f,"Full health freezes the retained recovery remainder");
    heroes.update(.25f,{});
    check(heroes.objectHitPoints(capped)==cap-4.f,"Hero retains partial recovery timer across wounds");
    heroes.damageObjectForTesting(capped,100000);
    const float deadHealth=heroes.objectHitPoints(capped);
    heroes.update(10.f,{});
    check(heroes.objectHitPoints(capped)==deadHealth,"Recovery does not revive dead heroes");
    std::printf("PASS: medic UI, Republic bonus/upgrade, Bacta garrison healing, hero recovery and pause\n");
}
