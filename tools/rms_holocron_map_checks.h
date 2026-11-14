// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
static void rmsHolocronMapChecks(const swgb::dat::DatFile &data) {
    const std::string objects=R"(<OBJECTS_GENERATION>
create_object 109 {
    number_of_objects 1
    set_place_for_every_player
    min_distance_to_players 0
    max_distance_to_players 0
}
create_object 83 {
    number_of_objects 1
    set_place_for_every_player
    min_distance_to_players 6
    max_distance_to_players 6
}

create_object 550 {
  set_place_for_every_player
  min_distance_to_players 7
  max_distance_to_players 9
  match_player_civ
}
create_object 285 { number_of_objects 5 min_distance_to_players 25 temp_min_distance_group_placement 20 })";
    const struct {uint32_t size,forest,seed,final,trees,terrainHash,count;float positions[5][2];} cases[]={
        {64u,0u,1u,2844409869u,0u,550507413u,5u,{{11.5f,2.5f},{58.5f,49.5f},{0.5f,50.5f},{0.5f,23.5f},{62.5f,0.5f}}},
        {64u,0u,12345u,4069983840u,0u,338914421u,3u,{{61.5f,59.5f},{36.5f,47.5f},{6.5f,10.5f}}},
        {120u,0u,1u,2502753765u,0u,2450304117u,5u,{{41.5f,50.5f},{8.5f,68.5f},{76.5f,95.5f},{3.5f,92.5f},{100.5f,117.5f}}},
        {120u,0u,12345u,1266959076u,0u,2911969909u,5u,{{98.5f,5.5f},{87.5f,109.5f},{57.5f,109.5f},{45.5f,17.5f},{70.5f,67.5f}}},
        {120u,37u,1u,498484780u,38u,3882501562u,5u,{{112.5f,49.5f},{26.5f,108.5f},{95.5f,105.5f},{9.5f,45.5f},{31.5f,48.5f}}},
        {120u,37u,12345u,3229700452u,38u,817449428u,5u,{{56.5f,58.5f},{13.5f,34.5f},{87.5f,92.5f},{108.5f,72.5f},{44.5f,30.5f}}},
        {120u,2500u,1u,4110658407u,2541u,654092012u,5u,{{7.5f,98.5f},{3.5f,13.5f},{52.5f,43.5f},{112.5f,116.5f},{72.5f,79.5f}}},
        {120u,2500u,12345u,2773853338u,2535u,22999099u,5u,{{29.5f,20.5f},{46.5f,102.5f},{37.5f,44.5f},{115.5f,60.5f},{86.5f,104.5f}}}
    };
    std::string error;
    for(const auto &f:cases) {
        const std::string land=std::string("<LAND_GENERATION> base_terrain ")+(f.forest?"41":"0")+
            " create_player_lands { terrain_type "+(f.forest?"9":"0")+" base_size 3 number_of_tiles 60 } ";
        const std::string paint=f.forest?"<TERRAIN_GENERATION> create_terrain 10 { base_terrain 41 number_of_tiles "+std::to_string(f.forest)+
            " number_of_clumps 4 spacing_to_other_terrain_types 1 set_avoid_player_start_areas } create_terrain 6 { base_terrain 9 number_of_tiles 9 number_of_clumps 2 clumping_factor 6 } ":"";
        const std::string source=land+paint+objects;swgb::RmsScript script;
        check(script.load(0,[&](int32_t id,std::vector<uint8_t> &bytes){if(id)return false;bytes.assign(source.begin(),source.end());return true;},{},&error),error.c_str());
        uint32_t rng=f.seed;swgb::RmsMap map;
        check(swgb::generateRmsMap(script,{},f.size,f.size,data,rng,map,&error,2,{1,1}),error.c_str());
        uint32_t hash=2166136261u;for(auto t:map.terrain)hash=(hash^t)*16777619u;
        check(rng==f.final && map.scenery.size()==f.trees && map.ownedObjects.size()==10 &&
              hash==f.terrainHash && map.resourcesAfterOwned.size()==f.count,"Original composed holocron map counts, terrain and RNG");
        for(size_t i=0;i<f.count;++i) {
            const auto &p=map.resourcesAfterOwned[i];
            check(p.unitId==285 && p.x==f.positions[i][0] && p.y==f.positions[i][1] && p.facet==7 && p.headingRadians==0,
                  "Original holocron pipeline placement and appearance");
        }
        rng=f.seed;
        check(!swgb::generateRmsMap(script,{"FIXED_POSITIONS"},f.size,f.size,data,rng,map,&error,2,{1,1}) &&
            rng==f.seed && map.terrain.empty(),"Unverified fixed-position holocron pipeline rejected");
    }
    std::puts("PASS: original holocron map pipeline on64 grass and120 grass/sparse/dense forest");
}
