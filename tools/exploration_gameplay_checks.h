// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
static void explorationGameplayChecks(swgb::Assets &assets) {
    swgb::Scenario scenario;scenario.map.width=scenario.map.height=64;scenario.map.tiles.resize(64*64);
    scenario.nextUnitId=1;scenario.playerSlotCount=2;scenario.civilizations.fill(1);
    for(int i=0;i<2;++i){scenario.players[i].active=true;scenario.players[i].human=i==0;scenario.players[i].civilization=1;}
    swgb::Game game(assets);std::string error;check(game.initScenario(scenario,&error),error.c_str());
    game.setDiplomacyForTesting(1,2,3);game.setDiplomacyForTesting(2,1,3);
    const auto worker=game.spawnObjectForTesting(1,83,1,27.f,40.5f);
    check(game.issueExploreForTesting(worker,27.f,40.5f),"Accept already-arrived exploration child");
    check(game.explorationStateForTesting(worker)==11&&game.explorationChildStatusForTesting(worker)==1&&
          game.pointMovementStatusForTesting(worker)==-1,"Exploration parent owns a separate completed child, not an ordinary point head");
    game.updatePaused(1,{});
    check(game.explorationStateForTesting(worker)==11&&game.explorationChildStatusForTesting(worker)==1,
          "Pause does not dispatch the exploration child outcome");
    game.update(.05f,{});
    check(game.explorationStateForTesting(worker)==3&&game.explorationChildStatusForTesting(worker)==-1,
          "Child completion requests another destination without retiring its parent");
    check(game.issueExploreForTesting(worker,30.f,40.5f)&&
          game.explorationDestinationForTesting(worker)==std::array<float,2>{27.f,40.5f}&&
          game.explorationStateForTesting(worker)==3,"Nonforced object-level retarget preserves the existing parent unchanged");
    check(game.issueExploreForTesting(worker,30.f,40.5f,true)&&game.explorationChildStatusForTesting(worker)==0,
          "Forced exploration retarget creates a new movement child");
    for(int i=0;i<400&&game.explorationChildStatusForTesting(worker)==0;++i)game.update(.05f,{});
    check(game.explorationChildStatusForTesting(worker)==1&&game.explorationStateForTesting(worker)==11,
          "Real arrival completes only the exploration child");
    game.update(.05f,{});
    check(game.explorationStateForTesting(worker)==3&&game.explorationChildStatusForTesting(worker)==-1,
          "Arrived exploration parent survives its next update");
    check(game.issueExploreForTesting(worker,-1,-1,true)&&game.explorationStateForTesting(worker)==3,
          "Unknown exploration destination requests planning without fabricating a route");
    check(game.issueExploreForTesting(worker,35,40.5f,true),"Resume exploration with a known destination");
    game.stopUnitForTesting(worker);
    check(game.explorationStateForTesting(worker)==-1&&game.explorationChildStatusForTesting(worker)==-1,
          "Stop cancels exploration parent and child");
    check(game.issueExploreForTesting(worker,35,40.5f,true)&&game.issuePointMoveForTesting(worker,33,40.5f),
          "Replace exploration with ordinary movement");
    check(game.explorationStateForTesting(worker)==-1&&game.pointMovementStatusForTesting(worker)==0,
          "Ordinary movement replaces both exploration slots");
    check(!game.issueExploreForTesting(worker,35,40.5f),"Nonforced exploration cannot replace an unrelated active movement");
    game.stopUnitForTesting(worker);
    const auto enemy=game.spawnObjectForTesting(1,83,2,32,40.5f);
    check(game.issueExploreForTesting(worker,35,40.5f,true)&&game.issueAttackForTesting(worker,enemy),
          "Accepted attack replaces exploration");
    check(game.explorationStateForTesting(worker)==-1&&game.explorationChildStatusForTesting(worker)==-1,
          "Attack cancels exploration parent and child");
    game.stopUnitForTesting(worker);
    // An all-water map has no reachable land tile; a building alone permits an approach path.
    for(auto &tile:scenario.map.tiles)tile.terrain=1;
    check(game.initScenario(scenario,&error),error.c_str());
    const auto stranded=game.spawnObjectForTesting(1,83,1,27.f,40.5f);
    check(game.issueExploreForTesting(stranded,45,45,true),"Accept exploration with no reachable land destination");
    check(game.explorationChildStatusForTesting(stranded)==2&&game.explorationStateForTesting(stranded)==11,
          "Native path failure is retained without inventing an original child-controller status");
    game.update(.05f,{});
    check(game.explorationChildStatusForTesting(stranded)==2&&game.explorationStateForTesting(stranded)==11,
          "Unknown failed-child mapping does not falsely complete or rebuild the parent");
    check(game.issueExploreForTesting(stranded,-1,-1,true)&&game.explorationChildStatusForTesting(stranded)==-1&&
          game.explorationStateForTesting(stranded)==3,
          "Native planner can explicitly replace a retained failed child with a destination request");

    check(game.initSkirmish(0x5A17,&error),error.c_str());
    const int troop=game.objectUnitId(11);const auto home=game.objectPosition(game.commandCenterId(2));
    std::vector<uint32_t> wave;
    for(int i=0;i<6;++i)wave.push_back(game.spawnObjectForTesting(1,troop,2,home[0]+i*.3f,home[1]+6));
    game.updateSkirmishArmyForTesting(2);
    bool exploring=false;for(auto id:wave)exploring|=game.explorationStateForTesting(id)>=0;
    check(exploring,"Existing native scouting planner now installs exploration parents and movement children");
    std::puts("PASS: exploration parent/child gameplay, retarget, cancellation, native failure boundary and scouting integration");
}
