// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
static void stalagmiteWorkerSpawnChecks(swgb::Assets &assets) {
    const std::string source=R"(/* Authored diagnostic using the forest and dry terrain types in the packaged Flats map. */
<LAND_GENERATION>
base_terrain 41
create_player_lands {
  terrain_type 9
  base_size 3
  number_of_tiles 60
}
<TERRAIN_GENERATION>
create_terrain 10 {
  set_avoid_player_start_areas
  base_terrain 41
  number_of_tiles 37
  number_of_clumps 4
  spacing_to_other_terrain_types 1
}
create_terrain 6 {
  base_terrain 9
  number_of_tiles 9
  number_of_clumps 2
  clumping_factor 6
}

<OBJECTS_GENERATION>
create_object 573 {
  set_gaia_object_only
  set_place_for_every_player
  number_of_objects 1
  min_distance_to_players 0
  max_distance_to_players 0
  group_placement_radius 18
}
create_object 109 {
  number_of_objects 1
  set_place_for_every_player
  min_distance_to_players 0
  max_distance_to_players 0
}
create_object 83 {
  number_of_objects 1
  set_place_for_every_player
  min_distance_to_players 1
  max_distance_to_players 3
}
)";
    swgb::RmsScript script;std::string error;
    check(script.load(0,[&](int32_t id,std::vector<uint8_t> &bytes) {
        if(id) return false;
        bytes.assign(source.begin(),source.end());return true;
    },{},&error),error.c_str());
    // Original delay includes the base one-second frame; native stores only its extra hold.
    struct Worker { int unit;float x,y;int facet,frame;float delay; };
    struct Fixture { uint32_t seed,finalState,trees,sceneryHash,terrainHashes[3];Worker workers[6]; };
    const Fixture cases[]={
        {1u,4221457422u,37,4201021415u,{2495521043u,3100568435u,2334202139u},{{83,17.5f,2.5f,7,3,1.000000000f},{83,15.5f,5.5f,7,2,1.000000000f},{83,15.5f,2.5f,3,2,1.000000000f},{293,12.5f,17.5f,7,7,3.707602262f},{293,10.5f,21.5f,1,3,1.000000000f},{293,10.5f,17.5f,7,6,1.000000000f}}},
        {12345u,3251164141u,38,2098472240u,{1986626585u,1873433801u,3002863657u},{{83,5.5f,17.5f,4,7,1.000000000f},{293,2.5f,20.5f,6,7,1.000000000f},{293,2.5f,21.5f,0,1,1.000000000f},{293,17.5f,10.5f,5,5,1.000000000f},{293,17.5f,12.5f,2,6,1.000000000f},{293,17.5f,8.5f,5,0,1.000000000f}}},
    };
    const uint16_t civilizations[][2]={{1,1},{1,2},{1,6},{1,8},{2,1},{6,1}};
    swgb::Game game(assets);
    for(const auto &civs:civilizations) for(const auto &f:cases) {
        uint32_t state=f.seed;swgb::RmsMap map;
        check(swgb::generateRmsMap(script,{},32,32,assets.dat(),state,map,&error,2,{civs[0],civs[1]}),error.c_str());
        check(state==f.finalState && map.scenery.size()==f.trees+2 && map.ownedObjects.size()==8,
              "Original573 cleanup with workers preserves stage RNG and creation count");
        uint32_t terrainHash=2166136261u,sceneryHash=terrainHash;
        for(auto tile:map.terrain) terrainHash=(terrainHash^tile)*16777619u;
        for(size_t i=0;i<f.trees;++i) {
            const auto &tree=map.scenery[i];check(tree.alive,"Cleanup retains living forest trees");
            for(uint32_t value:{uint32_t(tree.unitId),uint32_t(tree.x*2),uint32_t(tree.y*2),uint32_t(tree.facet)})
                sceneryHash=(sceneryHash^value)*16777619u;
        }
        check(terrainHash==f.terrainHashes[civs[0]==2?1:civs[0]==6?2:0] && sceneryHash==f.sceneryHash,
              "Original terrain foundations and automatic tree order remain unchanged");
        for(size_t i=0;i<2;++i) {
            const auto &dead=map.scenery[f.trees+i];
            check(dead.unitId==573 && !dead.alive && dead.x==map.playerStarts[i].x && dead.y==map.playerStarts[i].y,
                  "Original573 becomes inactive at the generated player start");
        }
        for(size_t i=0;i<6;++i) {
            const auto &a=map.ownedObjects[i+2];const auto &e=f.workers[i];
            check(a.unitId==e.unit && a.owner==(i<3?1:2) && a.x==e.x && a.y==e.y &&
                  a.facet==e.facet && a.animationFrame==e.frame && std::abs(a.initialIdleDelay-(e.delay-1.f))<.00001f,
                  "Original worker collision gates, variants, poses and animation survive573 cleanup");
        }
        state=f.seed;
        check(game.initRms(script,{},32,state,&error,2,{civs[0],civs[1]}),error.c_str());
        check(state==f.finalState && game.activeObjectCount()==f.trees+8,"Native import excludes two dead573 from active objects");
        for(uint32_t i=0;i<2;++i) {
            const auto deadId=f.trees+i+1,ccId=f.trees+i+3;
            check(game.objectUnitId(deadId)==-1 && !game.objectActive(deadId) && game.objectHitPoints(deadId)==0 &&
                  !game.selectObjectForTesting(deadId),"Deleted573 reserves its ID without being active or selectable");
            check(game.objectUnitId(ccId)==109 && game.objectPlayer(ccId)==int(i+1),"CC IDs include both earlier573 creations");
        }
        for(size_t i=0;i<6;++i) {
            const auto id=f.trees+uint32_t(i)+5;const int owner=i<3?1:2;
            check(game.objectUnitId(id)==f.workers[i].unit && game.objectPlayer(id)==owner &&
                  game.objectHitPoints(id)==(civs[owner-1]==8?55:40),"Worker native IDs shift by two and retain post-map health");
        }
        for(int owner=1;owner<=2;++owner)
            check(game.populationUsedForTesting(owner)==3 && game.resource(owner,98)==525 &&
                  game.resource(owner,164)==150 && game.resource(owner,165)==375,"Cleanup does not change worker population or costs");
        check(game.spawnObjectForTesting(1,83,1,2.5f,2.5f)==f.trees+11,"Dead573 IDs are not reused by the next creation");
    }
    std::puts("PASS: original573 cleanup with workers and mixed civilization foundations");
}
