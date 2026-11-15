// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
static void rmsWorkerTreeCollisionChecks(swgb::Assets &assets) {
    // Original seed8 rejects three generated411 trees inside4de830 before placing workers.
    const std::string source=R"(<LAND_GENERATION>
base_terrain 41
create_player_lands { terrain_type 9 base_size 3 number_of_tiles 60 }
<TERRAIN_GENERATION>
create_terrain 10 {
  base_terrain 9
  number_of_tiles 4
  number_of_clumps 2
  spacing_to_other_terrain_types 1
  clumping_factor 20
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
})";
    swgb::RmsScript script;std::string error;
    check(script.load(0,[&](int32_t id,std::vector<uint8_t> &bytes) {
        if(id) return false;
        bytes.assign(source.begin(),source.end());return true;
    },{},&error),error.c_str());
    const struct { float x,y;int facet; } trees[]={
        {16.5f,2.5f,2},{17.5f,2.5f,8},{16.5f,3.5f,3},{19.5f,11.5f,1},
    };
    const struct { float x,y;int facet,frame;float delay; } workers[]={
        {16.5f,5.5f,1,7,0},{18.5f,2.5f,7,7,.875667572f},{19.5f,2.5f,6,3,0},
        {12.5f,17.5f,3,1,0},{11.5f,17.5f,7,0,0},{9.5f,20.5f,1,4,0},
    };
    uint32_t state=8;swgb::RmsMap map;
    check(swgb::generateRmsMap(script,{},32,32,assets.dat(),state,map,&error,2,{1,1}),error.c_str());
    check(state==358447027u && map.scenery.size()==4 && map.ownedObjects.size()==8,
          "Original tree-collision full-stage RNG and object counts");
    uint32_t hash=2166136261u;
    for(auto tile:map.terrain) hash=(hash^tile)*16777619u;
    check(hash==1502501308u,"Original inland forest and final foundation terrain");
    for(size_t i=0;i<4;++i) {
        const auto &a=map.scenery[i];const auto &e=trees[i];
        check(a.alive && a.unitId==411 && a.x==e.x && a.y==e.y && a.facet==e.facet,
              "Original generated tree order, positions and facets survive worker placement");
    }
    check(map.ownedObjects[0].unitId==109 && map.ownedObjects[0].owner==1 &&
          map.ownedObjects[0].x==19 && map.ownedObjects[0].y==5 && map.ownedObjects[0].pendingFoundation==255 &&
          map.ownedObjects[1].unitId==109 && map.ownedObjects[1].owner==2 &&
          map.ownedObjects[1].x==12 && map.ownedObjects[1].y==20 && map.ownedObjects[1].pendingFoundation==27,
          "Original Command Center order and deferred enemy foundation");
    for(size_t i=0;i<6;++i) {
        const auto &a=map.ownedObjects[i+2];const auto &e=workers[i];
        check(a.unitId==293 && a.owner==(i<3?1:2) && a.x==e.x && a.y==e.y &&
              a.facet==e.facet && a.animationFrame==e.frame && std::abs(a.initialIdleDelay-e.delay)<.00001f,
              "Original worker results after occupied-tree candidates consume no RNG");
    }
    swgb::Game game(assets);state=8;
    check(game.initRms(script,{},32,state,&error,2,{1,1}),error.c_str());
    check(state==358447027u && game.activeObjectCount()==12,"Native tree-collision scene import count and RNG");
    for(size_t i=0;i<4;++i) {
        const uint32_t id=uint32_t(i)+1;const auto position=game.objectPosition(id);
        check(game.objectUnitId(id)==411 && game.objectPlayer(id)==0 &&
              position[0]==trees[i].y && position[1]==32-trees[i].x,"Original tree creation IDs remain intact");
    }
    for(size_t i=0;i<6;++i) {
        const uint32_t id=uint32_t(i)+7;const auto position=game.objectPosition(id);
        check(game.objectUnitId(id)==293 && game.objectPlayer(id)==(i<3?1:2) && game.objectHitPoints(id)==40 &&
              position[0]==workers[i].y && position[1]==32-workers[i].x,"Original worker creation IDs and positions after trees");
    }
    for(int owner=1;owner<=2;++owner)
        check(game.populationUsedForTesting(owner)==3 && game.populationCapacityForTesting(owner)==5 &&
              game.resource(owner,11)==3 && game.resource(owner,37)==3 && game.resource(owner,98)==525 &&
              game.resource(owner,164)==150 && game.resource(owner,165)==375 && game.resource(owner,0)==0,
              "Original per-owner worker population and cost accounting after tree rejection");
    std::puts("PASS: original full worker rows reject generated trees without consuming RNG");
}
