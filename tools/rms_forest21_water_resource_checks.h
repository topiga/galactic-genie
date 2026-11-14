// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Actual original 413/109/83/550/285/59/66/102 creation on native authored
// geometry, with real unit headers, terrain zones and controller maxima119.
// Includes original shoreline clearance rejections and full eligibility raster.
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
    },{},&error),error);
    auto hash=[](const std::vector<uint8_t>&bytes) {
        uint32_t value=2166136261u;for(auto b:bytes)value=(value^b)*16777619u;return value;
    };
    struct Point {int unit;float x,y,heading;int facet;};
    {
        uint32_t rng=1u;swgb::RmsMap map;
        check(swgb::generateRmsMap(script,{},120,120,data,rng,map,&error,2,{1,1}),error);
        check(rng==3135654677u && map.ownedObjects.size()==10 && map.resourcesAfterOwned.size()==65 && map.scenery.size()==103,"Water/Forest21 resource final RNG/count");
        check(hash(map.terrain)==226656437u,"Water/Forest21 original terrain");
        check(hash(map.elevation)==4089223887u,"Water/Forest21 original elevation");
        check(hash(map.slopes)==140112869u,"Water/Forest21 original slopes");
        const Point expected[]={{285,12.5f,19.5f,0.0f,7},{285,78.5f,55.5f,0.0f,7},{285,14.5f,52.5f,0.0f,7},{285,82.5f,79.5f,0.0f,7},{285,18.5f,80.5f,0.0f,7},{59,82.5f,17.5f,0.0f,1},{59,82.5f,18.5f,0.0f,3},{59,83.5f,17.5f,0.0f,1},{59,81.5f,17.5f,0.0f,3},{59,81.5f,18.5f,0.0f,4},{59,80.5f,18.5f,0.0f,3},{59,40.5f,92.5f,0.0f,5},{59,41.5f,92.5f,0.0f,1},{59,41.5f,93.5f,0.0f,3},{59,39.5f,92.5f,0.0f,2},{59,42.5f,92.5f,0.0f,0},{59,39.5f,93.5f,0.0f,1},{66,72.5f,0.5f,5.654866695404053f,8},{66,73.5f,0.5f,0.6283185482025146f,0},{66,72.5f,1.5f,2.5132741928100586f,3},{66,71.5f,0.5f,5.026548385620117f,7},{66,70.5f,0.5f,2.5132741928100586f,3},{66,69.5f,0.5f,5.026548385620117f,7},{66,70.5f,1.5f,3.1415927410125732f,4},{66,30.5f,93.5f,1.884955644607544f,2},{66,29.5f,93.5f,1.2566370964050293f,1},{66,29.5f,92.5f,0.0f,9},{66,28.5f,92.5f,1.884955644607544f,2},{66,28.5f,93.5f,3.769911289215088f,5},{66,27.5f,92.5f,5.026548385620117f,7},{66,28.5f,91.5f,0.6283185482025146f,0},{66,91.5f,27.5f,3.769911289215088f,5},{66,90.5f,27.5f,0.0f,9},{66,90.5f,28.5f,2.5132741928100586f,3},{66,89.5f,28.5f,1.884955644607544f,2},{66,22.5f,71.5f,4.398229598999023f,6},{66,22.5f,70.5f,5.026548385620117f,7},{66,23.5f,70.5f,3.1415927410125732f,4},{66,24.5f,70.5f,5.026548385620117f,7},{66,47.5f,30.5f,2.5132741928100586f,3},{66,46.5f,30.5f,0.0f,9},{66,48.5f,30.5f,1.884955644607544f,2},{66,49.5f,30.5f,1.884955644607544f,2},{66,47.5f,54.5f,2.5132741928100586f,3},{66,46.5f,54.5f,5.654866695404053f,8},{66,46.5f,53.5f,4.398229598999023f,6},{66,45.5f,53.5f,0.6283185482025146f,0},{102,83.5f,0.5f,4.398229598999023f,6},{102,84.5f,0.5f,1.2566370964050293f,1},{102,85.5f,0.5f,1.884955644607544f,2},{102,85.5f,1.5f,1.2566370964050293f,1},{102,82.5f,0.5f,0.0f,9},{102,31.5f,71.5f,3.1415927410125732f,4},{102,32.5f,71.5f,5.026548385620117f,7},{102,33.5f,71.5f,1.884955644607544f,2},{102,31.5f,70.5f,0.6283185482025146f,0},{102,30.5f,70.5f,1.884955644607544f,2},{102,52.5f,1.5f,2.5132741928100586f,3},{102,51.5f,1.5f,1.884955644607544f,2},{102,51.5f,2.5f,0.6283185482025146f,0},{102,50.5f,2.5f,0.6283185482025146f,0},{102,41.5f,103.5f,1.884955644607544f,2},{102,42.5f,103.5f,0.6283185482025146f,0},{102,42.5f,104.5f,1.884955644607544f,2},{102,42.5f,102.5f,1.884955644607544f,2}};
        auto checkPoints=[&](const std::vector<swgb::RmsTimberPlacement> &points,size_t offset) {
            for(size_t i=0;i<points.size();++i) {
                const auto &a=points[i];const auto &e=expected[offset+i];
                check(a.unitId==e.unit && a.x==e.x && a.y==e.y && a.facet==e.facet && a.headingRadians==e.heading,"Water/Forest21 original resource pose");
            }
        };
        checkPoints(map.resourcesAfterOwned,0);
        std::vector<uint32_t> occupied;
        for(const auto &tree:map.scenery)if(tree.alive)occupied.push_back(uint32_t(tree.y)*120+uint32_t(tree.x));
        rng=2981612379u;std::vector<swgb::RmsTimberPlacement> out;
        check(swgb::generateRmsHolocronPlacements(120,120,map.terrain,data,map.playerStarts,rng,out,&error,occupied),error);
        check(out.size()==5,"original holocron helper count");checkPoints(out,0);
        for(const auto &o:map.ownedObjects) {
            if(o.unitId==109)for(int y=int(o.y)-2;y<int(o.y)+2;++y)for(int x=int(o.x)-2;x<int(o.x)+2;++x)occupied.push_back(y*120+x);
            else occupied.push_back(uint32_t(o.y)*120+uint32_t(o.x));
        }
        for(const auto &o:out)occupied.push_back(uint32_t(o.y)*120+uint32_t(o.x));
        std::vector<uint8_t> eligibility;size_t offset=5;
        const int units[]={59,66,66,66,102,102},counts[]={6,7,4,4,5,4};
        const uint32_t states[]={3541210060u,1677541948u,3956775486u,4053341110u,3276564539u,1895164218u};
        for(int row=0;row<6;++row) {
            const auto distance=row==2?swgb::RmsResourceDistance::Medium:(row==3||row==5)?swgb::RmsResourceDistance::Far:swgb::RmsResourceDistance::Near;
            check(swgb::generateRmsGroupedResourcePlacements(120,120,map.terrain,data,units[row],{2,1,uint32_t(counts[row]),0,row>=4?2u:3u},rng,out,&error,occupied,16000000,map.playerStarts,distance,&eligibility),error);
            check(rng==states[row] && out.size()==size_t(counts[row]*2),"original resource row RNG/count");
            checkPoints(out,offset);offset+=out.size();
            for(const auto &o:out)occupied.push_back(uint32_t(o.y)*120+uint32_t(o.x));
        }
        check(hash(eligibility)==3873339827u,"original nonuniform persistent eligibility");
    }
    {
        uint32_t rng=12345u;swgb::RmsMap map;
        check(swgb::generateRmsMap(script,{},120,120,data,rng,map,&error,2,{1,1}),error);
        check(rng==2401144792u && map.ownedObjects.size()==10 && map.resourcesAfterOwned.size()==65 && map.scenery.size()==104,"Water/Forest21 resource final RNG/count");
        check(hash(map.terrain)==3791020342u,"Water/Forest21 original terrain");
        check(hash(map.elevation)==2763925479u,"Water/Forest21 original elevation");
        check(hash(map.slopes)==2713444239u,"Water/Forest21 original slopes");
        const Point expected[]={{285,93.5f,0.5f,0.0f,7},{285,78.5f,90.5f,0.0f,7},{285,9.5f,103.5f,0.0f,7},{285,52.5f,46.5f,0.0f,7},{285,7.5f,52.5f,0.0f,7},{59,15.5f,67.5f,0.0f,3},{59,15.5f,68.5f,0.0f,0},{59,14.5f,68.5f,0.0f,3},{59,16.5f,68.5f,0.0f,0},{59,17.5f,68.5f,0.0f,3},{59,17.5f,67.5f,0.0f,4},{59,87.5f,31.5f,0.0f,4},{59,87.5f,32.5f,0.0f,2},{59,87.5f,30.5f,0.0f,4},{59,86.5f,30.5f,0.0f,4},{59,86.5f,31.5f,0.0f,4},{59,85.5f,31.5f,0.0f,0},{66,27.5f,89.5f,1.2566370964050293f,1},{66,27.5f,88.5f,1.2566370964050293f,1},{66,27.5f,87.5f,3.1415927410125732f,4},{66,26.5f,87.5f,3.769911289215088f,5},{66,25.5f,87.5f,2.5132741928100586f,3},{66,25.5f,88.5f,0.0f,9},{66,24.5f,87.5f,4.398229598999023f,6},{66,81.5f,55.5f,1.884955644607544f,2},{66,81.5f,54.5f,3.1415927410125732f,4},{66,80.5f,54.5f,2.5132741928100586f,3},{66,81.5f,53.5f,0.0f,9},{66,80.5f,53.5f,3.1415927410125732f,4},{66,80.5f,52.5f,2.5132741928100586f,3},{66,81.5f,52.5f,1.2566370964050293f,1},{66,5.5f,100.5f,0.0f,9},{66,5.5f,99.5f,0.0f,9},{66,6.5f,99.5f,0.6283185482025146f,0},{66,6.5f,100.5f,0.6283185482025146f,0},{66,62.5f,48.5f,2.5132741928100586f,3},{66,62.5f,49.5f,0.6283185482025146f,0},{66,63.5f,48.5f,4.398229598999023f,6},{66,64.5f,48.5f,0.6283185482025146f,0},{66,29.5f,44.5f,1.2566370964050293f,1},{66,28.5f,44.5f,3.769911289215088f,5},{66,27.5f,44.5f,4.398229598999023f,6},{66,27.5f,45.5f,0.0f,9},{66,111.5f,46.5f,5.026548385620117f,7},{66,111.5f,45.5f,1.884955644607544f,2},{66,110.5f,45.5f,2.5132741928100586f,3},{66,112.5f,45.5f,5.026548385620117f,7},{102,0.5f,78.5f,1.884955644607544f,2},{102,0.5f,79.5f,3.1415927410125732f,4},{102,0.5f,80.5f,3.1415927410125732f,4},{102,1.5f,78.5f,4.398229598999023f,6},{102,2.5f,78.5f,3.769911289215088f,5},{102,97.5f,43.5f,2.5132741928100586f,3},{102,96.5f,43.5f,5.026548385620117f,7},{102,96.5f,42.5f,1.884955644607544f,2},{102,95.5f,43.5f,3.1415927410125732f,4},{102,97.5f,44.5f,1.2566370964050293f,1},{102,33.5f,99.5f,0.6283185482025146f,0},{102,33.5f,98.5f,0.6283185482025146f,0},{102,33.5f,97.5f,1.2566370964050293f,1},{102,32.5f,98.5f,5.026548385620117f,7},{102,56.5f,26.5f,1.884955644607544f,2},{102,55.5f,26.5f,1.884955644607544f,2},{102,55.5f,27.5f,5.654866695404053f,8},{102,55.5f,28.5f,4.398229598999023f,6}};
        auto checkPoints=[&](const std::vector<swgb::RmsTimberPlacement> &points,size_t offset) {
            for(size_t i=0;i<points.size();++i) {
                const auto &a=points[i];const auto &e=expected[offset+i];
                check(a.unitId==e.unit && a.x==e.x && a.y==e.y && a.facet==e.facet && a.headingRadians==e.heading,"Water/Forest21 original resource pose");
            }
        };
        checkPoints(map.resourcesAfterOwned,0);
        std::vector<uint32_t> occupied;
        for(const auto &tree:map.scenery)if(tree.alive)occupied.push_back(uint32_t(tree.y)*120+uint32_t(tree.x));
        rng=1686193382u;std::vector<swgb::RmsTimberPlacement> out;
        check(swgb::generateRmsHolocronPlacements(120,120,map.terrain,data,map.playerStarts,rng,out,&error,occupied),error);
        check(out.size()==5,"original holocron helper count");checkPoints(out,0);
        for(const auto &o:map.ownedObjects) {
            if(o.unitId==109)for(int y=int(o.y)-2;y<int(o.y)+2;++y)for(int x=int(o.x)-2;x<int(o.x)+2;++x)occupied.push_back(y*120+x);
            else occupied.push_back(uint32_t(o.y)*120+uint32_t(o.x));
        }
        for(const auto &o:out)occupied.push_back(uint32_t(o.y)*120+uint32_t(o.x));
        std::vector<uint8_t> eligibility;size_t offset=5;
        const int units[]={59,66,66,66,102,102},counts[]={6,7,4,4,5,4};
        const uint32_t states[]={582344033u,3953461002u,2605059885u,1067849797u,85973489u,2888241401u};
        for(int row=0;row<6;++row) {
            const auto distance=row==2?swgb::RmsResourceDistance::Medium:(row==3||row==5)?swgb::RmsResourceDistance::Far:swgb::RmsResourceDistance::Near;
            check(swgb::generateRmsGroupedResourcePlacements(120,120,map.terrain,data,units[row],{2,1,uint32_t(counts[row]),0,row>=4?2u:3u},rng,out,&error,occupied,16000000,map.playerStarts,distance,&eligibility),error);
            check(rng==states[row] && out.size()==size_t(counts[row]*2),"original resource row RNG/count");
            checkPoints(out,offset);offset+=out.size();
            for(const auto &o:out)occupied.push_back(uint32_t(o.y)*120+uint32_t(o.x));
        }
        check(hash(eligibility)==1991804077u,"original nonuniform persistent eligibility");
    }
}
