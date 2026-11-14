// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Original paired forest/573/CC full-stage oracles; default owner masks, civ(owner)=owner.
static void manyForestCleanupChecks(swgb::Assets &assets) {
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
    const struct { uint32_t players,seed,trees,finalState,terrainHash,sceneryHash;int centers[8][2]; } cases[]={
        {1,1u,38,4147748808u,3232917026u,1264919057u,{{18,5}}},
        {1,12345u,37,1841330707u,297464486u,3207088u,{{20,11}}},
        {3,1u,38,2552709919u,419769886u,3889132145u,{{18,5},{23,20},{5,18}}},
        {3,12345u,38,4272090361u,2885109120u,3116409675u,{{5,10},{20,11},{15,20}}},
        {4,1u,40,3839142062u,928422742u,2996465698u,{{18,5},{20,18},{5,13},{13,20}}},
        {4,12345u,37,3221072518u,3249248165u,4103688181u,{{11,5},{20,11},{5,20},{20,20}}},
        {5,1u,38,1630313889u,2197859924u,4202896513u,{{18,5},{19,20},{5,10},{5,22},{20,15}}},
        {5,12345u,37,1402663658u,2188957159u,2060409708u,{{14,5},{20,11},{5,14},{11,20},{23,20}}},
        {6,1u,37,226174596u,2928608459u,278001527u,{{18,5},{23,20},{5,18},{13,20},{20,13},{8,5}}},
        {6,12345u,37,1728759869u,292303585u,649809808u,{{16,5},{20,11},{5,10},{5,20},{20,21},{15,20}}},
        {7,1u,37,789884838u,1735463615u,421882643u,{{18,5},{20,19},{5,18},{11,20},{20,11},{5,10},{19,20}}},
        {7,12345u,37,1908590390u,2734012707u,630592376u,{{14,5},{20,11},{5,10},{5,18},{20,19},{11,20},{19,20}}},
        {8,1u,14,1002245335u,930223842u,70354911u,{{18,5},{20,17},{5,23},{15,20},{20,10},{5,9},{22,20},{5,16}}},
        {8,12345u,37,3870601973u,1196344967u,77405676u,{{15,5},{20,11},{8,5},{5,15},{20,18},{14,20},{21,20},{5,22}}},
    };
    swgb::Game game(assets);std::string error;
    for(bool clearScenery:{false,true}) {
        const auto source=prefix+(clearScenery?cleanup:std::string())+cc;
        swgb::RmsScript script;
        check(script.load(0,[&](int32_t id,std::vector<uint8_t> &bytes) {
            if(id) return false;
            bytes.assign(source.begin(),source.end());return true;
        },{},&error),error.c_str());
        for(const auto &f:cases) {
            std::vector<uint16_t> civs;for(uint16_t owner=1;owner<=f.players;++owner)civs.push_back(owner);
            const uint32_t deleted=clearScenery?f.players:0;
            uint32_t state=f.seed;swgb::RmsMap map;
            check(swgb::generateRmsMap(script,{},32,32,assets.dat(),state,map,&error,f.players,civs),error.c_str());
            check(state==f.finalState && map.scenery.size()==f.trees+deleted && map.ownedObjects.size()==f.players,
                  "Original many-owner forest and cleanup stage RNG/counts");
            uint32_t terrainHash=2166136261u,sceneryHash=terrainHash;
            for(auto tile:map.terrain)terrainHash=(terrainHash^tile)*16777619u;
            for(uint32_t i=0;i<f.trees;++i) {
                const auto &tree=map.scenery[i];check(tree.alive,"Many-owner cleanup retains live trees");
                for(uint32_t value:{uint32_t(tree.unitId),uint32_t(tree.x*2),uint32_t(tree.y*2),uint32_t(tree.facet)})
                    sceneryHash=(sceneryHash^value)*16777619u;
            }
            check(terrainHash==f.terrainHash && sceneryHash==f.sceneryHash,"Original many-owner forest raster and scattering order");
            for(uint32_t i=0;i<f.players;++i) {
                const auto &o=map.ownedObjects[i];
                check(o.unitId==109 && o.owner==i+1 && o.civilization==i+1 && o.x==f.centers[i][0] && o.y==f.centers[i][1] &&
                      o.pendingFoundation==(i?27:255),"Original owner positions, mixed civilizations and deferred foundations");
                if(clearScenery) {
                    const auto &dead=map.scenery[f.trees+i];
                    check(dead.unitId==573 && !dead.alive && dead.x==o.x && dead.y==o.y,"Every generated573 is cleared by the CC rows");
                }
            }
            state=f.seed;
            check(game.initRms(script,{},32,state,&error,f.players,civs),error.c_str());
            check(state==f.finalState && game.activeObjectCount()==f.trees+f.players,"Native import preserves many-owner living counts");
            for(uint32_t i=0;i<f.players;++i) {
                const auto id=f.trees+deleted+i+1;
                check(game.objectUnitId(id)==109 && game.objectPlayer(id)==int(i+1),"CC IDs follow actual scenery creation ordinals");
                if(clearScenery)check(!game.objectActive(f.trees+i+1) && !game.selectObjectForTesting(f.trees+i+1),
                                      "Deleted573 IDs remain inactive and unselectable");
            }
            check(game.spawnObjectForTesting(1,83,1,2.5f,2.5f)==f.trees+deleted+f.players+1,
                  "Actual next allocation preserves all deleted scenery ID gaps");
        }
    }
    std::puts("PASS: original one and three-to-eight owner forest with optional573 cleanup");
}
