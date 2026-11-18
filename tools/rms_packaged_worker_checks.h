// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Original executable packaged54103 worker-row full object-stage oracle: 2026-11-14, zero startup visibility masks.
static void packagedWorkerChecks(swgb::Assets &assets) {
    const std::string source=R"(<LAND_GENERATION> base_terrain 0
create_player_lands { terrain_type 0 base_size 3 number_of_tiles 60 }
<OBJECTS_GENERATION>
create_object 109 { number_of_objects 1 set_place_for_every_player min_distance_to_players 0 max_distance_to_players 0 }
create_object 83 { set_place_for_every_player min_distance_to_players 6 max_distance_to_players 6 }
)";
    swgb::RmsScript script;std::string error;
    check(script.load(0,[&](int32_t id,std::vector<uint8_t> &bytes) {
        if(id)return false;bytes.assign(source.begin(),source.end());return true;
    },{},&error),error.c_str());
    // Delay is raw original hold minus the fixed1s idle cycle.
    struct Worker { int owner,unit;float x,y;int facet,frame;float delay; };
    const struct { uint32_t size,players,seed,finalState,terrainHash,workerCount;int counts[8];int centers[8][2];Worker workers[24]; } cases[]={
        {32u,3u,1u,3841462793u,2282508853u,9u,{3,3,3},{{18,5},{23,20},{5,18}},{{1,293,12.500000000f,9.500000000f,6,4,0.000000000f},{1,293,12.500000000f,1.500000000f,2,7,0.000000000f},{1,293,12.500000000f,8.500000000f,0,4,0.000000000f},{2,83,17.500000000f,22.500000000f,2,6,0.000000000f},{2,83,26.500000000f,14.500000000f,0,7,0.000000000f},{2,293,17.500000000f,24.500000000f,0,4,0.000000000f},{3,83,3.500000000f,12.500000000f,5,7,2.408490181f},{3,293,9.500000000f,12.500000000f,7,4,0.000000000f},{3,293,0.500000000f,12.500000000f,5,5,0.000000000f}}},
        {32u,3u,12345u,2075660594u,479502229u,9u,{3,3,3},{{5,10},{20,11},{15,20}},{{1,83,9.500000000f,4.500000000f,6,7,1.078768253f},{1,293,2.500000000f,4.500000000f,6,0,0.000000000f},{1,293,6.500000000f,4.500000000f,0,7,0.000000000f},{2,83,21.500000000f,5.500000000f,6,3,0.000000000f},{2,83,22.500000000f,5.500000000f,4,1,0.000000000f},{2,83,20.500000000f,5.500000000f,3,4,0.000000000f},{3,293,13.500000000f,14.500000000f,6,7,0.000000000f},{3,293,12.500000000f,14.500000000f,5,7,0.578234196f},{3,293,9.500000000f,16.500000000f,7,7,2.374919891f}}},
        {32u,8u,1u,4266865838u,2282508853u,24u,{3,3,3,3,3,3,3,3},{{18,5},{20,17},{5,23},{15,20},{20,10},{5,9},{22,20},{5,16}},{{1,83,12.500000000f,4.500000000f,7,7,0.162633181f},{1,83,12.500000000f,0.500000000f,6,7,1.262062550f},{1,83,24.500000000f,0.500000000f,7,5,0.000000000f},{2,293,14.500000000f,12.500000000f,3,7,1.843165398f},{2,293,14.500000000f,11.500000000f,4,7,0.523514509f},{2,293,26.500000000f,11.500000000f,4,6,0.000000000f},{3,293,11.500000000f,26.500000000f,6,7,2.920438290f},{3,293,11.500000000f,27.500000000f,4,5,0.000000000f},{3,293,11.500000000f,28.500000000f,2,3,0.000000000f},{4,83,11.500000000f,14.500000000f,7,5,0.000000000f},{4,83,12.500000000f,14.500000000f,0,7,0.759514093f},{4,83,13.500000000f,14.500000000f,5,7,2.906338692f},{5,83,24.500000000f,4.500000000f,6,0,0.000000000f},{5,293,25.500000000f,4.500000000f,7,0,0.000000000f},{5,293,26.500000000f,4.500000000f,0,2,0.000000000f},{6,83,3.500000000f,3.500000000f,3,7,1.297982693f},{6,83,6.500000000f,3.500000000f,5,4,0.000000000f},{6,83,0.500000000f,3.500000000f,7,7,0.000000000f},{7,293,26.500000000f,14.500000000f,5,5,0.000000000f},{7,293,27.500000000f,14.500000000f,5,7,0.582598329f},{7,293,28.500000000f,14.500000000f,6,5,0.000000000f},{8,293,11.500000000f,10.500000000f,4,4,0.000000000f},{8,293,11.500000000f,11.500000000f,1,6,0.000000000f},{8,293,11.500000000f,12.500000000f,1,3,0.000000000f}}},
        {32u,8u,12345u,431136383u,1448477973u,24u,{3,3,3,3,3,3,3,3},{{15,5},{20,11},{8,5},{5,15},{20,18},{14,20},{21,20},{5,22}},{{1,293,21.500000000f,0.500000000f,2,5,0.000000000f},{1,293,21.500000000f,1.500000000f,3,0,0.000000000f},{1,293,21.500000000f,2.500000000f,3,7,1.078432560f},{2,83,24.500000000f,5.500000000f,0,7,0.000000000f},{2,293,22.500000000f,5.500000000f,0,7,2.588762999f},{2,293,14.500000000f,12.500000000f,1,7,0.000000000f},{3,293,2.500000000f,6.500000000f,2,4,0.000000000f},{3,293,2.500000000f,3.500000000f,2,6,0.000000000f},{3,293,2.500000000f,0.500000000f,6,0,0.000000000f},{4,83,0.500000000f,9.500000000f,7,7,0.000000000f},{4,293,1.500000000f,9.500000000f,5,5,0.000000000f},{4,293,2.500000000f,9.500000000f,5,0,0.000000000f},{5,293,14.500000000f,12.500000000f,7,7,1.759575129f},{5,293,26.500000000f,12.500000000f,2,7,1.969389915f},{5,293,14.500000000f,13.500000000f,0,4,0.000000000f},{6,83,14.500000000f,14.500000000f,5,0,0.000000000f},{6,293,11.500000000f,14.500000000f,1,0,0.000000000f},{6,293,12.500000000f,14.500000000f,5,0,0.000000000f},{7,83,26.500000000f,14.500000000f,3,5,0.000000000f},{7,83,27.500000000f,14.500000000f,2,4,0.000000000f},{7,293,27.500000000f,15.500000000f,6,5,0.000000000f},{8,293,11.500000000f,26.500000000f,5,2,0.000000000f},{8,293,11.500000000f,27.500000000f,3,7,0.000000000f},{8,293,0.500000000f,28.500000000f,7,5,0.000000000f}}},
    };
    swgb::Game game(assets);
    for(const auto &f:cases) {
        std::vector<uint16_t> civs;for(uint16_t i=1;i<=f.players;++i)civs.push_back(i);
        uint32_t state=f.seed;swgb::RmsMap map;
        check(swgb::generateRmsMap(script,{},f.size,f.size,assets.dat(),state,map,&error,f.players,civs),error.c_str());
        check(state==f.finalState && map.scenery.empty() && map.ownedObjects.size()==(f.players+f.workerCount),
              "Original many-owner CC/worker stage counts and final RNG");
        uint32_t hash=2166136261u;for(auto tile:map.terrain)hash=(hash^tile)*16777619u;
        check(hash==f.terrainHash,"Original many-owner stage terrain");
        for(uint32_t i=0;i<f.players;++i) {
            const auto &a=map.ownedObjects[i];
            check(a.owner==i+1 && a.civilization==i+1 && a.unitId==109 &&
                  a.x==f.centers[i][0] && a.y==f.centers[i][1] && a.pendingFoundation==(i?27:255),
                  "Original many-owner CC order and deferred foundation");
        }
        for(uint32_t i=0;i<f.workerCount;++i) {
            const auto &a=map.ownedObjects[f.players+i];const auto &e=f.workers[i];
            check(a.unitId==e.unit && a.owner==e.owner && a.civilization==e.owner &&
                  a.x==e.x && a.y==e.y && a.facet==e.facet && a.animationFrame==e.frame &&
                  std::abs(a.initialIdleDelay-e.delay)<.00001f,
                  "Original many-owner worker variant, position, pose and animation");
        }
        state=f.seed;
        check(game.initRms(script,{},f.size,state,&error,f.players,civs),error.c_str());
        check(state==f.finalState && game.activeObjectCount()==(f.players+f.workerCount),
              "Native many-owner RMS imported count and RNG");
        // Original IDs start0; native IDs start1, including partial worker placement. This is not allocator-call count.
        for(uint32_t i=0;i<f.players;++i)
            check(game.objectUnitId(i+1)==109 && game.objectPlayer(i+1)==i+1,
                  "Many-owner CC native creation IDs");
        for(uint32_t i=0;i<f.workerCount;++i) {
            const auto id=f.players+i+1;const int owner=f.workers[i].owner;
            check(game.objectUnitId(id)==f.workers[i].unit && game.objectPlayer(id)==owner &&
                  game.objectHitPoints(id)==(owner==8?55:40),
                  "Many-owner worker native IDs and post-map civilization health");
        }
        for(uint32_t owner=1;owner<=f.players;++owner)
            check(game.populationUsedForTesting(owner)==f.counts[owner-1] &&
                  game.populationCapacityForTesting(owner)==(owner==5?200.f:5.f) &&
                  game.resource(owner,98)==375+50*f.counts[owner-1] && game.resource(owner,164)==50*f.counts[owner-1] && game.resource(owner,165)==375,
                  "Many-owner population, post-map capacity and original cost accounting");
    }
    const auto prefix=source.substr(0,source.find("create_object 83"));
    for(const std::string attributes:{
        "min_distance_to_players 2 max_distance_to_players 6",
        "min_distance_to_players 6 max_distance_to_players 9",
        "min_distance_to_players 6 max_distance_to_players 6 group_placement_radius 18",
        "min_distance_to_players 6 max_distance_to_players 6 group_placement_radius 4",
        "min_distance_to_players 6 max_distance_to_players 6 group_placement_radius -1",
        "min_distance_to_players 6 max_distance_to_players 6 number_of_objects 2"}) {
        const auto unsupported=prefix+"create_object 83 { set_place_for_every_player "+attributes+" }";
        swgb::RmsScript rejected;
        check(rejected.load(0,[&](int32_t id,std::vector<uint8_t> &bytes) {
            if(id)return false;bytes.assign(unsupported.begin(),unsupported.end());return true;
        },{},&error),error.c_str());
        uint32_t state=1;swgb::RmsMap map;
        if(attributes.find("number_of_objects 2")!=std::string::npos) {
            // Explicit quantity is supported by the shared row controller;
            // keep this an integration check rather than a new original golden.
            check(swgb::generateRmsMap(rejected,{},32,32,assets.dat(),state,map,&error,3,{1,2,3}),error.c_str());
            std::array<int,4> workers{},centers{};
            for(const auto &object:map.ownedObjects) {
                check(object.owner>=1 && object.owner<=3,"Explicit worker quantity retains valid owners");
                if(object.unitId==109)++centers[size_t(object.owner)];
                else {
                    check(object.unitId==83 || object.unitId==293,"Explicit worker quantity creates worker prototypes");
                    ++workers[size_t(object.owner)];
                }
            }
            for(size_t owner=1;owner<=3;++owner)
                check(centers[owner]==1 && workers[owner]==2,"Explicit worker quantity gives each owner two workers and its starting center");
        } else {
            check(!swgb::generateRmsMap(rejected,{},32,32,assets.dat(),state,map,&error,3,{1,2,3}) &&
                  state==1 && map.ownedObjects.empty(),"Unsupported stock-worker distances/radii fail without changing RNG");
        }
    }
    const auto explicitDefaults=prefix+"create_object 83 { set_place_for_every_player number_of_objects 1 "
        "min_distance_to_players 6 max_distance_to_players 6 group_placement_radius 3 }";
    swgb::RmsScript explicitScript;
    check(explicitScript.load(0,[&](int32_t id,std::vector<uint8_t> &bytes) {
        if(id)return false;bytes.assign(explicitDefaults.begin(),explicitDefaults.end());return true;
    },{},&error),error.c_str());
    uint32_t explicitState=1;swgb::RmsMap explicitMap;
    check(swgb::generateRmsMap(explicitScript,{},32,32,assets.dat(),explicitState,explicitMap,&error,3,{1,2,3}) &&
          explicitState==cases[0].finalState && explicitMap.ownedObjects.size()==12,
          "Explicit stock quantity and group radius match parser defaults");
    std::puts("PASS: original packaged54103 workers and native imports");
}

static void packagedWorkerBoundaryChecks(swgb::Assets &assets) {
    struct Worker { int unit;float x,y;int facet,frame;float delay; };
    const struct { bool forest,cleanup;uint32_t seed,finalState,terrainHash,trees,sceneryHash;Worker workers[6]; } cases[]={
        {true,false,1u,2414692752u,2495521043u,37u,4201021415u,{{83,12.5f,8.5f,1,7,0.000000000f},{83,12.5f,6.5f,7,6,0.000000000f},{293,12.5f,2.5f,6,7,1.680684805f},{293,7.5f,20.5f,1,7,2.828791142f},{293,13.5f,14.5f,0,6,0.000000000f},{293,7.5f,15.5f,2,7,0.052522302f}}},
        {true,true,1u,2414692752u,2495521043u,37u,4201021415u,{{83,12.5f,8.5f,1,7,0.000000000f},{83,12.5f,6.5f,7,6,0.000000000f},{293,12.5f,2.5f,6,7,1.680684805f},{293,7.5f,20.5f,1,7,2.828791142f},{293,13.5f,14.5f,0,6,0.000000000f},{293,7.5f,15.5f,2,7,0.052522302f}}},
        {true,false,12345u,1067278215u,1986626585u,38u,2098472240u,{{83,7.5f,14.5f,2,6,0.000000000f},{293,0.5f,14.5f,3,7,0.320413828f},{293,1.5f,14.5f,3,7,0.000000000f},{293,14.5f,13.5f,4,0,0.000000000f},{293,20.5f,5.5f,1,7,2.782799721f},{293,23.5f,5.5f,6,1,0.000000000f}}},
        {true,true,12345u,1067278215u,1986626585u,38u,2098472240u,{{83,7.5f,14.5f,2,6,0.000000000f},{293,0.5f,14.5f,3,7,0.320413828f},{293,1.5f,14.5f,3,7,0.000000000f},{293,14.5f,13.5f,4,0,0.000000000f},{293,20.5f,5.5f,1,7,2.782799721f},{293,23.5f,5.5f,6,1,0.000000000f}}},
        {false,false,1u,1693511899u,2701429973u,0u,2166136261u,{}},
        {false,false,12345u,4151527826u,2701429973u,0u,2166136261u,{}},
    };
    swgb::Game game(assets);std::string error;
    for(const auto &f:cases) {
        const uint32_t players=f.forest?2:8,size=f.forest?32:8,workers=f.forest?6:0,deleted=f.cleanup?players:0;
        const std::string terrain=f.forest?R"(<LAND_GENERATION> base_terrain 41
create_player_lands { terrain_type 9 base_size 3 number_of_tiles 60 }
<TERRAIN_GENERATION>
create_terrain 10 { set_avoid_player_start_areas base_terrain 41 number_of_tiles 37 number_of_clumps 4 spacing_to_other_terrain_types 1 }
create_terrain 6 { base_terrain 9 number_of_tiles 9 number_of_clumps 2 clumping_factor 6 }
)":R"(<LAND_GENERATION> base_terrain 0
create_player_lands { terrain_type 0 base_size 2 number_of_tiles 60 }
)";
        const std::string source=terrain+"<OBJECTS_GENERATION> "+(f.cleanup?
            "create_object 573 { set_gaia_object_only set_place_for_every_player number_of_objects 1 min_distance_to_players 0 max_distance_to_players 0 group_placement_radius 18 } ":"")+
            "create_object 109 { set_place_for_every_player min_distance_to_players 0 max_distance_to_players 0 } "
            "create_object 83 { set_place_for_every_player min_distance_to_players 6 max_distance_to_players 6 }";
        swgb::RmsScript script;
        check(script.load(0,[&](int32_t id,std::vector<uint8_t> &bytes) {
            if(id)return false;bytes.assign(source.begin(),source.end());return true;
        },{},&error),error.c_str());
        std::vector<uint16_t> civs;for(uint16_t owner=1;owner<=players;++owner)civs.push_back(owner);
        uint32_t state=f.seed;swgb::RmsMap map;
        check(swgb::generateRmsMap(script,{},size,size,assets.dat(),state,map,&error,players,civs),error.c_str());
        check(state==f.finalState && map.scenery.size()==f.trees+deleted && map.ownedObjects.size()==players+workers,
              "Original stock workers with forest cleanup or fully excluded dense starts");
        uint32_t terrainHash=2166136261u,sceneryHash=terrainHash;
        for(auto tile:map.terrain)terrainHash=(terrainHash^tile)*16777619u;
        for(uint32_t i=0;i<f.trees;++i) {
            const auto &tree=map.scenery[i];check(tree.alive,"Stock worker placement retains forest trees");
            for(uint32_t value:{uint32_t(tree.unitId),uint32_t(tree.x*2),uint32_t(tree.y*2),uint32_t(tree.facet)})
                sceneryHash=(sceneryHash^value)*16777619u;
        }
        check(terrainHash==f.terrainHash && sceneryHash==f.sceneryHash,"Original stock-worker boundary terrain and scenery hashes");
        for(uint32_t i=0;i<deleted;++i)check(map.scenery[f.trees+i].unitId==573 && !map.scenery[f.trees+i].alive,
                                           "Stock-worker cleanup preserves dead573 records");
        for(uint32_t i=0;i<workers;++i) {
            const auto &a=map.ownedObjects[players+i];const auto &e=f.workers[i];
            check(a.owner==i/3+1 && a.unitId==e.unit && a.x==e.x && a.y==e.y && a.facet==e.facet &&
                  a.animationFrame==e.frame && std::abs(a.initialIdleDelay-e.delay)<.00001f,
                  "Original forest stock-worker positions, variants and animation");
        }
        state=f.seed;check(game.initRms(script,{},size,state,&error,players,civs),error.c_str());
        check(state==f.finalState && game.activeObjectCount()==f.trees+players+workers,"Stock boundary native import counts");
        for(uint32_t owner=1;owner<=players;++owner)
            check(game.populationUsedForTesting(owner)==(f.forest?3:0) && game.resource(owner,98)==(f.forest?525:375) &&
                  game.resource(owner,164)==(f.forest?150:0),"Only actually placed stock workers charge population/costs");
        for(uint32_t i=0;i<workers;++i)
            check(game.objectUnitId(f.trees+deleted+players+i+1)==f.workers[i].unit,
                  "Stock forest worker IDs preserve cleanup gaps");
    }
    std::puts("PASS: original stock6..6 workers with forest cleanup and dense excluded starts");
}
