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
    const std::string drySource=R"(<LAND_GENERATION> base_terrain 41
        create_player_lands { terrain_type 9 base_size 3 number_of_tiles 60 }
        <TERRAIN_GENERATION>
        create_terrain 6 { base_terrain 41 number_of_tiles 37 number_of_clumps 4 spacing_to_other_terrain_types 1 }
        create_terrain 44 { base_terrain 9 number_of_tiles 9 number_of_clumps 2 clumping_factor 6 })";
    check(script.load(0,[&](int32_t id,std::vector<uint8_t> &bytes) {
        if (id!=0) return false;
        bytes.assign(drySource.begin(),drySource.end()); return true;
    },{},&error),error.c_str());
    const uint32_t drySeeds[]={1,12345}, dryHashes[]={2900154801u,784717495u}, dryStates[]={3532705190u,3759851087u};
    for (size_t fixture=0;fixture<2;++fixture) {
        swgb::RmsMap map; state=drySeeds[fixture];
        check(swgb::generateRmsMap(script,{},32,32,assets.dat(),state,map,&error),error.c_str());
        uint32_t hash=2166136261u;
        for (auto tile:map.terrain) hash=(hash^tile)*16777619u;
        check(hash==dryHashes[fixture] && state==dryStates[fixture] && map.timber.empty(),
              "Original multiterrain pipeline raster/RNG");
        state=drySeeds[fixture];
        check(game.initRms(script,{},32,state,&error) && state==dryStates[fixture] && game.activeObjectCount()==0,
              "Native dry-terrain initialization must not spawn inactive scenery slots");
        for (int y=0;y<32;++y) for (int x=0;x<32;++x)
            check(game.terrainAtForTesting(x,y)==map.terrain[y*32+x],"Dry terrain IDs and orientation preserved");
    }
    const std::string waterSource=R"(<LAND_GENERATION> base_terrain 32
        create_player_lands { terrain_type 35 base_size 3 number_of_tiles 60 }
        <TERRAIN_GENERATION>
        create_terrain 22 { base_terrain 32 number_of_tiles 37 number_of_clumps 4 spacing_to_other_terrain_types 1 }
        create_terrain 1 { base_terrain 35 number_of_tiles 9 number_of_clumps 2 clumping_factor 6 })";
    check(script.load(0,[&](int32_t id,std::vector<uint8_t> &bytes) {
        if (id!=0) return false;
        bytes.assign(waterSource.begin(),waterSource.end()); return true;
    },{},&error),error.c_str());
    const uint32_t waterHashes[]={4080120994u,2516495575u};
    for (size_t fixture=0;fixture<2;++fixture) {
        swgb::RmsMap map; state=drySeeds[fixture];
        check(swgb::generateRmsMap(script,{},32,32,assets.dat(),state,map,&error),error.c_str());
        uint32_t hash=2166136261u;
        for (auto tile:map.terrain) hash=(hash^tile)*16777619u;
        check(hash==waterHashes[fixture] && state==dryStates[fixture] && map.scenery.empty(),
              "Original water/snow/ice-shore pipeline raster/RNG");
        state=drySeeds[fixture];
        check(game.initRms(script,{},32,state,&error) && state==dryStates[fixture] && game.activeObjectCount()==0,
              "Native water and ice-shore alias initialization");
        for (int y=0;y<32;++y) for (int x=0;x<32;++x)
            check(game.terrainAtForTesting(x,y)==map.terrain[y*32+x],"Water and shore IDs preserved");
    }
    const std::string forestSource=R"(<LAND_GENERATION> base_terrain 41
        create_player_lands { terrain_type 9 base_size 3 number_of_tiles 60 }
        <TERRAIN_GENERATION>
        create_terrain 10 { base_terrain 41 number_of_tiles 37 number_of_clumps 4 spacing_to_other_terrain_types 1 set_avoid_player_start_areas }
        create_terrain 6 { base_terrain 9 number_of_tiles 9 number_of_clumps 2 clumping_factor 6 }
        <OBJECTS_GENERATION> create_object 348 { number_of_objects 3 })";
    check(script.load(0,[&](int32_t id,std::vector<uint8_t> &bytes) {
        if (id!=0) return false;
        bytes.assign(forestSource.begin(),forestSource.end()); return true;
    },{},&error),error.c_str());
    struct ForestFixture {
        uint32_t seed,hash,finalState;
        std::vector<std::array<uint32_t,2>> trees;
        swgb::RmsTimberPlacement timber[3];
        int terrainId=10, unitId=411, angles=12;
        float carbon=100;
        int hitPoints=50;
    };
    const ForestFixture forestFixtures[]={
        {1u,2561248739u,666383434u,{{160,6},{161,9},{162,10},{192,3},{193,6},{224,11},{225,10},{256,7},{257,2},{288,9},{481,4},{512,4},{513,0},{544,6},{545,4},{546,1},{571,3},{576,10},{577,2},{602,11},{603,1},{604,10},{608,10},{634,5},{635,2},{666,7},{667,6},{698,7},{889,5},{890,0},{920,10},{921,1},{922,3},{953,3},{954,8},{955,6},{986,11}},{{12.5f,5.5f,4},{14.5f,2.5f,10},{19.5f,10.5f,2}}},
        {12345u,3519253294u,2906001213u,{{71,4},{102,1},{103,3},{104,0},{105,1},{134,10},{135,7},{136,6},{137,5},{138,0},{169,10},{695,7},{726,10},{727,3},{728,6},{758,1},{759,7},{790,8},{791,5},{823,5},{860,5},{892,3},{893,9},{894,6},{923,0},{924,4},{925,11},{926,2},{927,2},{941,2},{942,2},{958,4},{973,9},{974,6},{975,8},{1006,5},{1007,5},{1008,1}},{{3.5f,15.5f,5},{15.5f,13.5f,3},{16.5f,7.5f,5}}},
        {1u,3403470729u,666383434u,{{160,6},{161,8},{162,9},{192,3},{193,5},{224,10},{225,9},{256,7},{257,2},{288,8},{481,4},{512,4},{513,0},{544,6},{545,4},{546,1},{571,3},{576,9},{577,2},{602,10},{603,1},{604,9},{608,9},{634,4},{635,2},{666,6},{667,6},{698,6},{889,4},{890,0},{920,9},{921,1},{922,2},{953,3},{954,7},{955,6},{986,10}},{{12.5f,5.5f,4},{14.5f,2.5f,10},{19.5f,10.5f,2}},20,349,11,100},
        {1u,497445300u,666383434u,{{160,6},{161,9},{162,10},{192,3},{193,6},{224,11},{225,10},{256,7},{257,2},{288,9},{481,4},{512,4},{513,0},{544,6},{545,4},{546,1},{571,3},{576,10},{577,2},{602,11},{603,1},{604,10},{608,10},{634,5},{635,2},{666,7},{667,6},{698,7},{889,5},{890,0},{920,10},{921,1},{922,3},{953,3},{954,8},{955,6},{986,11}},{{12.5f,5.5f,4},{14.5f,2.5f,10},{19.5f,10.5f,2}},21,413,12,125},
        {1u,2718513552u,3607380636u,{{160,8},{161,9},{162,0},{192,2},{193,2},{224,8},{225,0},{256,3},{257,4},{288,11},{481,1},{512,4},{513,7},{544,6},{545,11},{546,2},{571,7},{576,11},{577,4},{602,7},{603,1},{604,6},{608,3},{634,2},{635,5},{666,9},{667,11},{698,3},{889,5},{890,11},{920,9},{921,5},{922,2},{953,11},{954,7},{955,0},{986,8}},{{5.5f,28.5f,7},{5.5f,5.5f,8},{2.5f,6.5f,3}},49,1341,12,100,50},
        {1u,4144179467u,3607380636u,{{160,8},{161,9},{162,0},{192,2},{193,2},{224,8},{225,0},{256,3},{257,4},{288,11},{481,1},{512,4},{513,7},{544,6},{545,11},{546,2},{571,7},{576,11},{577,4},{602,7},{603,1},{604,6},{608,3},{634,2},{635,5},{666,9},{667,11},{698,3},{889,5},{890,11},{920,9},{921,5},{922,2},{953,11},{954,7},{955,0},{986,8}},{{5.5f,28.5f,7},{5.5f,5.5f,8},{2.5f,6.5f,3}},50,217,12,100,100},
    };
    for (const auto &fixture:forestFixtures) {
        auto source=forestSource;
        source.replace(source.find("create_terrain 10"),std::string("create_terrain 10").size(),
                       "create_terrain "+std::to_string(fixture.terrainId));
        check(script.load(0,[&](int32_t id,std::vector<uint8_t> &bytes) {
            if (id!=0) return false;
            bytes.assign(source.begin(),source.end()); return true;
        },{},&error),error.c_str());
        swgb::RmsMap map; state=fixture.seed;
        check(swgb::generateRmsMap(script,{},32,32,assets.dat(),state,map,&error),error.c_str());
        uint32_t hash=2166136261u;
        for (auto tile:map.terrain) hash=(hash^tile)*16777619u;
        check(hash==fixture.hash && state==fixture.finalState && map.scenery.size()==fixture.trees.size() && map.timber.size()==3,
              "Original forest pipeline raster/scenery/RNG mismatch");
        state=fixture.seed;
        check(game.initRms(script,{},32,state,&error) && state==fixture.finalState,error.c_str());
        check(game.objectCountForTesting(0,fixture.unitId)==fixture.trees.size() && game.objectCountForTesting(0,348)==3,
              "Native forest and explicit timber counts");
        for (size_t i=0;i<map.scenery.size();++i) {
            const auto &tree=map.scenery[i]; const auto &expectedTree=fixture.trees[i];
            check(tree.unitId==fixture.unitId && tree.x==float(expectedTree[0]%32)+.5f && tree.y==float(expectedTree[0]/32)+.5f &&
                  tree.facet==expectedTree[1],"Original ordered automatic scenery coordinates/facets");
            check(game.objectPosition(i+1)==std::array<float,2>{tree.x,tree.y} &&
                  game.objectHitPoints(i+1)==fixture.hitPoints && game.objectResourceAmount(i+1)==fixture.carbon,
                  "Native scenery position/HP/carbon preserved");
        }
        for (size_t i=0;i<3;++i) {
            const auto &tree=map.timber[i];const auto &expectedTree=fixture.timber[i];
            check(tree.x==expectedTree.x && tree.y==expectedTree.y && tree.facet==expectedTree.facet &&
                  game.objectPosition(map.scenery.size()+i+1)==std::array<float,2>{tree.x,tree.y},
                  "Original explicit timber after forest occupancy");
        }
        for (size_t i=0;i<map.scenery.size()+3;++i) {
            const float facing=game.objectFacing(i+1);
            const float theta=std::atan2(std::cos(facing)+std::sin(facing),2*(std::cos(facing)-std::sin(facing)));
            const int angles=i<map.scenery.size()?fixture.angles:12;
            const int facet=(static_cast<int>(std::lround((theta-pi*.5f)/(2*pi/angles)))%angles+angles)%angles;
            const int expectedFacet=i<map.scenery.size()?map.scenery[i].facet:map.timber[i-map.scenery.size()].facet;
            check(facet==expectedFacet,"Native forest and timber sprite facets preserved");
        }
    }
    const std::string forestLand="<LAND_GENERATION> base_terrain 41 create_player_lands { terrain_type 10 base_size 3 number_of_tiles 60 }";
    check(script.load(0,[&](int32_t id,std::vector<uint8_t> &bytes) {
        if (id!=0) return false;
        bytes.assign(forestLand.begin(),forestLand.end());return true;
    },{},&error),error.c_str());
    const uint32_t landHashes[]={1140975191u,201624367u}, landStates[]={39977193u,3868010736u}, landTrees[]={230,226};
    for (size_t fixture=0;fixture<2;++fixture) {
        swgb::RmsMap map;state=drySeeds[fixture];
        check(swgb::generateRmsMap(script,{},32,32,assets.dat(),state,map,&error),error.c_str());
        uint32_t hash=2166136261u;
        for (auto tile:map.terrain) hash=(hash^tile)*16777619u;
        check(hash==landHashes[fixture] && state==landStates[fixture] && map.scenery.size()==landTrees[fixture],
              "Original active land with inactive base preserves delayed scenery phase");
        state=drySeeds[fixture];
        check(game.initRms(script,{},32,state,&error) && state==landStates[fixture] &&
              game.objectCountForTesting(0,411)==landTrees[fixture],"Native active-land scenery import");
    }
    const std::string unsafeBase="<LAND_GENERATION> base_terrain 10 create_player_lands { terrain_type 9 base_size 3 number_of_tiles 60 }";
    check(script.load(0,[&](int32_t id,std::vector<uint8_t> &bytes) {
        if (id!=0) return false;
        bytes.assign(unsafeBase.begin(),unsafeBase.end());return true;
    },{},&error),error.c_str());
    swgb::RmsMap rejected;state=1;
    check(!swgb::generateRmsMap(script,{},32,32,assets.dat(),state,rejected,&error) && state==1 && rejected.terrain.empty(),
          "Active base requires unimplemented early scenery callbacks and must reject");
    std::puts("PASS: original RMS player lands, timber spawning, facets, resources and validation");
}
