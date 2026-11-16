// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <array>
// Actual full original runner; equivalent aliases from original54000, no profile-specific admission.
static void rmsCommonBorderChecks(swgb::Assets &assets) {
    const std::string source=R"(#const DIRT1 6
#const BLDG-MAIN1 109
#const UNIT-WORKER 83
#const UNIT-BIKE-E 550
<PLAYER_SETUP> random_placement
<LAND_GENERATION>
base_terrain DIRT1
create_player_lands { terrain_type DIRT1 land_percent 100 base_size 7 left_border 10 right_border 20 top_border 15 bottom_border 5 }
<TERRAIN_GENERATION>
<OBJECTS_GENERATION>
create_object BLDG-MAIN1 { set_place_for_every_player group_placement_radius 18 min_distance_to_players 0 max_distance_to_players 0 }
create_object UNIT-WORKER { set_place_for_every_player min_distance_to_players 6 max_distance_to_players 6 }
create_object UNIT-BIKE-E { number_of_objects 1 set_place_for_every_player min_distance_to_players 7 max_distance_to_players 9 match_player_civ }
)";
    const struct {uint32_t seed,rng,terrain;std::vector<std::array<float,6>> objects;} cases[]={
        {1u,3895143042u,134834165u,{{{0.0f,1.0f,109.0f,0.0f,82.0f,44.0f}},{{1.0f,2.0f,109.0f,0.0f,35.0f,92.0f}},{{2.0f,1.0f,293.0f,7.0f,82.5f,38.5f}},{{3.0f,1.0f,293.0f,7.0f,76.5f,38.5f}},{{4.0f,1.0f,293.0f,3.0f,76.5f,46.5f}},{{5.0f,2.0f,293.0f,3.0f,37.5f,86.5f}},{{6.0f,2.0f,293.0f,6.0f,29.5f,96.5f}},{{7.0f,2.0f,293.0f,1.0f,31.5f,86.5f}},{{8.0f,1.0f,550.0f,3.0f,88.5f,51.5f}},{{9.0f,2.0f,550.0f,0.0f,33.5f,85.5f}}}},
        {20261116u,1396663036u,2611311893u,{{{0.0f,1.0f,109.0f,0.0f,79.0f,63.0f}},{{1.0f,2.0f,109.0f,0.0f,23.0f,76.0f}},{{2.0f,1.0f,293.0f,4.0f,73.5f,62.5f}},{{3.0f,1.0f,293.0f,1.0f,73.5f,67.5f}},{{4.0f,1.0f,293.0f,3.0f,73.5f,57.5f}},{{5.0f,2.0f,293.0f,3.0f,17.5f,73.5f}},{{6.0f,2.0f,293.0f,2.0f,23.5f,70.5f}},{{7.0f,2.0f,293.0f,7.0f,17.5f,72.5f}},{{8.0f,1.0f,550.0f,1.0f,71.5f,65.5f}},{{9.0f,2.0f,550.0f,6.0f,30.5f,70.5f}}}},
    };
    swgb::RmsScript script;std::string error;
    check(script.load(60000,[&](int32_t id,std::vector<uint8_t> &bytes){if(id!=60000)return false;bytes.assign(source.begin(),source.end());return true;},{},&error),error.c_str());
    for(const auto &c:cases) {
        uint32_t rng=c.seed;swgb::RmsMap map;
        check(swgb::generateRmsMap(script,{"TINY_MAP","FIXED_POSITIONS","DEFAULT_RESOURCES"},120,120,assets.dat(),rng,map,&error,2,{1,1}),error.c_str());
        std::vector<std::array<float,6>> objects;
        const auto append=[&](const auto &rows){for(const auto &o:rows)objects.push_back({float(objects.size()),float(o.owner),float(o.unitId),float(o.facet),o.x,o.y});};
        append(map.ownedObjects);append(map.resourcesAfterOwned);
        uint32_t terrain=2166136261u;for(auto tile:map.terrain)terrain=(terrain^tile)*16777619u;
        check(map.cliffs.empty() && map.scenery.empty() && map.objects.empty() && objects==c.objects && rng==c.rng && terrain==c.terrain,"original authored bordered owned startup without map-name exceptions");
        check(map.elevation.empty() && map.slopes.empty(),"authored border fixture is flat");
    }
}
