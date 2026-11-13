// SPDX-License-Identifier: GPL-3.0-or-later
#include "../src/engine/rms_objects.h"
#include <cmath>

static void rmsSpawnChecks(swgb::Assets &assets) {
    swgb::Scenario scenario;
    scenario.map.width=scenario.map.height=6;
    scenario.map.tiles.resize(36);
    scenario.nextUnitId=1;
    swgb::Game game(assets);
    std::string error;
    check(game.initScenario(scenario,&error),error.c_str());
    uint32_t expectedState=1,state=1;
    std::vector<swgb::RmsTimberPlacement> expected;
    check(swgb::generateRmsTimberPlacements(6,6,std::vector<uint8_t>(36,0),assets.dat(),
          36,expectedState,expected,&error),error.c_str());
    check(game.populateRmsTimber(36,state,&error),error.c_str());
    check(state==expectedState && game.objectCountForTesting(0,348)==36,
          "Original timber plan must create all native objects without extra RNG");
    constexpr float pi=3.14159265358979323846f;
    for (size_t i=0;i<expected.size();++i) {
        const uint32_t id=static_cast<uint32_t>(i)+1;
        const auto p=game.objectPosition(id);
        check(p[0]==expected[i].x && p[1]==expected[i].y && game.objectHitPoints(id)==50 &&
              game.objectResourceAmount(id)==100,"Timber positions, HP and carbon preserved");
        const float facing=game.objectFacing(id);
        const float theta=std::atan2(std::cos(facing)+std::sin(facing),
                                    2*(std::cos(facing)-std::sin(facing)));
        const int facet=(static_cast<int>(std::lround((theta-pi*.5f)/(2*pi/12)))%12+12)%12;
        check(facet==expected[i].facet,"Original random facet must survive native facing projection");
    }
    const uint32_t saved=state;
    check(!game.populateRmsTimber(1,state,&error) && state==saved && game.activeObjectCount()==36,
          "Nonempty map rejection must not alter objects or RNG");
    check(game.initScenario(scenario,&error),error.c_str());
    check(game.setCornerElevationForTesting(0,0,1),"Elevated spawn fixture");
    check(!game.populateRmsTimber(1,state,&error) && game.activeObjectCount()==0 && state==saved,
          "Unsupported elevation must fail before spawning");
    for (auto &tile:scenario.map.tiles) tile.terrain=1;
    check(game.initScenario(scenario,&error),error.c_str());
    check(game.populateRmsTimber(36,state,&error) && game.activeObjectCount()==0,
          "Rejected terrain must create no native objects");
    std::puts("PASS: original RMS timber spawning, facets, resources and validation");
}
