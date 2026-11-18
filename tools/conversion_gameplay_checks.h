// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
[[gnu::noinline]] static void conversionGameplayChecks(swgb::Assets &assets,swgb::SoftRenderer &renderer) {
    swgb::Scenario scenario;scenario.map.width=scenario.map.height=64;
    scenario.map.tiles.resize(64*64);scenario.playerSlotCount=2;scenario.nextUnitId=1;
    scenario.civilizations.fill(1);
    for(size_t index=0;index<scenario.players.size();++index) {
        auto &player=scenario.players[index];player.active=index<2;player.human=true;player.civilization=1;
        player.diplomacy.fill(3);if(index<2)player.diplomacy[index+1]=0;
    }
    auto game=std::make_unique<swgb::Game>(assets);auto &g=*game;std::string error;
    check(g.initScenario(scenario,&error),error.c_str());
    g.setClassicHud(true);
    const auto jedi=g.spawnObjectForTesting(1,115,1,20,20);
    const auto padawan=g.spawnObjectForTesting(1,180,1,22,20);
    const auto troop=g.spawnObjectForTesting(1,329,2,25,20);
    const auto other=g.spawnObjectForTesting(1,329,2,26,21);
    check(jedi && padawan && troop && other,"Create actual Jedi and hostile troop assets");
    for(auto id:{jedi,padawan,troop,other})g.setAttackModeForTesting(id,3);
    g.update(0,{});
    check(g.selectObjectForTesting(jedi) && g.classicHudIconForTesting(13)==14,
          "Original Convert icon occupies original command slot13");
    check(!g.issueConversionForTesting(padawan,troop),"Padawans cannot start conversion");
    check(!g.issueConversionForTesting(jedi,padawan),"Conversion rejects friendly targets");
    int conversionSound=-1,conversionSounds=0;
    for(const auto &task:assets.dat().unitHeaders[115].tasks)
        if(task.actionType==104) {
            const auto *graphic=assets.dat().graphic(task.proceedingGraphic);
            if(graphic)conversionSound=graphic->soundId;
        }
    check(conversionSound>=0,"Actual conversion proceeding graphic supplies its original sound");
    g.setUnitSoundPlayer([&](int sound,int){if(sound==conversionSound)++conversionSounds;});
    g.lookAt(23,20);g.render(renderer,960,544);
    swgb::InputState button;button.selectPressed=true;button.pointerX=80;button.pointerY=510;
    g.updatePaused(0,button);
    check(!g.conversionCursorActive(),"Pause blocks new conversion commands");
    g.update(0,button);
    check(g.conversionCursorActive() && g.classicHudIconForTesting(14)==0,
          "Original Convert button opens cancel-only target selection");
    swgb::InputState cancel;cancel.menuBack=true;g.updatePaused(0,cancel);
    check(!g.conversionCursorActive(),"Paused Escape cancels conversion targeting");
    g.update(0,button);
    float x=0,y=0;check(g.objectScreenPosition(troop,960,544,x,y),"Locate conversion target");
    swgb::InputState click;click.selectPressed=true;click.pointerX=x;click.pointerY=y-12;
    g.update(0,click);
    check(!g.conversionCursorActive() && g.conversionTargetForTesting(jedi)==troop,
          "Original Convert button and target click issue the conversion action");
    check(g.objectTaskReferencesForTesting(troop)==1,"Conversion retains its target reference");
    g.updatePaused(20,{});
    check(g.objectPlayer(troop)==2 && g.conversionChargeForTesting(jedi)==100,
          "Pause freezes conversion and Force charge");
    const float hp=g.objectHitPoints(troop);
    const float conversions=g.resource(1,41),losses=g.resource(2,154);
    const float convertedValue=g.resource(1,170);
    float elapsed=0;bool renderedPulse=false;
    while(g.objectPlayer(troop)==2 && elapsed<40) {
        g.update(1.f/30,{});elapsed+=1.f/30;
        if(conversionSounds && !renderedPulse) {
            g.render(renderer,960,544);
            check(renderer.savePng("jedi-conversion-pulse.png"),"Render original conversion proceeding graphic");
            renderedPulse=true;
        }
    }
    check(g.objectPlayer(troop)==1 && elapsed>=4,"Real conversion pulses finish after the original minimum time");
    check(conversionSounds>=2,"Original conversion work sound plays again on repeated pulse entry");
    check(g.objectHitPoints(troop)==hp && g.privateMasterForTesting(troop),
          "Conversion preserves HP and captures the source-owner prototype");
    check(g.resource(1,41)==conversions+1 && g.resource(2,154)==losses+1 &&
          g.resource(1,170)>convertedValue && g.resource(2,98)>=0,
          "Conversion updates original success, loss and captured-value statistics");
    check(g.conversionChargeForTesting(jedi)==0 && !g.conversionTargetForTesting(jedi) &&
          g.objectTaskReferencesForTesting(troop)==0,"Success consumes Force and releases the target");
    check(!g.issueConversionForTesting(jedi,other),"An exhausted Jedi cannot start another conversion");
    g.updatePaused(20,{});
    check(g.conversionChargeForTesting(jedi)==0,"Force recovery also freezes while paused");
    for(int i=0;i<30;++i)g.update(1.f/30,{});
    check(g.conversionChargeForTesting(jedi)>0,"Force recovers during ordinary simulation updates");
    const auto fresh=g.spawnObjectForTesting(1,115,1,23,21);
    g.setAttackModeForTesting(fresh,3);g.update(0,{});
    check(g.issueConversionForTesting(fresh,other),"Start another conversion for interruption checks");
    check(g.stopUnitForTesting(fresh) && !g.conversionTargetForTesting(fresh) &&
          g.objectTaskReferencesForTesting(other)==0 && g.conversionChargeForTesting(fresh)==100,
          "Stop cancels conversion without consuming Force or retaining the target");
    check(g.issueConversionForTesting(fresh,other),"Restart conversion after Stop");
    check(g.damageObjectForTesting(other,100000),"Destroy a referenced conversion target");
    g.update(1.f/30,{});
    check(!g.conversionTargetForTesting(fresh) && g.conversionChargeForTesting(fresh)==100,
          "Target destruction cancels conversion without consuming Force");
    g.render(renderer,960,544);
    check(renderer.savePng("jedi-conversion.png"),"Render original conversion UI assets");
    auto &data=const_cast<swgb::dat::DatFile&>(assets.dat());
    auto &special=data.effects[size_t(data.techs[501].effectId)].commands;
    struct RestoreSpecial {std::vector<swgb::dat::EffectCommand>& value;std::vector<swgb::dat::EffectCommand> saved;
        ~RestoreSpecial(){value=std::move(saved);}} restoreSpecial{special,special};
    for(int retainedCharge:{0,1}) {
        special={{1,193,0,0,float(retainedCharge)}};
        check(g.initScenario(scenario,&error),error.c_str());
        check(g.researchTechnologyForTesting(1,501),"Research original friendly-conversion charge flag");
        const auto converter=g.spawnObjectForTesting(1,115,1,20,20);
        const auto victim=g.spawnObjectForTesting(1,329,2,25,20);
        g.setAttackModeForTesting(converter,3);g.setAttackModeForTesting(victim,3);g.update(0,{});
        check(g.issueConversionForTesting(converter,victim) && g.convertObjectForTesting(victim,1),
              "Another conversion makes the retained target friendly before this actor's pulse");
        g.update(1.f/30,{});
        check(!g.conversionTargetForTesting(converter) && g.objectTaskReferencesForTesting(victim)==0 &&
              g.conversionChargeForTesting(converter)==(retainedCharge?100.f:0.f),
              "Friendly target cancels and applies current owner's resource193 charge policy");
    }
    special={{1,192,0,0,1}};
    check(g.initScenario(scenario,&error),error.c_str());
    check(g.researchTechnologyForTesting(2,501),"Research original conversion martyr flag on target owner");
    const auto converter=g.spawnObjectForTesting(1,115,1,20,20);
    const auto victim=g.spawnObjectForTesting(1,329,2,25,20);
    g.setAttackModeForTesting(converter,3);g.setAttackModeForTesting(victim,3);g.update(0,{});
    const float successes=g.resource(1,41),martyrLoss=g.resource(2,154);
    check(g.issueConversionForTesting(converter,victim),"Martyr target accepts an ordinary real conversion action");
    float martyrElapsed=0;
    while(g.objectActive(victim) && martyrElapsed<40) {g.update(1.f/30,{});martyrElapsed+=1.f/30;}
    check(!g.objectActive(victim) && !g.privateMasterForTesting(victim) && martyrElapsed>=4,
          "Original resource192 successful conversion kills instead of transferring the target");
    check(g.resource(1,41)==successes+1 && g.resource(2,154)==martyrLoss+1 &&
          g.conversionChargeForTesting(converter)==0 && !g.conversionTargetForTesting(converter) &&
          g.objectTaskReferencesForTesting(victim)==0,
          "Martyr success retains conversion statistics and consumes Force while releasing references");
    auto &recipientCost=data.effects[size_t(data.techs[174].effectId)].commands;
    RestoreSpecial restoreRecipientCost{recipientCost,recipientCost};
    special={{4,329,-1,103,17},{4,329,-1,104,19}};
    recipientCost={{4,329,-1,103,91}};
    check(g.initScenario(scenario,&error),error.c_str());
    check(g.researchTechnologyForTesting(2,501),"Source owner researches distinct troop costs before capture");
    const auto firstJedi=g.spawnObjectForTesting(1,115,1,20,20);
    const auto captured=g.spawnObjectForTesting(1,329,2,25,20);
    g.setAttackModeForTesting(firstJedi,3);g.setAttackModeForTesting(captured,3);g.update(0,{});
    float capturedCost=0;
    for(const auto &cost:g.unitCostsForTesting(2,data.civs[1].units[329]))
        if((cost.flag&0xff)==1 && cost.amount>0)capturedCost+=cost.amount;
    const float firstValue=g.resource(1,170);
    check(g.issueConversionForTesting(firstJedi,captured),"First real conversion captures source cost modifiers");
    for(int tick=0;tick<1200 && g.objectPlayer(captured)==2;++tick)g.update(1.f/30,{});
    check(g.objectPlayer(captured)==1 && g.resource(1,170)==firstValue+capturedCost,
          "First conversion records the original source's full modified unit value");
    check(g.researchTechnologyForTesting(1,174),"Recipient changes its shared cost after acquiring a private unit");
    float recipientSharedCost=0;
    for(const auto &cost:g.unitCostsForTesting(1,data.civs[1].units[329]))
        if((cost.flag&0xff)==1 && cost.amount>0)recipientSharedCost+=cost.amount;
    check(recipientSharedCost!=capturedCost,"Reconversion fixture distinguishes recipient shared and captured private prices");
    const auto secondJedi=g.spawnObjectForTesting(1,115,2,30,20);
    g.setAttackModeForTesting(secondJedi,3);g.update(0,{});
    const float secondValue=g.resource(2,170);
    check(g.issueConversionForTesting(secondJedi,captured),"Other owner begins a real conversion of the captured private unit");
    for(int tick=0;tick<1200 && g.objectPlayer(captured)==1;++tick)g.update(1.f/30,{});
    check(g.objectPlayer(captured)==2 && g.resource(2,170)==secondValue+capturedCost &&
          g.privateMasterForTesting(captured),
          "Repeated conversion values the private source snapshot instead of the recipient's later shared research");
    std::puts("PASS: original conversion UI, approach, pulses, ownership, charge, pause and cancellation");
}
