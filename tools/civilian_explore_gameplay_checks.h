// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
static void civilianExploreGameplayChecks(swgb::Assets &assets) {
    swgb::Scenario scenario;scenario.map.width=scenario.map.height=64;
    scenario.map.tiles.resize(64*64);scenario.nextUnitId=1;
    scenario.playerSlotCount=1;scenario.civilizations.fill(1);
    scenario.players[0].active=true;scenario.players[0].human=true;scenario.players[0].civilization=1;
    swgb::Game game(assets);std::string error;
    check(game.initScenario(scenario,&error),error.c_str());
    const auto worker=game.spawnObjectForTesting(1,83,1,32,32);
    check(worker!=0,"Local exploration worker fixture");
    game.update(0,{}); // Establish its real sight before choosing unseen tiles.
    // No Command Center or remembered targets exist:5ad950 must fail and
    // the civilian dispatcher must still queue705 with unknown coordinates.
    check(game.queueCivilianExploreForTesting(worker),"Missing tactical waypoint still queues civilian exploration");
    const auto assignmentCount=[&](const auto &ids) {
        return std::count(ids.begin(),ids.end(),int32_t(worker));
    };
    check(assignmentCount(game.economyForTesting(1).civilianExplorers)==1 &&
          assignmentCount(game.economyForTesting(1).pendingCivilianAssignments)==1,
          "Negative waypoint fallback registers exactly one explorer and pending assignment");
    check(game.stagedTacticalCountForTesting()==1,"Negative civilian waypoint remains a queued tactical command");
    game.updatePaused(.1f,{});
    check(game.explorationStateForTesting(worker)==-1,"Paused negative705 does not start an action");
    game.update(0,{});game.update(0,{});
    check(game.explorationStateForTesting(worker)==11 && game.explorationChildStatusForTesting(worker)>=0,
          "Negative705 activates original actor-local destination search and a movement child");
    const auto first=game.explorationDestinationForTesting(worker);
    const auto start=game.objectPosition(worker);
    check(first[0]>=0 && first[1]>=0 && first!=start,"Actor-local search chooses an unseen concrete waypoint");
    bool moved=false,completed=false;
    for(int i=0;i<1600 && !completed;++i) {
        game.update(.05f,{});
        moved|=game.objectPosition(worker)!=start;
        completed=game.explorationStateForTesting(worker)==3;
    }
    check(moved,"Worker actually walks after negative705 activation");
    check(completed && game.explorationDestinationForTesting(worker)==first,
          "Completed local waypoint retains concrete705 coordinates for forwarded AI continuation");
    game.stopUnitForTesting(worker);
    check(game.explorationStateForTesting(worker)==-1,"Stop retires local exploration normally");
    check(game.queueCivilianExploreForTesting(worker),"Redispatch an existing civilian explorer");
    check(assignmentCount(game.economyForTesting(1).civilianExplorers)==1 &&
          assignmentCount(game.economyForTesting(1).pendingCivilianAssignments)==1,
          "Redispatch does not duplicate original explorer or pending registry entries");
}
