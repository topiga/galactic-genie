// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
static void manyForestWorkerChecks(swgb::Assets &assets) {
    const std::string prefix=R"(<LAND_GENERATION>
base_terrain 41
create_player_lands { terrain_type 9 base_size 3 number_of_tiles 60 }
<TERRAIN_GENERATION>
create_terrain 10 { set_avoid_player_start_areas base_terrain 41 number_of_tiles 37 number_of_clumps 4 spacing_to_other_terrain_types 1 }
create_terrain 6 { base_terrain 9 number_of_tiles 9 number_of_clumps 2 clumping_factor 6 }
<OBJECTS_GENERATION>
)";
    const std::string cleanup=R"(create_object 573 { set_gaia_object_only set_place_for_every_player number_of_objects 1 min_distance_to_players 0 max_distance_to_players 0 group_placement_radius 18 }
)";
    const std::string cc=R"(create_object 109 { set_place_for_every_player number_of_objects 1 min_distance_to_players 0 max_distance_to_players 0 })";
    const std::string worker=R"( create_object 83 { number_of_objects 1 set_place_for_every_player min_distance_to_players 1 max_distance_to_players 3 })";
    struct Worker { int unit;float x,y;int facet,frame;float delay; };
    const struct { uint32_t players,seed,finalState,trees,terrainHash,sceneryHash;int centers[8][2];Worker workers[24]; } cases[]={
        {1u,1u,4000822828u,38u,3232917026u,1264919057u,{{18,5}},{{83,15.500000000f,5.500000000f,1,7,2.385998011f},{83,15.500000000f,4.500000000f,1,0,0.000000000f},{83,15.500000000f,2.500000000f,3,7,1.252998352f}}},
        {1u,12345u,2959792756u,37u,297464486u,3207088u,{{20,11}},{{83,17.500000000f,8.500000000f,0,7,0.000000000f},{293,18.500000000f,8.500000000f,3,0,0.000000000f},{293,19.500000000f,8.500000000f,0,7,2.377941132f}}},
        {3u,1u,3758076468u,38u,419769886u,3889132145u,{{18,5},{23,20},{5,18}},{{293,15.500000000f,4.500000000f,3,4,0.000000000f},{293,15.500000000f,3.500000000f,4,7,0.410382390f},{293,15.500000000f,6.500000000f,1,7,2.585406065f},{293,20.500000000f,21.500000000f,0,0,0.000000000f},{293,20.500000000f,17.500000000f,0,3,0.000000000f},{293,21.500000000f,17.500000000f,4,0,0.000000000f},{83,2.500000000f,18.500000000f,2,7,0.000000000f},{293,4.500000000f,15.500000000f,0,4,0.000000000f},{293,5.500000000f,15.500000000f,5,4,0.000000000f}}},
        {3u,12345u,665937878u,38u,2885109120u,3116409675u,{{5,10},{20,11},{15,20}},{{293,2.500000000f,10.500000000f,5,3,0.000000000f},{293,2.500000000f,7.500000000f,2,0,0.000000000f},{293,6.500000000f,7.500000000f,0,7,2.300393581f},{293,21.500000000f,8.500000000f,1,7,0.190160871f},{293,17.500000000f,10.500000000f,1,4,0.000000000f},{293,17.500000000f,9.500000000f,4,4,0.000000000f},{83,15.500000000f,17.500000000f,0,2,0.000000000f},{293,13.500000000f,17.500000000f,1,7,0.000000000f},{293,12.500000000f,17.500000000f,4,7,0.000000000f}}},
        {8u,1u,504322075u,14u,930223842u,70354911u,{{18,5},{20,17},{5,23},{15,20},{20,10},{5,9},{22,20},{5,16}},{{293,17.500000000f,2.500000000f,6,7,1.120395422f},{293,18.500000000f,2.500000000f,5,7,2.214453459f},{293,15.500000000f,6.500000000f,5,7,0.898159742f},{83,18.500000000f,14.500000000f,3,0,0.000000000f},{293,17.500000000f,14.500000000f,3,3,0.000000000f},{293,19.500000000f,14.500000000f,2,7,0.522507429f},{293,5.500000000f,20.500000000f,0,7,2.384655237f},{293,6.500000000f,20.500000000f,2,3,0.000000000f},{293,2.500000000f,22.500000000f,0,3,0.000000000f},{83,14.500000000f,17.500000000f,4,6,0.000000000f},{293,12.500000000f,20.500000000f,2,7,0.000000000f},{293,12.500000000f,17.500000000f,4,4,0.000000000f},{293,17.500000000f,7.500000000f,5,2,0.000000000f},{293,17.500000000f,9.500000000f,3,4,0.000000000f},{293,18.500000000f,7.500000000f,0,7,0.000000000f},{83,5.500000000f,6.500000000f,7,7,1.441999674f},{293,2.500000000f,6.500000000f,1,6,0.000000000f},{293,3.500000000f,6.500000000f,0,1,0.000000000f},{83,22.500000000f,17.500000000f,5,7,0.886074424f},{293,23.500000000f,17.500000000f,7,2,0.000000000f},{293,24.500000000f,17.500000000f,2,6,0.000000000f},{293,2.500000000f,15.500000000f,0,6,0.000000000f},{293,4.500000000f,13.500000000f,4,4,0.000000000f},{293,2.500000000f,17.500000000f,3,7,1.921720028f}}},
        {8u,12345u,1765183352u,37u,1196344967u,77405676u,{{15,5},{20,11},{8,5},{5,15},{20,18},{14,20},{21,20},{5,22}},{{83,14.500000000f,2.500000000f,2,7,0.121005893f},{83,12.500000000f,3.500000000f,0,7,1.515182972f},{83,12.500000000f,5.500000000f,7,5,0.000000000f},{293,18.500000000f,8.500000000f,1,1,0.000000000f},{293,17.500000000f,10.500000000f,6,6,0.000000000f},{293,17.500000000f,8.500000000f,2,6,0.000000000f},{83,9.500000000f,2.500000000f,5,1,0.000000000f},{293,5.500000000f,2.500000000f,2,7,0.000000000f},{293,6.500000000f,2.500000000f,3,5,0.000000000f},{293,2.500000000f,14.500000000f,7,7,0.000000000f},{293,2.500000000f,12.500000000f,2,1,0.000000000f},{293,3.500000000f,12.500000000f,7,1,0.000000000f},{83,17.500000000f,15.500000000f,7,3,0.000000000f},{293,18.500000000f,15.500000000f,1,5,0.000000000f},{293,19.500000000f,15.500000000f,1,7,0.000000000f},{293,11.500000000f,20.500000000f,0,5,0.000000000f},{293,11.500000000f,17.500000000f,0,2,0.000000000f},{293,12.500000000f,17.500000000f,6,7,2.928159475f},{83,22.500000000f,17.500000000f,1,6,0.000000000f},{293,23.500000000f,17.500000000f,0,1,0.000000000f},{293,24.500000000f,17.500000000f,5,5,0.000000000f},{293,2.500000000f,19.500000000f,0,7,0.000000000f},{293,3.500000000f,19.500000000f,3,1,0.000000000f},{293,4.500000000f,19.500000000f,0,7,0.502029419f}}},
    };
    swgb::Game game(assets);std::string error;
    for(bool clearScenery:{false,true}) {
        const auto source=prefix+(clearScenery?cleanup:std::string())+cc+worker;
        swgb::RmsScript script;
        check(script.load(0,[&](int32_t id,std::vector<uint8_t> &bytes) {
            if(id)return false;bytes.assign(source.begin(),source.end());return true;
        },{},&error),error.c_str());
        for(const auto &f:cases) {
            std::vector<uint16_t> civs;for(uint16_t i=1;i<=f.players;++i)civs.push_back(i);
            const uint32_t deleted=clearScenery?f.players:0;
            uint32_t state=f.seed;swgb::RmsMap map;
            check(swgb::generateRmsMap(script,{},32,32,assets.dat(),state,map,&error,f.players,civs),error.c_str());
            check(state==f.finalState && map.scenery.size()==f.trees+deleted && map.ownedObjects.size()==4*f.players,
                  "Original forest, cleanup, CC and worker stage counts/RNG");
            uint32_t terrainHash=2166136261u,sceneryHash=terrainHash;
            for(auto tile:map.terrain)terrainHash=(terrainHash^tile)*16777619u;
            for(uint32_t i=0;i<f.trees;++i) {
                const auto &tree=map.scenery[i];check(tree.alive,"Many-owner workers retain original forest");
                for(uint32_t v:{uint32_t(tree.unitId),uint32_t(tree.x*2),uint32_t(tree.y*2),uint32_t(tree.facet)})
                    sceneryHash=(sceneryHash^v)*16777619u;
            }
            check(terrainHash==f.terrainHash && sceneryHash==f.sceneryHash,"Original forest worker terrain and scattering");
            for(uint32_t i=0;i<f.players;++i) {
                const auto &a=map.ownedObjects[i];
                check(a.owner==i+1 && a.civilization==i+1 && a.unitId==109 &&
                      a.x==f.centers[i][0] && a.y==f.centers[i][1] && a.pendingFoundation==(i?27:255),
                      "Original forest worker owner starts and pending foundation");
                if(clearScenery) {
                    const auto &dead=map.scenery[f.trees+i];
                    check(dead.unitId==573 && !dead.alive && dead.x==a.x && dead.y==a.y,"CC clears each original573");
                }
            }
            for(uint32_t i=0;i<3*f.players;++i) {
                const auto &a=map.ownedObjects[f.players+i];const auto &e=f.workers[i];
                // Fixture delay is original raw animation hold minus the fixed1s idle cycle.
                check(a.unitId==e.unit && a.owner==i/3+1 && a.civilization==i/3+1 &&
                      a.x==e.x && a.y==e.y && a.facet==e.facet && a.animationFrame==e.frame &&
                      std::abs(a.initialIdleDelay-e.delay)<.00001f,
                      "Original many-owner forest worker variant, position, pose and idle delay");
            }
            state=f.seed;
            check(game.initRms(script,{},32,state,&error,f.players,civs),error.c_str());
            check(state==f.finalState && game.activeObjectCount()==f.trees+4*f.players,
                  "Native forest worker imports exclude destroyed scenery");
            for(uint32_t i=0;i<deleted;++i)
                check(!game.objectActive(f.trees+i+1) && !game.selectObjectForTesting(f.trees+i+1),"Dead573 IDs stay inactive");
            for(uint32_t i=0;i<f.players;++i) {
                const auto id=f.trees+deleted+i+1;
                check(game.objectUnitId(id)==109 && game.objectPlayer(id)==int(i+1),"Forest CC IDs follow scenery creation order");
            }
            for(uint32_t i=0;i<3*f.players;++i) {
                const auto id=f.trees+deleted+f.players+i+1;const int owner=i/3+1;
                check(game.objectUnitId(id)==f.workers[i].unit && game.objectPlayer(id)==owner &&
                      game.objectHitPoints(id)==(owner==8?55:40),"Forest worker IDs and post-map health");
            }
            for(uint32_t owner=1;owner<=f.players;++owner)
                check(game.populationUsedForTesting(owner)==3 && game.populationCapacityForTesting(owner)==(owner==5?200.f:5.f) &&
                      game.resource(owner,98)==525 && game.resource(owner,164)==150 && game.resource(owner,165)==375,
                      "Independent forest worker owners preserve population and cost accounting");
            check(game.spawnObjectForTesting(1,83,1,2.5f,2.5f)==f.trees+deleted+4*f.players+1,
                  "Next native creation preserves removed scenery ID gaps");
        }
    }
    std::puts("PASS: original one/three/eight-owner forest workers with optional573 cleanup");
}
