// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

static void attackReferenceChecks(swgb::Assets &assets) {
    swgb::Game game(assets);std::string error;
    swgb::Scenario scenario;
    scenario.map.width=scenario.map.height=96;scenario.map.tiles.resize(96*96);
    scenario.nextUnitId=1;scenario.playerSlotCount=2;scenario.civilizations.fill(1);
    for(int i=0;i<2;++i) {scenario.players[i].active=true;scenario.players[i].human=i==0;scenario.players[i].civilization=1;}
    check(game.initScenario(scenario,&error),error.c_str());
    game.setDiplomacyForTesting(1,2,3);game.setDiplomacyForTesting(2,1,3);
    const auto attacker=game.spawnObjectForTesting(1,307,1,27.f,40.5f);
    const auto wall=game.spawnObjectForTesting(1,72,2,29.5f,40.5f);
    check(game.setBuildingFrozenForTesting(wall,true) && game.issueAttackForTesting(attacker,wall),
          "Attack a frozen enemy wall");
    check(game.objectTaskReferencesForTesting(wall)==1 && !game.objectFrozenForTesting(wall),
          "Attack parent acquires and thaws target before the first hit");
    check(game.issueAttackForTesting(attacker,wall) && game.objectTaskReferencesForTesting(wall)==1,
          "Repeated accepted attacks do not accumulate target references");
    check(!game.issueAttackForTesting(attacker,attacker) && game.objectTaskReferencesForTesting(wall)==1,
          "Rejected attack preserves current target ownership");
    check(game.stopUnitForTesting(attacker) && game.objectTaskReferencesForTesting(wall)==0,
          "Stop releases attack primary");
    check(game.issueAttackForTesting(attacker,wall) && game.damageObjectForTesting(attacker,100000) &&
          game.objectTaskReferencesForTesting(wall)==0,"Attacker death releases its live target");

    const auto automatic=game.spawnObjectForTesting(1,307,1,27.f,40.5f);
    for(int tick=0;tick<20 && !game.attackTargetForTesting(automatic);++tick)game.update(.1f,{});
    check(game.attackTargetForTesting(automatic)==wall && game.objectTaskReferencesForTesting(wall)==1,
          "Automatic acquisition uses the same owned attack target path");
    game.damageObjectForTesting(wall,100000);game.update(.1f,{});
    check(game.objectTaskReferencesForTesting(wall)==0,"Invalid destroyed target releases attack ownership");

    const auto second=game.spawnObjectForTesting(1,72,2,31.5f,40.5f);
    check(game.issueAttackForTesting(automatic,second),"Acquire target before movement replacement");
    game.groupMoveForTesting({automatic},24.f,40.f);
    check(game.objectTaskReferencesForTesting(second)==0,"Group move releases the replaced attack target");
    const auto scout=game.spawnObjectForTesting(1,83,1,35.f,43.f);
    game.setAttackModeForTesting(scout,3);
    const auto pursued=game.spawnObjectForTesting(1,72,2,37.f,40.5f);
    check(game.issueAttackForTesting(automatic,pursued),"Start visible object pursuit");
    game.update(.1f,{});
    check(game.objectTaskReferencesForTesting(pursued)==2,"Visible pursuit has separate parent and movement-child references");
    check(game.issueAttackForTesting(automatic,pursued,true) && game.objectTaskReferencesForTesting(pursued)==1,
          "Same-target automatic attack resets movement child without dropping primary ownership");
    game.update(.1f,{});
    check(game.objectTaskReferencesForTesting(pursued)==2,"Restarted automatic pursuit creates its new child");
    check(game.retryAttackForTesting(automatic) && game.objectTaskReferencesForTesting(pursued)==1,
          "Approach retry releases the discarded movement child before planning again");
    game.update(.1f,{});
    check(game.objectTaskReferencesForTesting(pursued)==2,"Retried pursuit acquires its replacement child");
    game.moveObjectForTesting(automatic,34.5f,40.5f);game.update(.1f,{});
    check(game.objectTaskReferencesForTesting(pursued)==1,"Reaching firing range releases movement child before attack work");
    game.moveObjectForTesting(automatic,27.f,40.5f);
    check(game.issueAttackForTesting(automatic,pursued),"Restart distant visible pursuit");
    game.update(.1f,{});
    check(game.objectTaskReferencesForTesting(pursued)==2,"New pursuit reacquires its own child target");
    game.stopUnitForTesting(automatic);
    check(game.objectTaskReferencesForTesting(pursued)==0,"Pursuit cancellation releases child and parent");
    const auto container=game.spawnObjectForTesting(1,109,1,23.f,40.f);
    check(game.issueAttackForTesting(automatic,second) && game.garrisonForTesting(automatic,container),
          "Replace attack with accepted garrison");
    check(game.objectTaskReferencesForTesting(second)==0 && game.objectTaskReferencesForTesting(container)==1,
          "Accepted replacement transfers ownership from attack to garrison action");
    std::puts("PASS: attack primary acquisition, frozen target thaw, replacement, auto-acquire and destruction");
}
