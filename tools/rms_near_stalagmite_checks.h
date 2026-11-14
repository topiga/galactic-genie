// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Original default-startup oracles: sizes8/10 clear both573 at CC1; size12 clears each at its own CC.
static void nearStalagmiteCleanupChecks(swgb::Assets &assets) {
    const std::string source=R"(<LAND_GENERATION>
base_terrain 41
create_player_lands { terrain_type 9 base_size 3 number_of_tiles 60 }
<TERRAIN_GENERATION>
create_terrain 10 { base_terrain 9 number_of_tiles 4 number_of_clumps 2 spacing_to_other_terrain_types 1 }
create_terrain 6 { base_terrain 9 number_of_tiles 9 number_of_clumps 2 clumping_factor 6 }
<OBJECTS_GENERATION>
create_object 573 { set_gaia_object_only set_place_for_every_player number_of_objects 1 min_distance_to_players 0 max_distance_to_players 0 group_placement_radius 18 }
create_object 109 { set_place_for_every_player number_of_objects 1 min_distance_to_players 0 max_distance_to_players 0 }
)";
    swgb::RmsScript script;std::string error;
    check(script.load(0,[&](int32_t id,std::vector<uint8_t> &bytes) {
        if(id) return false;
        bytes.assign(source.begin(),source.end());return true;
    },{},&error),error.c_str());
    const struct { uint32_t size,seed,finalState,sceneryHash,terrainHashes[3]; } cases[]={
        {8,1u,1612182385u,1251157901u,{4002731625u,1652659609u,888148281u}},
        {10,12345u,1951333389u,2283084203u,{3672679910u,1826108886u,3522515766u}},
        {12,1u,83059500u,505731179u,{1986448999u,3888774327u,329497591u}},
        {12,12345u,1011388293u,2674859499u,{310018107u,1860129747u,1499427899u}},
    };
    swgb::Game game(assets);
    for(const auto &f:cases) for(uint16_t civ:{1,2,6}) {
        uint32_t state=f.seed;swgb::RmsMap map;
        check(swgb::generateRmsMap(script,{},f.size,f.size,assets.dat(),state,map,&error,2,{civ,1}),error.c_str());
        check(state==f.finalState && map.scenery.size()==6 && map.ownedObjects.size()==2,
              "Original near573 cleanup counts and RNG across civilization foundations");
        uint32_t terrainHash=2166136261u,sceneryHash=terrainHash;
        for(auto tile:map.terrain) terrainHash=(terrainHash^tile)*16777619u;
        for(size_t i=0;i<4;++i) {
            const auto &tree=map.scenery[i];check(tree.alive,"Near cleanup preserves original forest trees");
            for(uint32_t value:{uint32_t(tree.unitId),uint32_t(tree.x*2),uint32_t(tree.y*2),uint32_t(tree.facet)})
                sceneryHash=(sceneryHash^value)*16777619u;
        }
        check(sceneryHash==f.sceneryHash && terrainHash==f.terrainHashes[civ==2?1:civ==6?2:0],
              "Original near cleanup final terrain and tree positions/facets");
        for(size_t i=0;i<2;++i) {
            const auto &dead=map.scenery[4+i];const auto &cc=map.ownedObjects[i];
            check(dead.unitId==573 && !dead.alive && dead.x==map.playerStarts[i].x && dead.y==map.playerStarts[i].y &&
                  cc.unitId==109 && cc.owner==int(i+1) && cc.x==dead.x && cc.y==dead.y,
                  "Both original573 become inactive, including shared CC cleanup footprints");
        }
        state=f.seed;
        check(game.initRms(script,{},f.size,state,&error,2,{civ,1}),error.c_str());
        check(state==f.finalState && game.activeObjectCount()==6,"Near import keeps four trees and two CCs");
        for(uint32_t i=0;i<2;++i) {
            check(!game.objectActive(5+i) && !game.selectObjectForTesting(5+i),"Deleted near573 cannot be selected");
            check(game.objectUnitId(7+i)==109 && game.objectPlayer(7+i)==int(i+1),"Near CC IDs preserve deleted573 ordinals");
        }
        check(game.spawnObjectForTesting(1,83,1,2.5f,2.5f)==9,"Next near creation does not reuse deleted573 IDs");
    }
    std::puts("PASS: original near573 cleanup across shared and separate footprints");
}
