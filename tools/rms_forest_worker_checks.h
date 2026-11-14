// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
static void forestWorkerSpawnChecks(swgb::Assets &assets) {
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
    struct Worker { int unit;float x,y;int facet,frame;float delay; };
    const struct { uint16_t civs[2];uint32_t seed,finalState,trees,hash;Worker workers[6]; } cases[]={
        {{1,1},1u,4221457422u,37,2495521043u,{{83,17.5f,2.5f,7,3,0.000000000f},{83,15.5f,5.5f,7,2,0.000000000f},{83,15.5f,2.5f,3,2,0.000000000f},{293,12.5f,17.5f,7,7,2.707602262f},{293,10.5f,21.5f,1,3,0.000000000f},{293,10.5f,17.5f,7,6,0.000000000f}}},
        {{1,1},12345u,3251164141u,38,1986626585u,{{83,5.5f,17.5f,4,7,0.000000000f},{293,2.5f,20.5f,6,7,0.000000000f},{293,2.5f,21.5f,0,1,0.000000000f},{293,17.5f,10.5f,5,5,0.000000000f},{293,17.5f,12.5f,2,6,0.000000000f},{293,17.5f,8.5f,5,0,0.000000000f}}},
        {{1,2},1u,4221457422u,37,2495521043u,{{83,17.5f,2.5f,7,3,0.000000000f},{83,15.5f,5.5f,7,2,0.000000000f},{83,15.5f,2.5f,3,2,0.000000000f},{293,12.5f,17.5f,7,7,2.707602262f},{293,10.5f,21.5f,1,3,0.000000000f},{293,10.5f,17.5f,7,6,0.000000000f}}},
        {{1,2},12345u,3251164141u,38,1986626585u,{{83,5.5f,17.5f,4,7,0.000000000f},{293,2.5f,20.5f,6,7,0.000000000f},{293,2.5f,21.5f,0,1,0.000000000f},{293,17.5f,10.5f,5,5,0.000000000f},{293,17.5f,12.5f,2,6,0.000000000f},{293,17.5f,8.5f,5,0,0.000000000f}}},
        {{1,6},1u,4221457422u,37,2495521043u,{{83,17.5f,2.5f,7,3,0.000000000f},{83,15.5f,5.5f,7,2,0.000000000f},{83,15.5f,2.5f,3,2,0.000000000f},{293,12.5f,17.5f,7,7,2.707602262f},{293,10.5f,21.5f,1,3,0.000000000f},{293,10.5f,17.5f,7,6,0.000000000f}}},
        {{1,6},12345u,3251164141u,38,1986626585u,{{83,5.5f,17.5f,4,7,0.000000000f},{293,2.5f,20.5f,6,7,0.000000000f},{293,2.5f,21.5f,0,1,0.000000000f},{293,17.5f,10.5f,5,5,0.000000000f},{293,17.5f,12.5f,2,6,0.000000000f},{293,17.5f,8.5f,5,0,0.000000000f}}},
        {{1,8},1u,4221457422u,37,2495521043u,{{83,17.5f,2.5f,7,3,0.000000000f},{83,15.5f,5.5f,7,2,0.000000000f},{83,15.5f,2.5f,3,2,0.000000000f},{293,12.5f,17.5f,7,7,2.707602262f},{293,10.5f,21.5f,1,3,0.000000000f},{293,10.5f,17.5f,7,6,0.000000000f}}},
        {{1,8},12345u,3251164141u,38,1986626585u,{{83,5.5f,17.5f,4,7,0.000000000f},{293,2.5f,20.5f,6,7,0.000000000f},{293,2.5f,21.5f,0,1,0.000000000f},{293,17.5f,10.5f,5,5,0.000000000f},{293,17.5f,12.5f,2,6,0.000000000f},{293,17.5f,8.5f,5,0,0.000000000f}}},
        {{2,1},1u,4221457422u,37,3100568435u,{{83,17.5f,2.5f,7,3,0.000000000f},{83,15.5f,5.5f,7,2,0.000000000f},{83,15.5f,2.5f,3,2,0.000000000f},{293,12.5f,17.5f,7,7,2.707602262f},{293,10.5f,21.5f,1,3,0.000000000f},{293,10.5f,17.5f,7,6,0.000000000f}}},
        {{2,1},12345u,3251164141u,38,1873433801u,{{83,5.5f,17.5f,4,7,0.000000000f},{293,2.5f,20.5f,6,7,0.000000000f},{293,2.5f,21.5f,0,1,0.000000000f},{293,17.5f,10.5f,5,5,0.000000000f},{293,17.5f,12.5f,2,6,0.000000000f},{293,17.5f,8.5f,5,0,0.000000000f}}},
        {{6,1},1u,4221457422u,37,2334202139u,{{83,17.5f,2.5f,7,3,0.000000000f},{83,15.5f,5.5f,7,2,0.000000000f},{83,15.5f,2.5f,3,2,0.000000000f},{293,12.5f,17.5f,7,7,2.707602262f},{293,10.5f,21.5f,1,3,0.000000000f},{293,10.5f,17.5f,7,6,0.000000000f}}},
        {{6,1},12345u,3251164141u,38,3002863657u,{{83,5.5f,17.5f,4,7,0.000000000f},{293,2.5f,20.5f,6,7,0.000000000f},{293,2.5f,21.5f,0,1,0.000000000f},{293,17.5f,10.5f,5,5,0.000000000f},{293,17.5f,12.5f,2,6,0.000000000f},{293,17.5f,8.5f,5,0,0.000000000f}}},
    };
    swgb::Game game(assets);
    for(const auto &f:cases) {
        uint32_t state=f.seed;swgb::RmsMap map;
        check(swgb::generateRmsMap(script,{},32,32,assets.dat(),state,map,&error,2,{f.civs[0],f.civs[1]}),error.c_str());
        check(state==f.finalState && map.scenery.size()==f.trees && map.ownedObjects.size()==8,
              "Original forest-scattering/CC/worker full-stage RNG and counts");
        uint32_t hash=2166136261u;for(auto tile:map.terrain) hash=(hash^tile)*16777619u;
        check(hash==f.hash,"Original forest/worker final terrain across civilization foundations");
        for(size_t i=0;i<6;++i) {
            const auto &a=map.ownedObjects[i+2];const auto &e=f.workers[i];
            check(a.unitId==e.unit && a.x==e.x && a.y==e.y && a.owner==(i<3?1:2) &&
                  a.facet==e.facet && a.animationFrame==e.frame && std::abs(a.initialIdleDelay-e.delay)<.00001f,
                  "Original forest worker order, position, variant, pose and animation");
        }
        state=f.seed;
        check(game.initRms(script,{},32,state,&error,2,{f.civs[0],f.civs[1]}),error.c_str());
        check(state==f.finalState && game.activeObjectCount()==f.trees+8,
              "Native forest workers import all original trees and owned objects");
        for(size_t i=0;i<6;++i) {
            const auto id=f.trees+uint32_t(i)+3;const int owner=i<3?1:2;
            check(game.objectUnitId(id)==f.workers[i].unit && game.objectPlayer(id)==owner &&
                  game.objectHitPoints(id)==(f.civs[owner-1]==8?55:40),
                  "Forest worker creation IDs and post-map health");
        }
        for(int owner=1;owner<=2;++owner)
            check(game.populationUsedForTesting(owner)==3 && game.resource(owner,98)==525 &&
                  game.resource(owner,164)==150 && game.resource(owner,165)==375,
                  "Forest workers retain independent population and original cost accounting");
    }
    std::puts("PASS: original two-owner workers with forest scattering and civilization foundations");
}
