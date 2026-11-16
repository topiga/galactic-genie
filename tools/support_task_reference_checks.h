// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

static void supportTaskReferenceChecks(swgb::Assets &assets) {
    swgb::Game game(assets);std::string error;
    check(game.init(7,96,&error,1,1),error.c_str());
    const auto worker=game.spawnObjectForTesting(1,83,1,27.f,40.5f);
    const auto building=game.spawnObjectForTesting(1,70,1,28.5f,40.5f);
    check(game.setBuildingFrozenForTesting(building,true) && game.objectFrozenForTesting(building) &&
          game.objectPendingSleepForTesting(building) && game.objectUpdateListForTesting(building)==0,
          "Explicit building freeze requests deferred sleep");
    check(game.setBuildingFrozenForTesting(building,true) && game.objectPendingSleepForTesting(building),
          "Repeated freeze preserves its pending transition");
    game.update(.1f,{});
    check(game.objectUpdateListForTesting(building)==1 && game.objectFrozenForTesting(building),
          "Frozen building commits migration and remains asleep");
    check(game.damageObjectForTesting(building,1) && !game.objectFrozenForTesting(building) &&
          game.objectUpdateListForTesting(building)==0,"Nonlethal building damage thaws before applying damage");
    game.setBuildingFrozenForTesting(building,true);
    check(game.issueRepairForTesting(worker,building) && !game.objectFrozenForTesting(building) &&
          !game.objectPendingSleepForTesting(building),"Repair acquisition cancels pending frozen migration");
    game.stopUnitForTesting(worker);
    game.setBuildingFrozenForTesting(building,true);game.update(.1f,{});
    check(game.issueRepairForTesting(worker,building) && game.objectUpdateListForTesting(building)==0 &&
          !game.objectFrozenForTesting(building),"Repair acquisition wakes an already migrated frozen building");
    game.stopUnitForTesting(worker);
    game.setBuildingFrozenForTesting(building,true);
    check(game.transferObjectForTesting(building,0) && game.objectFrozenForTesting(building) &&
          game.transferObjectForTesting(building,1),"Frozen membership survives owner transfer");
    check(game.setBuildingFrozenForTesting(building,false) && game.objectUpdateListForTesting(building)==0,
          "Explicit thaw restores primary membership after transferred pending sleep");
    check(game.damageObjectForTesting(building,20) && game.issueRepairForTesting(worker,building),
          "Accept repair reference fixture");
    check(game.objectTaskReferencesForTesting(building)==1,"Repair parent acquires its building target");
    check(!game.issueRepairForTesting(worker,worker) && game.objectTaskReferencesForTesting(building)==1,
          "Rejected repair preserves the current parent reference");
    check(game.issueRepairForTesting(worker,building) && game.objectTaskReferencesForTesting(building)==1,
          "Replacing repair on the same target does not accumulate references");
    check(game.stopUnitForTesting(worker) && game.objectTaskReferencesForTesting(building)==0,
          "Stop releases repair target");
    check(game.issueRepairForTesting(worker,building),"Restart repair before worker removal");
    check(game.damageObjectForTesting(worker,100000) && game.objectTaskReferencesForTesting(building)==0,
          "Worker death releases repair references");

    const auto medic=game.spawnObjectForTesting(1,939,1,45.f,40.5f);
    const auto patient=game.spawnObjectForTesting(1,307,1,45.5f,40.5f);
    check(game.damageObjectForTesting(patient,10) && game.issueHealForTesting(medic,patient),
          "Accept heal reference fixture");
    check(game.objectTaskReferencesForTesting(patient)==1,"Heal parent acquires patient");
    check(!game.issueHealForTesting(medic,building) && game.objectTaskReferencesForTesting(patient)==1,
          "Rejected healing leaves current target ownership intact");
    check(game.stopUnitForTesting(medic) && game.objectTaskReferencesForTesting(patient)==0,
          "Stop releases healer target");
    check(game.issueHealForTesting(medic,patient),"Restart healing");
    for(int tick=0;tick<80;++tick)game.update(.1f,{});
    check(game.objectHitPoints(patient)==game.objectMaxHitPoints(patient) &&
          game.objectTaskReferencesForTesting(patient)==0,"Healing completion releases target on the next controller update");

    const auto builder=game.spawnObjectForTesting(1,83,1,60.f,60.f);
    const auto foundation=game.spawnFoundationForTesting(1,70,1,62.f,60.f,{builder});
    check(foundation && game.objectTaskReferencesForTesting(foundation)==1,
          "Successful builder assignment acquires foundation");
    check(game.stopUnitForTesting(builder) && game.objectTaskReferencesForTesting(foundation)==0,
          "Stopping construction releases foundation reference");
    const auto finisher=game.spawnObjectForTesting(1,83,1,70.f,60.f);
    const auto finished=game.spawnFoundationForTesting(1,70,1,72.f,60.f,{finisher});
    check(finished && game.objectTaskReferencesForTesting(finished)==1 &&
          game.completeFoundationForTesting(finished),"Finish a referenced foundation");
    game.update(.1f,{});
    check(game.objectTaskReferencesForTesting(finished)==0,
          "Building completion releases the builder's original primary slot");
    const auto orphan=game.spawnObjectForTesting(1,83,1,60.f,70.f);
    const auto destroyed=game.spawnFoundationForTesting(1,70,1,62.f,70.f,{orphan});
    check(destroyed && game.objectTaskReferencesForTesting(destroyed)==1 &&
          game.damageObjectForTesting(destroyed,100000),"Destroy a referenced foundation");
    game.update(.1f,{});
    check(game.objectTaskReferencesForTesting(destroyed)==0,
          "Invalid construction target cancels builder action and releases its reference");
    const auto wall=game.spawnFoundationForTesting(1,72,1,80.f,60.f,{});
    check(wall && game.completeFoundationForTesting(wall),"Complete original light wall");
    game.update(.1f,{});
    check(game.objectFrozenForTesting(wall) && game.objectUpdateListForTesting(wall)==1,
          "Owned light-wall completion enters frozen sleeping membership");
    check(!game.objectFrozenForTesting(finished),"Ordinary completed buildings do not inherit the light-wall freeze rule");
    check(game.damageObjectForTesting(wall,1) && !game.objectFrozenForTesting(wall),
          "Damage thaws naturally frozen light wall");
    const auto center=game.spawnObjectForTesting(1,109,1,12.f,60.f);
    game.setResourceForTesting(1,0,1000.f);
    check(game.queueUnitForTesting(center,83),"Queue production before explicit freeze");
    const float remaining=game.productionRemainingForTesting(center);
    game.setBuildingFrozenForTesting(center,true);game.update(1.f,{});
    check(game.productionRemainingForTesting(center)==remaining,
          "Frozen building skips production even though native production uses a separate phase");
    game.setBuildingFrozenForTesting(center,false);game.update(1.f,{});
    check(game.productionRemainingForTesting(center)<remaining,"Explicit thaw resumes production");
    const auto passenger=game.spawnObjectForTesting(1,83,1,12.f,64.f);
    game.setBuildingFrozenForTesting(center,true);
    check(game.garrisonForTesting(passenger,center) && game.objectTaskReferencesForTesting(center)==1 &&
          !game.objectFrozenForTesting(center),"Accepted garrison acquires and thaws its container");
    check(!game.garrisonForTesting(passenger,wall) && game.objectTaskReferencesForTesting(center)==1,
          "Rejected garrison preserves the existing container reference");
    game.stopUnitForTesting(passenger);
    check(game.objectTaskReferencesForTesting(center)==0,"Stop releases pending garrison reference");
    check(game.garrisonForTesting(passenger,center),"Restart accepted boarding");
    for(int tick=0;tick<200 && game.garrisonContainerForTesting(passenger)<0;++tick)game.update(.1f,{});
    check(game.garrisonContainerForTesting(passenger)==int32_t(center) &&
          game.objectTaskReferencesForTesting(center)==0,
          "Boarding completes garrison action; containment is not a task reference");
    const auto latePassenger=game.spawnObjectForTesting(1,83,1,12.f,68.f);
    check(game.garrisonForTesting(latePassenger,center),"Acquire container before destruction");
    game.damageObjectForTesting(center,100000);game.update(.1f,{});
    check(game.objectTaskReferencesForTesting(center)==0,"Destroyed container releases pending garrison action target");
    std::puts("PASS: repair, heal and build primary references, rejection, replacement, death and completion");
}
