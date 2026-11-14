// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Actual original413 scatter and109/83/550/285 startup on authored native
// geometry. Separate original helper tests establish preceding geometry stages.
static void checkRmsForest21Holocrons(const swgb::dat::DatFile &data) {
    const std::string source=R"(<LAND_GENERATION> base_terrain 6 create_player_lands { terrain_type 6 base_size 3 number_of_tiles 60 }
<ELEVATION_GENERATION> create_elevation 2 { base_terrain 6 number_of_tiles 200 number_of_clumps 4 }
<TERRAIN_GENERATION> create_terrain 21 { base_terrain 6 number_of_tiles 100 number_of_clumps 4 spacing_to_other_terrain_types 1 set_avoid_player_start_areas }
<OBJECTS_GENERATION>
create_object 109 { set_place_for_every_player min_distance_to_players 0 max_distance_to_players 0 }
create_object 83 { set_place_for_every_player min_distance_to_players 6 max_distance_to_players 6 }
create_object 550 { set_place_for_every_player min_distance_to_players 7 max_distance_to_players 9 match_player_civ }
create_object 285 { number_of_objects 5 min_distance_to_players 25 temp_min_distance_group_placement 20 }
)";
    swgb::RmsScript script;std::string error;
    check(script.load(0,[&](int32_t id,std::vector<uint8_t>&out) {
        if(id)return false;out.assign(source.begin(),source.end());return true;
    },{},&error),error.c_str());
    auto hash=[](const std::vector<uint8_t>&bytes) {
        uint32_t value=2166136261u;for(auto b:bytes)value=(value^b)*16777619u;return value;
    };
    struct Point {float x,y;};
    {
        uint32_t rng=1u;swgb::RmsMap map;
        check(swgb::generateRmsMap(script,{},120,120,data,rng,map,&error,2,{1,1}),error.c_str());
        check(rng==3135314386u && map.ownedObjects.size()==10 && map.resourcesAfterOwned.size()==5 && map.scenery.size()==102,"Forest21 holocron final RNG/count");
        check(hash(map.terrain)==1015550221u,"Forest21 holocron original terrain");
        check(hash(map.elevation)==4089223887u,"Forest21 holocron original elevation");
        check(hash(map.slopes)==140112869u,"Forest21 holocron original slopes");
        const Point points[]={{14.5f,4.5f},{111.5f,0.5f},{16.5f,33.5f},{8.5f,107.5f},{107.5f,72.5f}};
        for(size_t i=0;i<5;++i) {
            const auto &a=map.resourcesAfterOwned[i];const auto &e=points[i];
            check(a.unitId==285 && a.x==e.x && a.y==e.y && a.facet==7 && a.headingRadians==0,"Forest21 original holocron pose");
        }
    }
    {
        uint32_t rng=12345u;swgb::RmsMap map;
        check(swgb::generateRmsMap(script,{},120,120,data,rng,map,&error,2,{1,1}),error.c_str());
        check(rng==2099619989u && map.ownedObjects.size()==10 && map.resourcesAfterOwned.size()==5 && map.scenery.size()==103,"Forest21 holocron final RNG/count");
        check(hash(map.terrain)==520995590u,"Forest21 holocron original terrain");
        check(hash(map.elevation)==2763925479u,"Forest21 holocron original elevation");
        check(hash(map.slopes)==2713444239u,"Forest21 holocron original slopes");
        const Point points[]={{41.5f,116.5f},{70.5f,13.5f},{85.5f,97.5f},{55.5f,42.5f},{86.5f,75.5f}};
        for(size_t i=0;i<5;++i) {
            const auto &a=map.resourcesAfterOwned[i];const auto &e=points[i];
            check(a.unitId==285 && a.x==e.x && a.y==e.y && a.facet==7 && a.headingRadians==0,"Forest21 original holocron pose");
        }
    }
}
