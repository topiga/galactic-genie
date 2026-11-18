// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
static void movementProgressGameplayChecks(swgb::Assets &assets) {
    swgb::Scenario scenario;scenario.map.width=scenario.map.height=64;
    scenario.map.tiles.resize(64*64);scenario.nextUnitId=1;
    scenario.playerSlotCount=1;scenario.civilizations.fill(1);
    scenario.players[0].active=true;scenario.players[0].human=true;scenario.players[0].civilization=1;
    swgb::Game game(assets);std::string error;
    check(game.initScenario(scenario,&error),error.c_str());
    const auto worker=game.spawnObjectForTesting(1,83,1,20,20);
    const auto first=game.spawnObjectForTesting(0,102,0,40,20);
    const auto second=game.spawnObjectForTesting(0,102,0,30,32);
    check(worker && first && second,"Worker route-progress fixtures");
    game.update(0,{});
    check(game.issueGatherForTesting(worker,first),"Start a work approach without formation-order setup");
    // Gather orders attach their work approach during the owner update, not
    // while accepting the command. Zero-time passes isolate route setup from motion.
    for(int i=0;i<4 && game.movementProgressForTesting(worker)[0]==0;++i)game.update(0,{});
    const auto initial=game.movementProgressForTesting(worker);
    check(initial[0]>15 && initial[1]==0,"New work route initializes a nonzero distance baseline and clears stale stall time");
    const auto start=game.objectPosition(worker);
    game.updatePaused(2,{});
    check(game.objectPosition(worker)==start && game.movementProgressForTesting(worker)==initial,
          "Pause leaves movement progress clocks unchanged");
    for(int i=0;i<100;++i)game.update(.05f,{});
    const auto walking=game.movementProgressForTesting(worker);
    check(game.objectPosition(worker)!=start && walking[0]<initial[0]-1 && walking[1]<.25f,
          "A worker making real progress does not accumulate false stall time");
    check(game.issueGatherForTesting(worker,second),"Retarget the worker to a different resource");
    for(int i=0;i<4 && game.movementProgressForTesting(worker)[0]==walking[0];++i)game.update(0,{});
    const auto retargeted=game.movementProgressForTesting(worker);
    check(retargeted[0]>5 && retargeted[1]==0,"A replacement work route establishes its own progress baseline");
    for(int i=0;i<40;++i)game.update(.05f,{});
    const auto resumed=game.movementProgressForTesting(worker);
    check(resumed[0]<retargeted[0] && resumed[1]<.25f,"Retargeted work continues tracking actual progress");
}
