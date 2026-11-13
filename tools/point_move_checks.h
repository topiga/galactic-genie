// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

static void pointMoveChecks(swgb::Assets &assets) {
    swgb::Scenario scenario;
    scenario.map.width=scenario.map.height=32;
    scenario.map.tiles.resize(32*32);
    scenario.nextUnitId=1;
    swgb::Game game(assets);std::string error;
    check(game.initScenario(scenario,&error),error.c_str());
    auto &workerType=const_cast<swgb::dat::DatFile &>(assets.dat()).civs[1].units[83];
    const float oldX=workerType.collisionSize[0],oldY=workerType.collisionSize[1];
    workerType.collisionSize[0]=.5f;workerType.collisionSize[1]=.25f;
    const auto worker=game.spawnObjectForTesting(1,83,1,10,10);
    check(worker!=0,"Independent point-move fixture");
    auto moving=[&](uint32_t id) {
        const auto list=game.movingObjects();
        return std::any_of(list.begin(),list.end(),[&](const swgb::MovingObjectInfo &o){return o.spawnId==id;});
    };
    game.groupMoveForTesting({worker},15,10);
    const bool previousPath=moving(worker);
    game.groupMoveForTesting({worker},10.5f,10.25f);
    const bool cornerComplete=!moving(worker);
    for(int i=0;i<10;++i) game.update(1.f/30,{});
    const bool stayed=game.objectPosition(worker)==std::array<float,2>{10,10};
    game.groupMoveForTesting({worker},10.5001f,10.25f);
    const bool outsideMoves=moving(worker);
    game.groupMoveForTesting({worker},10.5f,10.2501f);
    const bool shortAxisMoves=moving(worker);
    workerType.collisionSize[0]=oldX;workerType.collisionSize[1]=oldY;
    check(previousPath && cornerComplete && stayed,
          "Original point action completes at rectangular footprint corner and cancels old path");
    check(outsideMoves && shortAxisMoves,"Point just beyond either footprint axis still requests movement");
    const auto a=game.spawnObjectForTesting(1,307,1,20,20);
    const auto b=game.spawnObjectForTesting(1,307,1,22,20);
    check(a && b,"Formation point-move fixtures");
    game.groupMoveForTesting({a,b},25,20);
    check(moving(a) && moving(b),"Formation destinations retain their separate movement path");
    std::puts("PASS: independent initial point-action footprint completion, cancellation and formation isolation");
}
