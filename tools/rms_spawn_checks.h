// SPDX-License-Identifier: GPL-3.0-or-later
#include "../src/engine/rms_objects.h"
#include "../src/engine/rms_map.h"
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
    const std::string source=R"(<LAND_GENERATION> base_terrain 0
        create_land { terrain_type 3 land_position 30 30 base_size 0 number_of_tiles 10 border_fuzziness 0 zone 0 }
        create_land { terrain_type 3 land_position 60 60 base_size 0 number_of_tiles 10 border_fuzziness 0 zone 1 }
        <OBJECTS_GENERATION> create_object 348 { number_of_objects 3 })";
    swgb::RmsScript script;
    check(script.load(0,[&](int32_t id,std::vector<uint8_t> &bytes) {
        if (id!=0) return false;
        bytes.assign(source.begin(),source.end()); return true;
    },{},&error),error.c_str());
    struct Fixture { uint32_t seed, hash, finalState; swgb::RmsTimberPlacement objects[3]; bool terrainRows=false, protection=false; };
    const Fixture fixtures[]={
        {1,1509571036u,2283988067u,{{3.5f,1.5f,2},{2.5f,4.5f,7},{.5f,1.5f,5}}},
        {12345,2339149697u,4028089366u,{{.5f,3.5f,5},{4.5f,1.5f,9},{3.5f,.5f,9}}}
    };
    for (const auto &fixture:fixtures) {
        swgb::RmsMap map;
        state=fixture.seed;
        check(swgb::generateRmsMap(script,{},7,7,assets.dat(),state,map,&error),error.c_str());
        uint32_t hash=2166136261u;
        for (auto tile:map.terrain) hash=(hash^tile)*16777619u;
        check(hash==fixture.hash && state==fixture.finalState && map.timber.size()==3,
              "Original composed RMS stages terrain/RNG mismatch");
        state=fixture.seed;
        check(game.initRms(script,{},7,state,&error),error.c_str());
        check(state==fixture.finalState && game.activeObjectCount()==3,"Native RMS pipeline object count/RNG");
        for (int y=0;y<7;++y) for (int x=0;x<7;++x)
            check(game.terrainAtForTesting(x,y)==map.terrain[y*7+x],"RMS terrain orientation preserved");
        for (uint32_t i=0;i<3;++i) {
            const auto p=game.objectPosition(i+1);
            check(p[0]==fixture.objects[i].x && p[1]==fixture.objects[i].y &&
                  map.timber[i].facet==fixture.objects[i].facet,"Original RMS object fixture");
            const float facing=game.objectFacing(i+1);
            const float theta=std::atan2(std::cos(facing)+std::sin(facing),
                                        2*(std::cos(facing)-std::sin(facing)));
            const int facet=(static_cast<int>(std::lround((theta-pi*.5f)/(2*pi/12)))%12+12)%12;
            check(facet==fixture.objects[i].facet,"RMS scenario bridge preserves facets");
        }
    }
    const std::string playerSource=R"(<LAND_GENERATION> base_terrain 0
        create_player_lands { terrain_type 3 base_size 3 number_of_tiles 60 clumping_factor 8 border_fuzziness 20 }
        <OBJECTS_GENERATION> create_object 348 { number_of_objects 3 })";
    const Fixture playerFixtures[]={
        {1,3728055951u,1403560318u,{{1.5f,5.5f,7},{3.5f,18.5f,3},{1.5f,29.5f,3}}},
        {12345,1771775183u,3972322661u,{{18.5f,15.5f,1},{26.5f,22.5f,10},{4.5f,15.5f,2}}},
        {1,1549600828u,3883528439u,{{18.5f,6.5f,4},{12.5f,28.5f,4},{1.5f,19.5f,7}},true},
        {12345,693012607u,2130667826u,{{6.5f,10.5f,7},{25.5f,28.5f,4},{3.5f,22.5f,8}},true},
        {1,3452956004u,942928812u,{{2.5f,18.5f,1},{18.5f,9.5f,3},{9.5f,14.5f,9}},true,true},
        {12345,353970588u,3736035543u,{{.5f,19.5f,5},{11.5f,18.5f,9},{7.5f,24.5f,5}},true,true}
    };
    for (const auto &fixture:playerFixtures) {
        auto source=playerSource;
        if (fixture.terrainRows) source.insert(source.find("<OBJECTS_GENERATION>"),R"(
            <TERRAIN_GENERATION>
            create_terrain 3 { base_terrain 0 number_of_tiles 37 number_of_clumps 4 spacing_to_other_terrain_types 1 }
            create_terrain 0 { base_terrain 3 number_of_tiles 9 number_of_clumps 2 clumping_factor 6 }
        )");
        if (fixture.protection) source.insert(source.find("spacing_to_other_terrain_types"),"set_avoid_player_start_areas ");
        check(script.load(0,[&](int32_t id,std::vector<uint8_t> &bytes) {
            if (id!=0) return false;
            bytes.assign(source.begin(),source.end()); return true;
        },{},&error),error.c_str());
        swgb::RmsMap map;
        state=fixture.seed;
        check(swgb::generateRmsMap(script,{},32,32,assets.dat(),state,map,&error),error.c_str());
        uint32_t hash=2166136261u;
        for (auto tile:map.terrain) hash=(hash^tile)*16777619u;
        check(hash==fixture.hash && state==fixture.finalState && map.timber.size()==3 && map.playerStarts.size()==2,
              "Original composed player-land pipeline terrain/RNG mismatch");
        state=fixture.seed;
        check(game.initRms(script,{},32,state,&error) && state==fixture.finalState,error.c_str());
        for (uint32_t i=0;i<3;++i) {
            const auto p=game.objectPosition(i+1);
            check(p[0]==fixture.objects[i].x && p[1]==fixture.objects[i].y &&
                  map.timber[i].facet==fixture.objects[i].facet,"Original player-land timber fixture");
        }
    }
    std::puts("PASS: original RMS player lands, timber spawning, facets, resources and validation");
}
