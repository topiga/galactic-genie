// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "../src/core/genie_movement_action.h"

static void runMovementActionChecks() {
    using swgb::GenieMovementStatus;
    swgb::GenieMovementAction head;
    // Original 409160/408220 -> 409800; retained movement-head-lifecycle v1.
    check(!head.present && !head.retire(), "Empty movement controller has no head to retire");
    head.begin(50.f, 60.f);
    check(head.present && head.status == GenieMovementStatus::Active &&
          head.destinationX == 50.f && head.destinationY == 60.f && !head.retire(),
          "Accepted point movement stays attached while active");
    head.complete();
    check(head.present && head.status == GenieMovementStatus::Completed &&
          head.destinationX == 50.f && head.destinationY == 60.f,
          "Already-completed movement retains its destination until controller retirement");
    head.begin(50.f, 60.f);
    check(head.present && head.status == GenieMovementStatus::Active && !head.retire(),
          "Same-destination command replaces the completed head with a fresh active head");
    head.fail();
    check(head.present && head.status == GenieMovementStatus::Failed && !head.retire(),
          "Failure does not implicitly destroy the movement head");
    head.begin(70.f, 80.f);
    check(head.status == GenieMovementStatus::Active &&
          head.destinationX == 70.f && head.destinationY == 80.f,
          "New accepted command replaces failed movement and its destination");
    head.complete();
    check(head.retire() && !head.present && head.destinationX == -1.f &&
          head.destinationY == -1.f && !head.retire(),
          "Controller retirement removes a completed head exactly once");
    head.begin(0.f, 0.f);
    check(head.present && head.destinationX == 0.f && head.destinationY == 0.f,
          "Map-edge destinations do not encode an empty movement controller");
    head.cancel();
    head.complete();
    head.fail();
    check(!head.present && !head.retire(),
          "Stop clears the head and later outcomes cannot resurrect it");
}

static void movementActionGameplayChecks(swgb::Assets &assets) {
    swgb::Scenario scenario;scenario.map.width=scenario.map.height=64;scenario.map.tiles.resize(64*64);
    scenario.nextUnitId=1;scenario.playerSlotCount=2;scenario.civilizations.fill(1);
    for(int i=0;i<2;++i){scenario.players[i].active=true;scenario.players[i].human=i==0;scenario.players[i].civilization=1;}
    swgb::Game game(assets);std::string error;check(game.initScenario(scenario,&error),error.c_str());
    const auto worker=game.spawnObjectForTesting(1,83,1,27.f,40.5f);
    check(game.issuePointMoveForTesting(worker,27.f,40.5f),"Accept already-arrived ordinary point move");
    check(game.pointMovementStatusForTesting(worker)==1 &&
          game.pointMovementDestinationForTesting(worker)==std::array<float,2>{27.f,40.5f},
          "Zero-distance command keeps a completed head and original destination");
    game.updatePaused(1,{});
    check(game.pointMovementStatusForTesting(worker)==1,"Pause does not retire completed movement");
    game.update(.05f,{});
    check(game.pointMovementStatusForTesting(worker)==-1,"Admitted owner update retires completed head");
    check(game.issuePointMoveForTesting(worker,30.f,40.5f),"Accept ordinary worker movement");
    check(game.pointMovementStatusForTesting(worker)==0,"Accepted movement installs active head");
    for(int i=0;i<400 && game.pointMovementStatusForTesting(worker)==0;++i)game.update(.05f,{});
    check(game.pointMovementStatusForTesting(worker)==1,"Arrival completes without immediately removing its head");
    const auto arrival=game.objectPosition(worker);
    game.update(.05f,{});
    check(game.pointMovementStatusForTesting(worker)==-1 && game.objectPosition(worker)==arrival,
          "Next owner update retires the arrived movement without moving again");
    check(game.issuePointMoveForTesting(worker,35.f,40.5f),"Replace retired movement");
    game.stopUnitForTesting(worker);
    check(game.pointMovementStatusForTesting(worker)==-1,"Stop cancels active head immediately");
    std::puts("PASS: native point movement attachment, pause, completion, retirement and Stop");
}
