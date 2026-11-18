// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstring>
// Fresh original derived4e7420/4df9b0 full RMS controls (2026-11-18), seeds1/12345.
// rms-text-full-water-forest21-current-*-oracle: actual parser, geometry and factories.
// Original symbolic terrain/object aliases carry the same IDs as these native literals.
// Replaces Nov14 authored-geometry goldens predating the shallow-water flat default.
// Includes every object pose, shoreline clearance and full persistent eligibility.
static void checkRmsForest21WaterResources(const swgb::dat::DatFile &data) {
    const std::string source=R"(<LAND_GENERATION> base_terrain 6 create_player_lands { terrain_type 6 base_size 3 number_of_tiles 60 }
<ELEVATION_GENERATION> create_elevation 2 { base_terrain 6 number_of_tiles 200 number_of_clumps 4 }
<TERRAIN_GENERATION> create_terrain 1 { base_terrain 6 number_of_tiles 700 number_of_clumps 5 spacing_to_other_terrain_types 1 set_avoid_player_start_areas } create_terrain 21 { base_terrain 6 number_of_tiles 100 number_of_clumps 4 spacing_to_other_terrain_types 1 set_avoid_player_start_areas }
<OBJECTS_GENERATION>
create_object 109 { set_place_for_every_player min_distance_to_players 0 max_distance_to_players 0 }
create_object 83 { set_place_for_every_player min_distance_to_players 6 max_distance_to_players 6 }
create_object 550 { set_place_for_every_player min_distance_to_players 7 max_distance_to_players 9 match_player_civ }
create_object 285 { number_of_objects 5 min_distance_to_players 25 temp_min_distance_group_placement 20 }
create_object 59 { set_place_for_every_player set_gaia_object_only set_tight_grouping number_of_objects 6 group_placement_radius 3 min_distance_to_players 10 max_distance_to_players 12 min_distance_group_placement 6 max_distance_to_other_zones 7 }
create_object 66 { set_place_for_every_player set_gaia_object_only set_tight_grouping number_of_objects 7 group_placement_radius 3 min_distance_to_players 12 max_distance_to_players 16 min_distance_group_placement 7 max_distance_to_other_zones 7 }
create_object 66 { set_place_for_every_player set_gaia_object_only set_tight_grouping number_of_objects 4 group_placement_radius 3 min_distance_to_players 18 max_distance_to_players 26 min_distance_group_placement 7 max_distance_to_other_zones 7 }
create_object 66 { set_place_for_every_player set_gaia_object_only set_tight_grouping number_of_objects 4 group_placement_radius 3 min_distance_to_players 25 max_distance_to_players 35 min_distance_group_placement 7 max_distance_to_other_zones 7 }
create_object 102 { set_place_for_every_player set_gaia_object_only set_tight_grouping number_of_objects 5 group_placement_radius 2 min_distance_to_players 14 max_distance_to_players 18 min_distance_group_placement 7 max_distance_to_other_zones 7 }
create_object 102 { set_place_for_every_player set_gaia_object_only set_tight_grouping number_of_objects 4 group_placement_radius 2 min_distance_to_players 20 max_distance_to_players 26 min_distance_group_placement 7 max_distance_to_other_zones 7 }
)";
    swgb::RmsScript script;std::string error;
    check(script.load(0,[&](int32_t id,std::vector<uint8_t>&out) {
        if(id)return false;out.assign(source.begin(),source.end());return true;
    },{},&error),error.c_str());
    auto hash=[](const std::vector<uint8_t>&bytes) {
        uint32_t value=2166136261u;for(auto b:bytes)value=(value^b)*16777619u;return value;
    };
    auto poseHash=[](const auto &objects) {
        uint32_t value=2166136261u;
        for(const auto &object:objects) {
            uint32_t x,y;std::memcpy(&x,&object.x,4);std::memcpy(&y,&object.y,4);
            for(uint32_t field:{uint32_t(object.unitId),x,y,uint32_t(object.facet)})
                for(unsigned shift=0;shift<32;shift+=8)value=(value^uint8_t(field>>shift))*16777619u;
        }
        return value;
    };
    struct Point {int unit;float x,y,heading;int facet;};
    {
        uint32_t rng=1u;swgb::RmsMap map;
        check(swgb::generateRmsMap(script,{},120,120,data,rng,map,&error,2,{1,1}),error.c_str());
        check(rng==2670305172u && map.ownedObjects.size()==10 && map.resourcesAfterOwned.size()==65 && map.scenery.size()==104,"Water/Forest21 resource final RNG/count");
        check(hash(map.terrain)==2264591383u,"Water/Forest21 original terrain");
        check(hash(map.elevation)==4089223887u,"Water/Forest21 original elevation");
        check(hash(map.slopes)==140112869u,"Water/Forest21 original slopes");
        check(poseHash(map.ownedObjects)==2546703060u && poseHash(map.scenery)==3149863926u,"Original full RMS owned/scenery poses in creation order");
        const Point expected[]={{285,94.5f,39.5f,0.0f,7},{285,68.5f,47.5f,0.0f,7},{285,22.5f,28.5f,0.0f,7},{285,100.5f,89.5f,0.0f,7},{285,116.5f,30.5f,0.0f,7},{59,71.5f,24.5f,0.0f,4},{59,70.5f,24.5f,0.0f,5},{59,70.5f,25.5f,0.0f,4},{59,71.5f,25.5f,0.0f,0},{59,71.5f,26.5f,0.0f,5},{59,72.5f,26.5f,0.0f,5},{59,48.5f,70.5f,0.0f,5},{59,48.5f,71.5f,0.0f,5},{59,47.5f,70.5f,0.0f,0},{59,46.5f,70.5f,0.0f,1},{59,46.5f,69.5f,0.0f,4},{59,46.5f,71.5f,0.0f,1},{66,62.5f,1.5f,2.5132741928100586f,3},{66,63.5f,1.5f,5.654866695404053f,8},{66,63.5f,2.5f,1.884955644607544f,2},{66,61.5f,1.5f,0.0f,9},{66,62.5f,0.5f,5.026548385620117f,7},{66,62.5f,2.5f,1.884955644607544f,2},{66,64.5f,1.5f,3.1415927410125732f,4},{66,44.5f,94.5f,1.2566370964050293f,1},{66,44.5f,93.5f,0.6283185482025146f,0},{66,44.5f,92.5f,0.6283185482025146f,0},{66,45.5f,93.5f,3.769911289215088f,5},{66,45.5f,92.5f,0.0f,9},{66,46.5f,92.5f,4.398229598999023f,6},{66,46.5f,93.5f,0.6283185482025146f,0},{66,90.5f,8.5f,5.026548385620117f,7},{66,90.5f,7.5f,5.654866695404053f,8},{66,89.5f,7.5f,5.026548385620117f,7},{66,89.5f,8.5f,0.6283185482025146f,0},{66,31.5f,61.5f,0.0f,9},{66,31.5f,62.5f,0.6283185482025146f,0},{66,32.5f,62.5f,2.5132741928100586f,3},{66,33.5f,62.5f,5.026548385620117f,7},{66,44.5f,10.5f,0.0f,9},{66,44.5f,9.5f,3.1415927410125732f,4},{66,43.5f,9.5f,1.2566370964050293f,1},{66,44.5f,8.5f,1.884955644607544f,2},{66,49.5f,49.5f,3.1415927410125732f,4},{66,50.5f,49.5f,0.6283185482025146f,0},{66,51.5f,49.5f,3.1415927410125732f,4},{66,51.5f,50.5f,0.6283185482025146f,0},{102,59.5f,29.5f,3.1415927410125732f,4},{102,59.5f,30.5f,1.2566370964050293f,1},{102,60.5f,30.5f,1.884955644607544f,2},{102,58.5f,30.5f,1.2566370964050293f,1},{102,59.5f,28.5f,0.6283185482025146f,0},{102,40.5f,67.5f,2.5132741928100586f,3},{102,40.5f,66.5f,5.026548385620117f,7},{102,39.5f,66.5f,5.654866695404053f,8},{102,41.5f,66.5f,2.5132741928100586f,3},{102,38.5f,66.5f,1.2566370964050293f,1},{102,76.5f,36.5f,5.654866695404053f,8},{102,76.5f,37.5f,4.398229598999023f,6},{102,76.5f,35.5f,1.884955644607544f,2},{102,76.5f,38.5f,4.398229598999023f,6},{102,63.5f,59.5f,1.884955644607544f,2},{102,63.5f,60.5f,4.398229598999023f,6},{102,62.5f,59.5f,3.1415927410125732f,4},{102,62.5f,58.5f,1.884955644607544f,2}};
        auto checkPoints=[&](const std::vector<swgb::RmsTimberPlacement> &points,size_t offset) {
            for(size_t i=0;i<points.size();++i) {
                const auto &a=points[i];const auto &e=expected[offset+i];
                check(a.unitId==e.unit && a.x==e.x && a.y==e.y && a.facet==e.facet && a.headingRadians==e.heading,"Water/Forest21 original resource pose");
            }
        };
        checkPoints(map.resourcesAfterOwned,0);
        std::vector<uint32_t> occupied;
        for(const auto &tree:map.scenery)if(tree.alive)occupied.push_back(uint32_t(tree.y)*120+uint32_t(tree.x));
        rng=802124419u;std::vector<swgb::RmsTimberPlacement> out;
        check(swgb::generateRmsHolocronPlacements(120,120,map.terrain,data,map.playerStarts,rng,out,&error,occupied),error.c_str());
        check(out.size()==5,"original holocron helper count");checkPoints(out,0);
        for(const auto &o:map.ownedObjects) {
            if(o.unitId==109)for(int y=int(o.y)-2;y<int(o.y)+2;++y)for(int x=int(o.x)-2;x<int(o.x)+2;++x)occupied.push_back(y*120+x);
            else occupied.push_back(uint32_t(o.y)*120+uint32_t(o.x));
        }
        for(const auto &o:out)occupied.push_back(uint32_t(o.y)*120+uint32_t(o.x));
        std::vector<uint8_t> eligibility;size_t offset=5;
        const int units[]={59,66,66,66,102,102},counts[]={6,7,4,4,5,4};
        const uint32_t states[]={2165959828u,1721049864u,3435158515u,1278257995u,2200510882u,4199171685u};
        for(int row=0;row<6;++row) {
            const auto distance=row==2?swgb::RmsResourceDistance::Medium:(row==3||row==5)?swgb::RmsResourceDistance::Far:swgb::RmsResourceDistance::Near;
            check(swgb::generateRmsGroupedResourcePlacements(120,120,map.terrain,data,units[row],{2,1,uint32_t(counts[row]),0,row>=4?2u:3u},rng,out,&error,occupied,16000000,map.playerStarts,distance,&eligibility),error.c_str());
            check(rng==states[row] && out.size()==size_t(counts[row]*2),"original resource row RNG/count");
            checkPoints(out,offset);offset+=out.size();
            for(const auto &o:out)occupied.push_back(uint32_t(o.y)*120+uint32_t(o.x));
        }
        check(hash(eligibility)==1870250032u,"original nonuniform persistent eligibility");
    }
    {
        uint32_t rng=12345u;swgb::RmsMap map;
        check(swgb::generateRmsMap(script,{},120,120,data,rng,map,&error,2,{1,1}),error.c_str());
        check(rng==360311268u && map.ownedObjects.size()==10 && map.resourcesAfterOwned.size()==65 && map.scenery.size()==105,"Water/Forest21 resource final RNG/count");
        check(hash(map.terrain)==456671907u,"Water/Forest21 original terrain");
        check(hash(map.elevation)==2763925479u,"Water/Forest21 original elevation");
        check(hash(map.slopes)==2713444239u,"Water/Forest21 original slopes");
        check(poseHash(map.ownedObjects)==3551480818u && poseHash(map.scenery)==519843192u,"Original full RMS owned/scenery poses in creation order");
        const Point expected[]={{285,112.5f,94.5f,0.0f,7},{285,70.5f,93.5f,0.0f,7},{285,102.5f,5.5f,0.0f,7},{285,11.5f,52.5f,0.0f,7},{285,63.5f,3.5f,0.0f,7},{59,4.5f,76.5f,0.0f,3},{59,4.5f,75.5f,0.0f,0},{59,5.5f,75.5f,0.0f,1},{59,4.5f,74.5f,0.0f,1},{59,3.5f,74.5f,0.0f,0},{59,2.5f,74.5f,0.0f,5},{59,78.5f,31.5f,0.0f,2},{59,78.5f,32.5f,0.0f,4},{59,78.5f,33.5f,0.0f,0},{59,78.5f,34.5f,0.0f,1},{59,79.5f,32.5f,0.0f,0},{59,77.5f,33.5f,0.0f,3},{66,22.5f,89.5f,3.769911289215088f,5},{66,22.5f,88.5f,3.769911289215088f,5},{66,21.5f,88.5f,4.398229598999023f,6},{66,23.5f,88.5f,5.026548385620117f,7},{66,21.5f,89.5f,0.6283185482025146f,0},{66,20.5f,89.5f,0.0f,9},{66,19.5f,89.5f,5.654866695404053f,8},{66,91.5f,55.5f,4.398229598999023f,6},{66,91.5f,56.5f,5.654866695404053f,8},{66,92.5f,56.5f,0.6283185482025146f,0},{66,93.5f,56.5f,0.0f,9},{66,93.5f,55.5f,5.654866695404053f,8},{66,93.5f,54.5f,1.2566370964050293f,1},{66,94.5f,56.5f,2.5132741928100586f,3},{66,18.5f,101.5f,1.2566370964050293f,1},{66,17.5f,101.5f,5.654866695404053f,8},{66,17.5f,100.5f,3.769911289215088f,5},{66,16.5f,100.5f,1.884955644607544f,2},{66,77.5f,64.5f,0.0f,9},{66,76.5f,64.5f,3.769911289215088f,5},{66,78.5f,64.5f,1.884955644607544f,2},{66,77.5f,65.5f,1.884955644607544f,2},{66,0.5f,43.5f,0.0f,9},{66,1.5f,43.5f,2.5132741928100586f,3},{66,1.5f,42.5f,0.0f,9},{66,0.5f,42.5f,4.398229598999023f,6},{66,50.5f,9.5f,5.026548385620117f,7},{66,51.5f,9.5f,5.654866695404053f,8},{66,52.5f,9.5f,0.0f,9},{66,51.5f,10.5f,2.5132741928100586f,3},{102,6.5f,91.5f,1.2566370964050293f,1},{102,5.5f,91.5f,5.026548385620117f,7},{102,5.5f,90.5f,3.1415927410125732f,4},{102,4.5f,90.5f,1.884955644607544f,2},{102,5.5f,89.5f,3.1415927410125732f,4},{102,70.5f,55.5f,3.769911289215088f,5},{102,70.5f,56.5f,3.1415927410125732f,4},{102,71.5f,56.5f,0.6283185482025146f,0},{102,72.5f,56.5f,5.026548385620117f,7},{102,72.5f,57.5f,0.6283185482025146f,0},{102,8.5f,57.5f,5.026548385620117f,7},{102,9.5f,57.5f,3.1415927410125732f,4},{102,9.5f,58.5f,5.654866695404053f,8},{102,8.5f,58.5f,3.1415927410125732f,4},{102,58.5f,35.5f,5.654866695404053f,8},{102,58.5f,36.5f,1.2566370964050293f,1},{102,58.5f,34.5f,0.0f,9},{102,58.5f,33.5f,0.0f,9}};
        auto checkPoints=[&](const std::vector<swgb::RmsTimberPlacement> &points,size_t offset) {
            for(size_t i=0;i<points.size();++i) {
                const auto &a=points[i];const auto &e=expected[offset+i];
                check(a.unitId==e.unit && a.x==e.x && a.y==e.y && a.facet==e.facet && a.headingRadians==e.heading,"Water/Forest21 original resource pose");
            }
        };
        checkPoints(map.resourcesAfterOwned,0);
        std::vector<uint32_t> occupied;
        for(const auto &tree:map.scenery)if(tree.alive)occupied.push_back(uint32_t(tree.y)*120+uint32_t(tree.x));
        rng=3220867577u;std::vector<swgb::RmsTimberPlacement> out;
        check(swgb::generateRmsHolocronPlacements(120,120,map.terrain,data,map.playerStarts,rng,out,&error,occupied),error.c_str());
        check(out.size()==5,"original holocron helper count");checkPoints(out,0);
        for(const auto &o:map.ownedObjects) {
            if(o.unitId==109)for(int y=int(o.y)-2;y<int(o.y)+2;++y)for(int x=int(o.x)-2;x<int(o.x)+2;++x)occupied.push_back(y*120+x);
            else occupied.push_back(uint32_t(o.y)*120+uint32_t(o.x));
        }
        for(const auto &o:out)occupied.push_back(uint32_t(o.y)*120+uint32_t(o.x));
        std::vector<uint8_t> eligibility;size_t offset=5;
        const int units[]={59,66,66,66,102,102},counts[]={6,7,4,4,5,4};
        const uint32_t states[]={3422471330u,4242641121u,1442532267u,3456787222u,1324166253u,1729905397u};
        for(int row=0;row<6;++row) {
            const auto distance=row==2?swgb::RmsResourceDistance::Medium:(row==3||row==5)?swgb::RmsResourceDistance::Far:swgb::RmsResourceDistance::Near;
            check(swgb::generateRmsGroupedResourcePlacements(120,120,map.terrain,data,units[row],{2,1,uint32_t(counts[row]),0,row>=4?2u:3u},rng,out,&error,occupied,16000000,map.playerStarts,distance,&eligibility),error.c_str());
            check(rng==states[row] && out.size()==size_t(counts[row]*2),"original resource row RNG/count");
            checkPoints(out,offset);offset+=out.size();
            for(const auto &o:out)occupied.push_back(uint32_t(o.y)*120+uint32_t(o.x));
        }
        check(hash(eligibility)==2652320376u,"original nonuniform persistent eligibility");
    }
}
