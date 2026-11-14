// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
static void ownedForestSpawnChecks(swgb::Assets &assets) {
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
)";
    swgb::RmsScript script;std::string error;
    check(script.load(0,[&](int32_t id,std::vector<uint8_t> &bytes) {
        if(id) return false;
        bytes.assign(source.begin(),source.end());return true;
    },{},&error),error.c_str());
    const struct { uint32_t seed,finalState,count,terrainHash,sceneryHash; } cases[]={
        {1u,2026451952u,37,2495521043u,4201021415u},
        {12345u,353298318u,38,1986626585u,2098472240u},
    };
    swgb::Game game(assets);
    for(const auto &f:cases) {
        uint32_t state=f.seed;swgb::RmsMap map;
        check(swgb::generateRmsMap(script,{},32,32,assets.dat(),state,map,&error,2,{1,1}),error.c_str());
        check(state==f.finalState && map.scenery.size()==f.count && map.ownedObjects.size()==2,
              "Original automatic forest scattering to two-owner CC count/RNG");
        uint32_t hash=2166136261u,sceneryHash=hash;
        for(auto t:map.terrain) hash=(hash^t)*16777619u;
        for(const auto &tree:map.scenery) {
            check(tree.alive,"Original live411 tree survives owned-start cleanup");
            for(uint32_t v:{uint32_t(tree.unitId),uint32_t(tree.x*2),uint32_t(tree.y*2),uint32_t(tree.facet)})
                sceneryHash=(sceneryHash^v)*16777619u;
        }
        check(hash==f.terrainHash && sceneryHash==f.sceneryHash,"Original final forest raster, tree order, coordinates and facets");
        state=f.seed;
        check(game.initRms(script,{},32,state,&error,2,{1,1}),error.c_str());
        check(game.activeObjectCount()==f.count+2 && game.objectPlayer(f.count+1)==1 &&
              game.objectPlayer(f.count+2)==2 && game.objectUnitId(f.count+1)==109,
              "Owned forest native IDs follow all original Gaia objects");
        const auto next=game.spawnObjectForTesting(1,83,1,2.5f,2.5f);
        check(next==f.count+3,"Next original allocation follows object IDs, not world registration count");
        const auto before=state;
        check(!game.initRms(script,{},32,state,&error,2,{1,2}) && state==before && game.activeObjectCount()==f.count+3,
              "Unsupported forest civilization context fails atomically");
    }
    std::puts("PASS: original automatic411 scattering followed by two owned Command Centers");
}
