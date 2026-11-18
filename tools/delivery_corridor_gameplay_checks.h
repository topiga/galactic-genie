// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
[[gnu::noinline]] static void deliveryCorridorGameplayChecks(swgb::Assets &assets) {
    swgb::Scenario scenario;scenario.map.width=scenario.map.height=120;scenario.map.tiles.resize(14400);
    scenario.nextUnitId=1;scenario.playerSlotCount=2;scenario.civilizations.fill(1);
    for(int i=0;i<2;++i) {scenario.players[i].active=true;scenario.players[i].human=true;scenario.players[i].civilization=1;}
    swgb::Game game(assets);std::string error;check(game.initScenario(scenario,&error),error.c_str());
    game.setResourceForTesting(2,3,0);
    const auto center=game.spawnObjectForTesting(1,109,2,82,106);
    check(center && game.spawnObjectForTesting(1,562,2,82,109) && game.spawnObjectForTesting(1,562,2,85,109),
          "Delivery fixture has a Command Center and two depots making a single-file corridor");
    const auto idle=game.spawnObjectForTesting(1,83,2,83.618759f,110.022751f);
    check(idle && game.setWorkerCargoForTesting(idle,0,4.9f),"Idle loaded worker occupies the corridor entrance");
    std::vector<uint32_t> carriers;
    for(const auto &position:std::vector<std::array<float,2>>{
        {84.060547f,110.671234f},{84.025383f,110.217865f},{83.933456f,111.110970f},
        {83.522705f,110.915237f},{83.647972f,110.482437f}}) {
        const auto worker=game.spawnObjectForTesting(1,83,2,position[0],position[1]);
        check(worker && game.setWorkerCargoForTesting(worker,3,10) && game.issueDropOffForTesting(worker,center),
              "A loaded worker receives a real drop-off action");carriers.push_back(worker);
    }
    game.updatePaused(1,{});
    check(game.resource(2,3)==0,"Paused delivery cannot credit queued cargo");
    for(int tick=0;tick<2700 && game.resource(2,3)<50;++tick)game.update(1.f/30,{});
    check(game.resource(2,3)==50,"Five loaded workers reach free drop-off perimeter points within ninety seconds");
    for(uint32_t worker:carriers)check(game.objectActive(worker) && game.objectHitPoints(worker)>0 && game.objectUnitId(worker)==83,"Delivery preserves all workers");
    check(game.initScenario(scenario,&error),error.c_str());
    check(game.spawnObjectForTesting(1,109,2,82,106) &&
          game.spawnObjectForTesting(1,562,2,82,109) &&
          game.spawnObjectForTesting(1,562,2,85,109),
          "Fractional point fixture retains the static corridor without any crowd");
    const auto walker=game.spawnObjectForTesting(1,83,2,83.70832062f,109.01177216f);
    check(walker && game.issuePointMoveForTesting(walker,84.35f,106),
          "An ordinary move targets a fractional point beyond a building corner");
    for(int tick=0;tick<900;++tick)game.update(1.f/30,{});
    const auto reached=game.objectPosition(walker);
    check(std::hypot(reached[0]-84.35f,reached[1]-106)<0.25f,
          "An unobstructed final segment completes a partial point route without crowd recovery");
    std::puts("PASS: congested object-goal delivery preserves cargo and reaches alternate perimeter entrances");
}
