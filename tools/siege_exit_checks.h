// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

static void siegeExitChecks(swgb::Assets &assets) {
    swgb::Scenario scenario;
    scenario.map.width=scenario.map.height=64;
    scenario.map.tiles.resize(64*64);
    scenario.playerSlotCount=2;
    scenario.nextUnitId=1;
    scenario.civilizations.fill(1);
    for(int p=0;p<2;++p) {
        scenario.players[p].active=scenario.players[p].human=true;
        scenario.players[p].civilization=1;
    }
    const auto add=[&](int id,int type,float x,float y) {
        swgb::ScenarioUnit unit;
        unit.spawnId=id;unit.unitId=type;unit.player=2;
        unit.x=64-y;unit.y=x;
        scenario.units.push_back(unit);
    };
    add(1942,109,29.0000000f,16.0000000f);
    add(2089,83,31.8065395f,20.8027325f);
    add(2093,83,31.6234474f,22.7632198f);
    add(2146,12,32.5000000f,16.5000000f);
    add(2166,87,33.5000000f,20.5000000f);
    add(2206,101,29.0000000f,20.0000000f);
    add(2369,691,31.4949856f,20.0976391f);
    add(2388,12,33.5000000f,16.5000000f);
    add(2391,12,35.5000000f,22.5000000f);
    add(2393,12,29.5000000f,22.5000000f);
    add(2402,307,30.6368408f,22.5441189f);
    add(2406,307,32.0000000f,22.5000000f);
    add(2408,307,32.5000000f,22.5000000f);
    add(2411,307,33.0000000f,22.5000000f);
    add(2414,307,33.5000000f,22.5000000f);
    add(2417,307,31.2147369f,21.9760246f);
    add(2421,469,28.2000008f,22.6000004f);
    add(2423,307,34.0000000f,22.5000000f);
    add(2426,307,34.5000000f,22.5000000f);
    add(2427,50,29.5000000f,24.5000000f);
    add(2436,469,31.1127682f,22.6994686f);
    auto game=std::make_unique<swgb::Game>(assets);
    std::string error;
    check(game->initScenario(scenario,&error),error.c_str());
    for(const auto &unit:scenario.units)game->setAttackModeForTesting(unit.spawnId,3);
    game->update(0,{});
    const auto route=[&](int id,const std::vector<std::array<float,2>> &path) {
        check(game->restoreMovementRouteForTesting(id,path.back()[0],path.back()[1],path),
              "Restore captured approach through the factory exit");
    };
    route(2089,{{31.35f,16.776245f}});
    route(2093,{{31.5f,22.5f},{31.247488f,18.247488f}});
    route(2402,{{30.974226f,23.100246f}});
    route(2436,{{30.520971f,22.918459f}});
    route(2369,{{31.5f,22.5f},{34.5f,28.5f},{55.5f,55.5f}});
    for(int frame=0;frame<30*90;++frame) {
        game->update(1.f/30,{});
        const auto point=game->objectPosition(2369);
        check(game->positionPassableForTesting(2369,point[0],point[1]),
              "Artillery clears the buildings on each movement update");
    }
    const auto pos=game->objectPosition(2369);
    check(pos[1]>30,"Artillery exits beyond the factory and Troop Center");
}
