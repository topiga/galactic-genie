// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "../src/engine/rms_walls.h"
// Actual499a60 stages; local rms-wall117-native-prototype findings, 2026-11-16.
static void ownedRmsWallBoundaryChecks(swgb::Assets &assets) {
    const std::string source=R"(#const RMS_6 6
#const RMS_21 21
#const RMS_109 109
#const RMS_83 83
#const RMS_550 550
#const RMS_87 87
<LAND_GENERATION> base_terrain RMS_6 create_player_lands { terrain_type RMS_6 base_size 13 number_of_tiles 7200 } <CLIFF_GENERATION> <TERRAIN_GENERATION> create_terrain RMS_21 { base_terrain RMS_6 number_of_tiles 6000 number_of_clumps 60 spacing_to_other_terrain_types 2 clumping_factor 20 set_avoid_player_start_areas } <OBJECTS_GENERATION> create_object RMS_109 { set_place_for_every_player number_of_objects 1 min_distance_to_players 0 max_distance_to_players 0 group_placement_radius 18 } create_object RMS_83 { set_place_for_every_player number_of_objects 1 min_distance_to_players 6 max_distance_to_players 6 group_placement_radius 3 } create_object RMS_550 { set_place_for_every_player match_player_civ number_of_objects 1 min_distance_to_players 7 max_distance_to_players 9 group_placement_radius 3 } )";
    struct Case { uint32_t seed,terrain; std::array<uint32_t,16> stages; };
    const Case cases[]={
        {1u, 742396127u, {3576039877u, 3664260213u, 3934188782u, 1644581822u, 680468115u, 3499084747u, 699229451u, 3423741803u, 1934467781u, 1842582091u, 4034547226u, 1198949132u, 1215750800u, 2041820795u, 806659457u, 344334813u}},
        {12345u, 2544610571u, {928842437u, 2400590430u, 2019525533u, 3663342404u, 2109202993u, 1277640200u, 3102986594u, 1621929952u, 1621929952u, 1969232334u, 131935967u, 3312367949u, 144266538u, 468505685u, 314624542u, 1240707077u}},
        {16160u, 3787531182u, {3332784581u, 1485044019u, 1060473042u, 4080570182u, 1714203709u, 3664350798u, 1212449678u, 426989960u, 1473983288u, 761724018u, 1376899582u, 990901592u, 86664909u, 2656006818u, 1831916887u, 1115432037u}},
    };
    const auto hash=[](const auto &values){uint32_t h=2166136261u;for(auto value:values)h=(h^uint32_t(value))*16777619u;return h;};
    for(const auto &c:cases) {
        swgb::RmsScript script;std::string error;
        check(script.load(0,[&](int32_t id,std::vector<uint8_t> &bytes){if(id)return false;bytes.assign(source.begin(),source.end());return true;},{},&error),error.c_str());
        swgb::RmsMap map;uint32_t rng=c.seed;
        check(swgb::generateRmsMap(script,{"FIXED_POSITIONS"},120,120,assets.dat(),rng,map,&error,2,{1,1}),error.c_str());
        check(hash(map.terrain)==c.terrain,"Original wall-entry terrain from native startup");
        swgb::RmsWallInput input;input.width=input.height=120;input.terrain=map.terrain;
        input.restrictions=assets.dat().terrainRestrictions[25].passableBuildableDmgMultiplier;
        auto append=[&](int unit,int owner,float x,float y) {
            const auto &u=assets.dat().civs[owner?1:0].units[unit];
            const float rx=u.collisionSize[assets.dat().footprintAxesSwapped?1:0],ry=u.collisionSize[assets.dat().footprintAxesSwapped?0:1];
            input.objects.push_back({owner,x,y,rx,ry,u.obstructionType,u.obstructionClass,u.cls});
            if(unit==109)input.commandCenters.push_back({owner,x,y,rx,ry});
        };
        for(const auto &o:map.cliffs)append(o.unit,0,o.x,o.y);
        for(const auto &o:map.scenery)if(o.alive)append(o.unitId,0,o.x,o.y);
        for(const auto &o:map.ownedObjects)append(o.unitId,o.owner,o.x,o.y);
        check(map.resourcesAfterOwned.empty(),"Wall boundary fixture precedes extra owned rows");
        swgb::RmsWallBoundary result;
        check(swgb::generateRmsWallBoundary(input,result,&error),error.c_str());
        std::vector<uint32_t> actual{hash(result.obstruction),hash(result.directions)};
        check(result.routeBoundaries.size()==8,"Original four boundary routes per owner");
        for(const auto &grid:result.routeBoundaries)actual.push_back(hash(grid));
        for(const auto *grid:{&result.reachableScratch,&result.reachableBoundary,&result.prunedScratch,&result.prunedBoundary,&result.segmentScratch})actual.push_back(hash(*grid));
        std::vector<int> segments;
        for(const auto &owner:result.segments)for(const auto &v:owner)
            segments.insert(segments.end(),{v.owner,int(v.active),v.fromX,v.fromY,v.toX,v.toY,v.length});
        actual.push_back(hash(segments));
        check(actual.size()==c.stages.size() && std::equal(actual.begin(),actual.end(),c.stages.begin()),"Original wall obstruction, routes, reachability, pruning and ordered segments");
        if(c.seed==1) {
            const auto original=hash(result.obstruction);auto invalid=input;
            invalid.objects[0].obstructionType=10;
            check(!swgb::generateRmsWallBoundary(invalid,result,&error) && !error.empty() && hash(result.obstruction)==original,"Unverified wall obstruction rejects atomically");
            invalid=input;invalid.objects[0].unitClass=29;
            check(!swgb::generateRmsWallBoundary(invalid,result,&error) && hash(result.obstruction)==original,"Unverified special wall class rejects atomically");
            invalid=input;invalid.terrain.pop_back();
            check(!swgb::generateRmsWallBoundary(invalid,result,&error) && hash(result.obstruction)==original,"Malformed wall terrain rejects atomically");
        }
    }
    std::puts("PASS: original wall boundary stages from native generated objects");
}
