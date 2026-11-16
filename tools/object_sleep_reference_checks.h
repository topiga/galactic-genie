// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

static void objectSleepReferenceChecks(swgb::Assets &assets) {
    // Actual parent/child controls: tree-action-reference-lifecycle-findings-2026-11-17.json.
    swgb::Game game(assets); std::string error;
    check(game.init(7,96,&error,1,1),error.c_str());
    const auto worker=game.spawnObjectForTesting(1,83,1,27.f,40.5f);
    const auto tree=game.spawnObjectForTesting(0,349,0,27.5f,40.5f);
    check(game.objectUpdateListForTesting(tree)==1 && game.objectTaskReferencesForTesting(tree)==0,
          "Healthy base constructor starts asleep without task references");
    for (int slot : {0,2,1}) {
        check(game.setGatherReferenceForTesting(worker,tree,slot),"Acquire distinct action slot");
        check(game.objectUpdateListForTesting(tree)==0,"Acquiring target wakes into primary list");
    }
    check(game.objectTaskReferencesForTesting(tree)==3,"Parent primary, secondary and child each own a reference");
    check(game.setGatherReferenceForTesting(worker,tree,1) && game.objectTaskReferencesForTesting(tree)==3,
          "Reassigning the same pointer within one action slot is a no-op");
    for (int slot : {1,2,0}) game.setGatherReferenceForTesting(worker,0,slot);
    check(game.objectTaskReferencesForTesting(tree)==0 && game.objectPendingSleepForTesting(tree) &&
          game.objectUpdateListForTesting(tree)==0,"Last release requests deferred sleep without moving lists");
    game.setGatherReferenceForTesting(worker,tree,0);
    check(!game.objectPendingSleepForTesting(tree) && game.objectUpdateListForTesting(tree)==0,
          "Reacquiring before migration cancels pending sleep");
    game.setGatherReferenceForTesting(worker,0,0);
    game.updatePaused(1.f,{});
    check(game.objectPendingSleepForTesting(tree),"Pause does not commit pending owner-list migration");
    game.update(.1f,{});
    check(game.objectUpdateListForTesting(tree)==1 && !game.objectPendingSleepForTesting(tree),
          "Next owner update commits sleep migration");
    check(game.transferObjectForTesting(tree,1) && game.objectUpdateListForTesting(tree)==1 &&
          game.transferObjectForTesting(tree,0),"Owner transfer preserves sleeping membership");

    const auto second=game.spawnObjectForTesting(1,83,1,27.f,40.8f);
    check(game.issueGatherForTesting(worker,tree) && game.issueGatherForTesting(second,tree),
          "Two workers acquire the same resource");
    check(game.objectTaskReferencesForTesting(tree)==2,"Each worker owns an independent gather parent");
    game.update(.1f,{});
    check(game.objectTaskReferencesForTesting(tree)==4,"Each felling child owns an additional reference");
    game.stopUnitForTesting(second);
    check(game.objectTaskReferencesForTesting(tree)==2 && !game.objectPendingSleepForTesting(tree),
          "Stopping one worker releases its child and parent without sleeping a shared target");
    game.stopUnitForTesting(worker);
    check(game.objectTaskReferencesForTesting(tree)==0 && game.objectPendingSleepForTesting(tree),
          "Stop releases the final worker's child and parent");

    swgb::Game lethal(assets);
    check(lethal.init(7,96,&error,1,1),error.c_str());
    const auto cutter=lethal.spawnObjectForTesting(1,83,1,27.f,40.5f);
    const auto target=lethal.spawnObjectForTesting(0,349,0,27.5f,40.5f);
    check(lethal.issueGatherForTesting(cutter,target),"Begin lethal cancellation fixture");
    for(int tick=0;tick<25;++tick)lethal.update(.1f,{});
    check(lethal.objectHitPoints(target)==0.f && !lethal.objectFelled(target) &&
          lethal.objectTaskReferencesForTesting(target)==2,"Lethal attack leaves state2 with two references before Gaia admission");
    lethal.stopUnitForTesting(cutter);
    lethal.update(.1f,{});
    check(lethal.objectUpdateListForTesting(target)==1 && !lethal.objectFelled(target) &&
          lethal.objectHitPoints(target)==0.f,"Lethal cancellation sleeps before death admission, despite pending felling");
    check(lethal.issueGatherForTesting(cutter,target),"Reacquire the sleeping zero-HP resource");
    lethal.update(.1f,{});
    check(lethal.objectFelled(target) && lethal.objectUpdateListForTesting(target)==0,
          "Reacquiring wakes the target and permits its deferred death admission");
    lethal.stopUnitForTesting(cutter);
    check(!lethal.objectPendingSleepForTesting(target),"State3 release does not request base sleep");

    // Rejected drop-off must not destroy a valid gather controller.
    for(int tick=0;tick<12;++tick)lethal.update(.1f,{});
    lethal.issueGatherForTesting(cutter,target);
    for(int tick=0;tick<12;++tick)lethal.update(.1f,{});
    const auto barracks=lethal.spawnObjectForTesting(1,12,1,32.f,40.f);
    const int beforeRejected=lethal.objectTaskReferencesForTesting(target);
    check(lethal.objectCarriedAmount(cutter)>0.f && barracks &&
          !lethal.issueDropOffForTesting(cutter,barracks) &&
          lethal.objectGatheringTarget(cutter,target) &&
          lethal.objectTaskReferencesForTesting(target)==beforeRejected,
          "Rejected drop-off preserves gather action and its target references");
    const auto commandCenter=lethal.spawnObjectForTesting(1,109,1,32.f,46.f);
    check(lethal.issueDropOffForTesting(cutter,commandCenter) &&
          lethal.objectTaskReferencesForTesting(target)==beforeRejected &&
          lethal.objectTaskReferencesForTesting(commandCenter)==1,
          "Accepted drop-off retains primary and acquires secondary");
    lethal.stopUnitForTesting(cutter);
    check(lethal.objectTaskReferencesForTesting(commandCenter)==0,
          "Stop releases secondary target without sleeping an animated building");

    for(int effectType : {12,14,15}) {
        swgb::Scenario scenario;
        scenario.map.width=scenario.map.height=32;scenario.map.tiles.resize(32*32);
        scenario.playerSlotCount=1;scenario.civilizations.fill(1);
        scenario.players[0].active=scenario.players[0].human=true;
        scenario.players[0].civilization=1;
        swgb::ScenarioUnit actor;actor.unitId=83;actor.spawnId=1;actor.player=1;
        actor.state=2;actor.x=12.f;actor.y=12.5f;scenario.units.push_back(actor);
        actor.unitId=349;actor.spawnId=2;actor.player=0;actor.x=12.5f;scenario.units.push_back(actor);
        scenario.nextUnitId=3;
        swgb::ScenarioTrigger trigger;trigger.enabled=true;
        swgb::ScenarioCondition timer;timer.type=10;timer.fields.resize(8,-1);timer.fields[7]=1;
        trigger.conditions.push_back(timer);
        swgb::ScenarioEffect remove;remove.type=effectType;remove.selectedUnitIds={1};
        remove.fields.resize(16,-1);remove.fields[14]=10;remove.fields[15]=20;
        trigger.effects.push_back(remove);scenario.triggers.push_back(trigger);
        swgb::Game removed(assets);check(removed.initScenario(scenario,&error),error.c_str());
        check(removed.issueGatherForTesting(1,2),"Trigger removal starts with an owned gather action");
        removed.update(.1f,{});
        check(removed.objectTaskReferencesForTesting(2)==2,"Trigger fixture owns parent and felling child");
        removed.update(1.f,{});
        check(removed.objectActive(1)==(effectType==12) && removed.objectTaskReferencesForTesting(2)==0 &&
              removed.objectUpdateListForTesting(2)==1,"Trigger move/kill/remove replaces the gather controller and releases its references");
    }

    // Exercise the type25 route using an animated prototype; no claim about its factory side effects.
    auto &gaia=const_cast<swgb::dat::DatFile &>(assets.dat()).civs.at(0).units;
    auto animated=std::find_if(gaia.begin(),gaia.end(),[](const auto &unit) {
        return unit.exists && unit.type==swgb::dat::UT_Flag && unit.standingGraphic[0]>=0;
    });
    check(animated!=gaia.end(),"Animated fixture for auxiliary routing");
    struct RestoreType { swgb::dat::Unit &unit; int type; ~RestoreType(){unit.type=type;} } restore{*animated,animated->type};
    animated->type=swgb::dat::UT_25;
    swgb::Game auxiliaryGame(assets);
    check(auxiliaryGame.init(7,96,&error,1,1),error.c_str());
    const auto auxiliaryWorker=auxiliaryGame.spawnObjectForTesting(1,83,1,27.f,40.5f);
    const auto auxiliary=auxiliaryGame.spawnObjectForTesting(0,animated->id,0,20.f,20.f);
    check(auxiliary && auxiliaryGame.objectUpdateListForTesting(auxiliary)==2,"Animated type25 enters auxiliary owner list");
    const float beforeAuxiliary=auxiliaryGame.objectAnimationForTesting(auxiliary)[0];
    auxiliaryGame.update(.25f,{});
    check(std::abs(auxiliaryGame.objectAnimationForTesting(auxiliary)[0]-beforeAuxiliary-.25f)<.00001f,
          "Auxiliary object advances exactly once in the world tail");
    check(auxiliaryGame.transferObjectForTesting(auxiliary,1) && auxiliaryGame.objectUpdateListForTesting(auxiliary)==2,
          "Auxiliary routing survives owner transfer");
    auxiliaryGame.setGatherReferenceForTesting(auxiliaryWorker,auxiliary,0);
    auxiliaryGame.setGatherReferenceForTesting(auxiliaryWorker,0,0);
    check(!auxiliaryGame.objectPendingSleepForTesting(auxiliary) && auxiliaryGame.objectUpdateListForTesting(auxiliary)==2,
          "Animated reference release does not inherit base sleep behavior");
    animated->type=restore.type;

    // Reference cap is the original saturating byte, not an unbounded owner count.
    std::vector<uint32_t> holders;
    for(int i=0;i<251;++i) {
        const auto id=game.spawnObjectForTesting(1,83,1,27.f,40.5f);
        holders.push_back(id);game.setGatherReferenceForTesting(id,tree,0);
    }
    check(game.objectTaskReferencesForTesting(tree)==250,"Original reference count saturates at250");
    for(auto id:holders)game.setGatherReferenceForTesting(id,0,0);
    check(game.objectTaskReferencesForTesting(tree)==0,"Releases do not underflow after saturation");
    check(game.init(7,96,&error,1,1),error.c_str());
    const auto fresh=game.spawnObjectForTesting(0,349,0,27.5f,40.5f);
    check(game.objectTaskReferencesForTesting(fresh)==0 && game.objectUpdateListForTesting(fresh)==1,
          "Session reset clears all routed lists and task references");
    std::puts("PASS: original task slots, sleep/wake, shared targets, lethal cancellation, reference cap and reset");
}
