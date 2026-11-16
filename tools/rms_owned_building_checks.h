// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Actual4de830/5cfad0, captured Empire87/70/50/79; local rms-owned{87,70} findings, 2026-11-16.
static void ownedRmsBuildingChecks(swgb::Assets &assets,swgb::SoftRenderer &renderer) {
    const std::string baseline=R"(#const RMS_6 6
#const RMS_21 21
#const RMS_109 109
#const RMS_83 83
#const RMS_550 550
#const RMS_87 87
<LAND_GENERATION> base_terrain RMS_6 create_player_lands { terrain_type RMS_6 base_size 13 number_of_tiles 7200 } <CLIFF_GENERATION> <TERRAIN_GENERATION> create_terrain RMS_21 { base_terrain RMS_6 number_of_tiles 6000 number_of_clumps 60 spacing_to_other_terrain_types 2 clumping_factor 20 set_avoid_player_start_areas } <OBJECTS_GENERATION> create_object RMS_109 { set_place_for_every_player number_of_objects 1 min_distance_to_players 0 max_distance_to_players 0 group_placement_radius 18 } create_object RMS_83 { set_place_for_every_player number_of_objects 1 min_distance_to_players 6 max_distance_to_players 6 group_placement_radius 3 } create_object RMS_550 { set_place_for_every_player match_player_civ number_of_objects 1 min_distance_to_players 7 max_distance_to_players 9 group_placement_radius 3 } create_object RMS_87 { set_place_for_every_player number_of_objects 1 min_distance_to_players 8 max_distance_to_players 14 }
)";
    const std::string extra="create_object RMS_87 { set_place_for_every_player number_of_objects 1 min_distance_to_players 8 max_distance_to_players 14 }";
    struct Case { uint32_t seed,rows,count,rng,objects,terrain,heights,slopes; bool onCenter=false; int unit=87; bool loose=false; int groups=1,mixed=0,prefabs1=4,prefabs2=4; };
    const Case cases[]={
        {1u,1u,5989u,1745024857u,1456092772u,3952391958u,1017236165u,1017236165u},
        {12345u,1u,5973u,3433587427u,3999233898u,2021000264u,1017236165u,1017236165u},
        {16160u,1u,6065u,3908054248u,1674239946u,2635766779u,1017236165u,1017236165u},
        {1u,2u,5991u,2727130514u,258802950u,3531810289u,1017236165u,1017236165u},
        {1u,1u,5989u,524140293u,2363215044u,742396127u,1017236165u,1017236165u,true},
        {1u, 1u, 5989u, 1040066035u, 683316884u, 1401291587u, 1017236165u, 1017236165u, false, 70, false},
        {12345u, 1u, 5973u, 3797195514u, 1950773600u, 219316191u, 1017236165u, 1017236165u, false, 70, false},
        {16160u, 1u, 6065u, 1428071604u, 2223973520u, 2447633602u, 1017236165u, 1017236165u, false, 70, false},
        {1u, 2u, 5991u, 2617341845u, 2276589332u, 639547895u, 1017236165u, 1017236165u, false, 70, false},
        {1u, 1u, 5995u, 2723706306u, 189115598u, 4015094335u, 1017236165u, 1017236165u, false, 70, true},
        {12345u, 1u, 5979u, 2206864911u, 657115638u, 184374139u, 1017236165u, 1017236165u, false, 70, true},
        {16160u, 1u, 6071u, 1753002939u, 2754205972u, 602091358u, 1017236165u, 1017236165u, false, 70, true},
        {1u, 1u, 5989u, 1626247642u, 2041514426u, 1526155850u, 1017236165u, 1017236165u, false, 50},
        {12345u, 1u, 5973u, 2524091778u, 1320807386u, 2529876188u, 1017236165u, 1017236165u, false, 50, false, 1},
        {16160u, 1u, 6065u, 2983090062u, 1103799056u, 4191456395u, 1017236165u, 1017236165u, false, 50, false, 1},
        {1u, 1u, 5997u, 1477568208u, 1200157676u, 615177112u, 1017236165u, 1017236165u, false, 50, false, 5},
        {1u, 1u, 5987u, 2551954224u, 1372774334u, 742396127u, 1017236165u, 1017236165u, true, 50},
        {1u, 1u, 5989u, 3117704239u, 1785968726u, 2071255114u, 1017236165u, 1017236165u, false, 79, false, 1},
        {12345u, 1u, 5973u, 2132019962u, 374746908u, 1538211736u, 1017236165u, 1017236165u, false, 79, false, 1},
        {16160u, 1u, 6065u, 510229448u, 2944556398u, 972306119u, 1017236165u, 1017236165u, false, 79, false, 1},
        {1u, 1u, 5995u, 313705913u, 1879166474u, 1580653879u, 1017236165u, 1017236165u, false, 79, false, 4},
        {12345u, 1u, 5981u, 3311600017u, 2243955132u, 929382136u, 1017236165u, 1017236165u, false, 50, false, 5},
        {16160u, 1u, 6073u, 1320910692u, 1198529154u, 735700569u, 1017236165u, 1017236165u, false, 50, false, 5},
        {1u, 1u, 6007u, 659122895u, 2691377374u, 1007834865u, 1017236165u, 1017236165u, false, 87, false, 1, true},
        {12345u, 1u, 5991u, 1730317561u, 1728207720u, 714626033u, 1017236165u, 1017236165u, false, 87, false, 1, true},
        {16160u, 1u, 6083u, 4222011319u, 3212885382u, 1852023360u, 1017236165u, 1017236165u, false, 87, false, 1, true},
        {1u, 1u, 6014u, 332111031u, 1997371185u, 4155637983u, 1017236165u, 1017236165u, false, 87, false, 1, 2, 4, 3},
        {12345u, 1u, 5999u, 2037640229u, 1431426170u, 4001648909u, 1017236165u, 1017236165u, false, 87, false, 1, 2, 4, 4},
        {16160u, 1u, 6091u, 4185843122u, 2633264522u, 2270325818u, 1017236165u, 1017236165u, false, 87, false, 1, 2, 4, 4},
    };
    struct Building { uint32_t seed,rows,id,owner; float x,y; bool onCenter=false; int unit=87; bool loose=false; int groups=1; };
    const Building buildings[]={
        {1,1,6020,1, 90.5f, 25.5f},
        {1,1,6021,2, 24.5f, 102.5f},
        {12345,1,6000,1, 29.5f, 89.5f},
        {12345,1,6001,2, 86.5f, 23.5f},
        {16160,1,6092,1, 16.5f, 28.5f},
        {16160,1,6093,2, 45.5f, 87.5f},
        {1,2,6020,1, 90.5f, 25.5f},
        {1,2,6021,2, 24.5f, 102.5f},
        {1,2,6022,1, 89.5f, 45.5f},
        {1,2,6023,2, 28.5f, 101.5f},
        {1,1,6020,1, 96.5f, 34.5f,true},
        {1,1,6021,2, 37.5f, 92.5f,true},
        {1, 1, 6020, 1, 106.0f, 34.0f, false, 70, false},
        {1, 1, 6021, 2, 47.0f, 98.0f, false, 70, false},
        {12345, 1, 6000, 1, 9.0f, 76.0f, false, 70, false},
        {12345, 1, 6001, 2, 83.0f, 24.0f, false, 70, false},
        {16160, 1, 6092, 1, 7.0f, 46.0f, false, 70, false},
        {16160, 1, 6093, 2, 31.0f, 95.0f, false, 70, false},
        {1, 2, 6020, 1, 106.0f, 34.0f, false, 70, false},
        {1, 2, 6021, 2, 47.0f, 98.0f, false, 70, false},
        {1, 2, 6022, 1, 84.0f, 42.0f, false, 70, false},
        {1, 2, 6023, 2, 32.0f, 102.0f, false, 70, false},
        {1, 1, 6020, 1, 107.0f, 35.0f, false, 70, true},
        {1, 1, 6021, 1, 105.0f, 34.0f, false, 70, true},
        {1, 1, 6022, 1, 103.0f, 35.0f, false, 70, true},
        {1, 1, 6023, 1, 104.0f, 32.0f, false, 70, true},
        {1, 1, 6024, 2, 22.0f, 95.0f, false, 70, true},
        {1, 1, 6025, 2, 24.0f, 95.0f, false, 70, true},
        {1, 1, 6026, 2, 25.0f, 97.0f, false, 70, true},
        {1, 1, 6027, 2, 26.0f, 95.0f, false, 70, true},
        {12345, 1, 6000, 1, 10.0f, 74.0f, false, 70, true},
        {12345, 1, 6001, 1, 6.0f, 76.0f, false, 70, true},
        {12345, 1, 6002, 1, 7.0f, 73.0f, false, 70, true},
        {12345, 1, 6003, 1, 12.0f, 73.0f, false, 70, true},
        {12345, 1, 6004, 2, 80.0f, 4.0f, false, 70, true},
        {12345, 1, 6005, 2, 78.0f, 1.0f, false, 70, true},
        {12345, 1, 6006, 2, 80.0f, 2.0f, false, 70, true},
        {12345, 1, 6007, 2, 82.0f, 1.0f, false, 70, true},
        {16160, 1, 6092, 1, 8.0f, 46.0f, false, 70, true},
        {16160, 1, 6093, 1, 7.0f, 43.0f, false, 70, true},
        {16160, 1, 6094, 1, 9.0f, 43.0f, false, 70, true},
        {16160, 1, 6095, 1, 6.0f, 45.0f, false, 70, true},
        {16160, 1, 6096, 2, 32.0f, 94.0f, false, 70, true},
        {16160, 1, 6097, 2, 29.0f, 93.0f, false, 70, true},
        {16160, 1, 6098, 2, 30.0f, 95.0f, false, 70, true},
        {16160, 1, 6099, 2, 31.0f, 92.0f, false, 70, true},
        {1, 1, 6020, 1, 98.5f, 37.5f, false, 50},
        {1, 1, 6021, 2, 32.5f, 89.5f, false, 50},
        {12345, 1, 6000, 1, 21.5f, 79.5f, false, 50, false, 1},
        {12345, 1, 6001, 2, 88.5f, 9.5f, false, 50, false, 1},
        {16160, 1, 6092, 1, 14.5f, 36.5f, false, 50, false, 1},
        {16160, 1, 6093, 2, 45.5f, 98.5f, false, 50, false, 1},
        {1, 1, 6020, 1, 98.5f, 37.5f, false, 50, false, 5},
        {1, 1, 6021, 1, 98.5f, 29.5f, false, 50, false, 5},
        {1, 1, 6022, 1, 93.5f, 29.5f, false, 50, false, 5},
        {1, 1, 6023, 1, 91.5f, 36.5f, false, 50, false, 5},
        {1, 1, 6024, 1, 101.5f, 29.5f, false, 50, false, 5},
        {1, 1, 6025, 2, 32.5f, 93.5f, false, 50, false, 5},
        {1, 1, 6026, 2, 33.5f, 90.5f, false, 50, false, 5},
        {1, 1, 6027, 2, 37.5f, 87.5f, false, 50, false, 5},
        {1, 1, 6028, 2, 35.5f, 95.5f, false, 50, false, 5},
        {1, 1, 6029, 2, 39.5f, 95.5f, false, 50, false, 5},
        {1, 1, 6020, 1, 84.5f, 42.5f, false, 79, false, 1},
        {1, 1, 6021, 2, 42.5f, 102.5f, false, 79, false, 1},
        {12345, 1, 6000, 1, 25.5f, 94.5f, false, 79, false, 1},
        {12345, 1, 6001, 2, 73.5f, 2.5f, false, 79, false, 1},
        {16160, 1, 6092, 1, 22.5f, 47.5f, false, 79, false, 1},
        {16160, 1, 6093, 2, 47.5f, 106.5f, false, 79, false, 1},
        {1, 1, 6020, 1, 84.5f, 42.5f, false, 79, false, 4},
        {1, 1, 6021, 1, 96.5f, 23.5f, false, 79, false, 4},
        {1, 1, 6022, 1, 107.5f, 33.5f, false, 79, false, 4},
        {1, 1, 6023, 1, 105.5f, 44.5f, false, 79, false, 4},
        {1, 1, 6024, 2, 24.5f, 97.5f, false, 79, false, 4},
        {1, 1, 6025, 2, 40.5f, 103.5f, false, 79, false, 4},
        {1, 1, 6026, 2, 48.5f, 92.5f, false, 79, false, 4},
        {1, 1, 6027, 2, 33.5f, 79.5f, false, 79, false, 4},
        {12345, 1, 6000, 1, 21.5f, 79.5f, false, 50, false, 5},
        {12345, 1, 6001, 1, 17.5f, 81.5f, false, 50, false, 5},
        {12345, 1, 6002, 1, 21.5f, 86.5f, false, 50, false, 5},
        {12345, 1, 6003, 1, 24.5f, 79.5f, false, 50, false, 5},
        {12345, 1, 6004, 1, 18.5f, 86.5f, false, 50, false, 5},
        {12345, 1, 6005, 2, 86.5f, 10.5f, false, 50, false, 5},
        {12345, 1, 6006, 2, 89.5f, 17.5f, false, 50, false, 5},
        {12345, 1, 6007, 2, 83.5f, 17.5f, false, 50, false, 5},
        {12345, 1, 6008, 2, 82.5f, 9.5f, false, 50, false, 5},
        {12345, 1, 6009, 2, 89.5f, 14.5f, false, 50, false, 5},
        {16160, 1, 6092, 1, 14.5f, 36.5f, false, 50, false, 5},
        {16160, 1, 6093, 1, 17.5f, 39.5f, false, 50, false, 5},
        {16160, 1, 6094, 1, 22.5f, 39.5f, false, 50, false, 5},
        {16160, 1, 6095, 1, 18.5f, 32.5f, false, 50, false, 5},
        {16160, 1, 6096, 1, 14.5f, 39.5f, false, 50, false, 5},
        {16160, 1, 6097, 2, 44.5f, 99.5f, false, 50, false, 5},
        {16160, 1, 6098, 2, 39.5f, 91.5f, false, 50, false, 5},
        {16160, 1, 6099, 2, 37.5f, 95.5f, false, 50, false, 5},
        {16160, 1, 6100, 2, 40.5f, 99.5f, false, 50, false, 5},
        {16160, 1, 6101, 2, 37.5f, 99.5f, false, 50, false, 5},
    };
    for(const auto &c:cases) {
        std::string source=baseline+(c.rows==2?extra:"");
        if(c.mixed) {
            const auto pos=source.rfind("create_object RMS_87");
            const auto troop=source.substr(pos);source.resize(pos);
            if(c.mixed==2)source+="create_object RMS_79 { set_place_for_every_player number_of_objects 4 min_distance_to_players 10 max_distance_to_players 13 min_distance_group_placement 6 } ";
            else source+=troop;
            source+="create_object RMS_70 { set_place_for_every_player number_of_objects 4 set_loose_grouping min_distance_to_players 10 max_distance_to_players 12 } ";
            if(c.mixed==2)source+=troop;
            source+="create_object RMS_50 { set_place_for_every_player number_of_objects 5 min_distance_to_players 2 max_distance_to_players 5 }";
            source="#const RMS_79 79\n#const RMS_70 70\n#const RMS_50 50\n"+source;
        }
        const bool prefab=c.unit==70,farm=c.unit==50,tower=c.unit==79;
        const int foundation=farm?7:27;
        const int quantity=c.mixed?10:(farm && c.onCenter)?0:c.rows*(c.loose?4:c.groups);
        if(c.unit!=87) {
            const auto token="RMS_"+std::to_string(c.unit);
            for(size_t pos=0;(pos=source.find("RMS_87",pos))!=std::string::npos;pos+=6)source.replace(pos,6,token);
            source.replace(source.find("#const "+token+" 87"),16,"#const "+token+" "+std::to_string(c.unit));
            const std::string range="min_distance_to_players 8 max_distance_to_players 14";
            for(size_t pos=0;(pos=source.find(range,pos))!=std::string::npos;) {
                source.replace(pos,range.size(),tower?"min_distance_to_players 10 max_distance_to_players 13 min_distance_group_placement 6":farm?"min_distance_to_players 2 max_distance_to_players 5":"min_distance_to_players 10 max_distance_to_players 12");
                ++pos;
            }
            if(c.loose) {
                const auto pos=source.rfind("number_of_objects 1");
                source.replace(pos,19,"number_of_objects 4 set_loose_grouping");
            }
        }
        if(c.groups!=1) {
            const auto pos=source.rfind("number_of_objects 1");
            source.replace(pos,19,"number_of_objects "+std::to_string(c.groups));
        }
        if(c.onCenter) {
            const std::string range=farm?"min_distance_to_players 2 max_distance_to_players 5":"min_distance_to_players 8 max_distance_to_players 14";
            source.replace(source.find(range),range.size(),"min_distance_to_players 0 max_distance_to_players 0");
        }
        swgb::RmsScript script;std::string error;
        check(script.load(0,[&](int32_t id,std::vector<uint8_t> &bytes){if(id)return false;bytes.assign(source.begin(),source.end());return true;},{},&error),error.c_str());
        swgb::RmsMap map;uint32_t rng=c.seed;
        if(c.seed==1 && c.rows==1 && !c.onCenter && !c.loose && c.groups==1 && !c.mixed) {
            auto scaled=source;scaled.insert(scaled.rfind('}')," set_scaling_to_player_number ");
            swgb::RmsScript scaledScript;
            check(scaledScript.load(0,[&](int32_t id,std::vector<uint8_t> &bytes){if(id)return false;bytes.assign(scaled.begin(),scaled.end());return true;},{},&error),error.c_str());
            check(!swgb::generateRmsMap(scaledScript,{"FIXED_POSITIONS"},120,120,assets.dat(),rng,map,&error,2,{1,1}) && rng==c.seed && map.terrain.empty(),
                  "Unverified scaled owned-building quantity rejects atomically");
            if(tower) {
                auto adjacent=source;const auto pos=adjacent.find("min_distance_group_placement 6");
                adjacent.replace(pos,30,"min_distance_group_placement 0");
                swgb::RmsScript adjacentScript;
                check(adjacentScript.load(0,[&](int32_t id,std::vector<uint8_t> &bytes){if(id)return false;bytes.assign(adjacent.begin(),adjacent.end());return true;},{},&error),error.c_str());
                check(!swgb::generateRmsMap(adjacentScript,{"FIXED_POSITIONS"},120,120,assets.dat(),rng,map,&error,2,{1,1}) && rng==c.seed && map.terrain.empty(),
                      "Unverified touching turret placement rejects atomically");
            }
            auto bad=assets.dat();bad.civs[1].units[c.unit].foundationTerrainId=farm?27:7;
            check(!swgb::generateRmsMap(script,{"FIXED_POSITIONS"},120,120,bad,rng,map,&error,2,{1,1}) && rng==c.seed && map.terrain.empty(),
                  "Unsupported owned-building foundation rejects atomically");
            if(prefab || farm) {
                bad=assets.dat();bad.civs[1].units[c.unit].resourceStorages[0].amount=prefab?10:16;
                check(!swgb::generateRmsMap(script,{"FIXED_POSITIONS"},120,120,bad,rng,map,&error,2,{1,1}) && rng==c.seed && map.terrain.empty(),
                      "Unverified building resource storage rejects atomically");
            }
            check(!swgb::generateRmsMap(script,{},120,120,assets.dat(),rng,map,&error,2,{1,1}) && rng==c.seed && map.terrain.empty(),
                  "Unverified randomized owned-building starts reject atomically");
        }
        if(c.mixed && c.seed==1) {
            auto bad=assets.dat();bad.civs[1].units[50].resourceStorages[0].amount=16;
            check(!swgb::generateRmsMap(script,{"FIXED_POSITIONS"},120,120,bad,rng,map,&error,2,{1,1}) && rng==c.seed && map.terrain.empty(),
                  "Mixed startup validates later-row prototypes before generation");
        }
        check(swgb::generateRmsMap(script,{"FIXED_POSITIONS"},120,120,assets.dat(),rng,map,&error,2,{1,1}),error.c_str());
        check(rng==c.rng && map.resourcesAfterOwned.size()==size_t(c.mixed==2?20+c.prefabs1+c.prefabs2:quantity*2),"Original owned-building row count and completion RNG");
        uint32_t hash=2166136261u,count=0,next=0;
        auto record=[&](uint32_t id,uint32_t owner,int unit,int facet,float x,float y) {
            for(uint32_t v:{id,owner,uint32_t(unit),uint32_t(facet),uint32_t(x*2),uint32_t(y*2)})hash=(hash^v)*16777619u;
            ++count;
        };
        for(const auto &o:map.cliffs){record(o.creationOrdinal,0,o.unit,o.facet,o.x,o.y);next=o.creationOrdinal+1;}
        for(const auto &o:map.scenery){if(o.alive)record(next,0,o.unitId,o.facet,o.x,o.y);++next;}
        for(const auto &o:map.ownedObjects)record(next++,o.owner,o.unitId,o.facet,o.x,o.y);
        for(const auto &o:map.resourcesAfterOwned)record(next++,o.owner,o.unitId,o.facet,o.x,o.y);
        check(count==c.count && hash==c.objects,"Original complete building startup object order and positions");
        auto gridHash=[](const auto &grid){uint32_t h=2166136261u;if(grid.empty()){for(int i=0;i<14400;++i)h*=16777619u;}else for(auto value:grid)h=(h^value)*16777619u;return h;};
        check(gridHash(map.terrain)==c.terrain && gridHash(map.elevation)==c.heights && gridHash(map.slopes)==c.slopes,"Original building foundation terrain and flat grids");
        swgb::Game game(assets);rng=c.seed;
        check(game.initRms(script,{"FIXED_POSITIONS"},120,rng,&error,2,{1,1}),error.c_str());
        check(rng==c.rng && game.activeObjectCount()==c.count,"Owned-building native import count/RNG");
        for(int owner:{1,2}) {
            const int prefabCount=owner==1?c.prefabs1:c.prefabs2;
            const int extraCost=c.mixed==2?1010+25*prefabCount:c.mixed?510:(tower?150:farm?50:prefab?25:160)*quantity;
            check(game.resource(owner,98)==575+extraCost && game.resource(owner,164)==200 && game.resource(owner,165)==375+extraCost &&
                  game.resource(owner,4)==1+(c.mixed==2?5*prefabCount:c.mixed?20:prefab?5*quantity:0) && game.resource(owner,11)==4 && game.resource(owner,21)==((prefab || farm || tower)?2:3) && game.resource(owner,99)==0,
                  "Original building cost and research counters without duplicate completion grants");
            check(game.technologyOrderForTesting(owner)==(c.mixed==2?std::vector<int>({0,486,22,451,11,16,31,85,209,20}):c.mixed?std::vector<int>({0,486,22,451,31,85,209,16,20}):tower?std::vector<int>({0,486,22,451,11}):farm?(quantity?std::vector<int>({0,486,22,451,20}):std::vector<int>({0,486,22,451})):prefab?std::vector<int>({0,486,22,451,16}):std::vector<int>({0,486,22,451,31,85,209})) && (prefab || farm || tower || game.unitAvailableForTesting(owner,307)),
                  "Original CC then building research chronology and completion unlock");
        }
        for(const auto &b:buildings)if(!c.mixed && b.seed==c.seed && b.rows==c.rows && b.onCenter==c.onCenter && b.unit==c.unit && b.loose==c.loose && b.groups==c.groups) {
            const auto id=b.id+1;const auto position=game.objectPosition(id);
            check(game.objectUnitId(id)==c.unit && game.objectPlayer(id)==int(b.owner) && game.objectHitPoints(id)==(tower?1250:farm?480:prefab?800:1500) &&
                  position[0]==b.y && position[1]==120-b.x && game.pendingFoundationForTesting(id)==(b.owner==1?255:foundation),
                  "Original building ownership, health, position and deferred foundation");
            if(tower)check(std::abs(game.objectFacing(id)-2.35619449f)<1e-5f,"Original isolated turret facet2 survives native import");
            if(farm)check(game.objectResourceAmount(id)==15,"Original completed Farm stores fifteen food");
            const auto animation=game.objectAnimationForTesting(id);
            check(animation[0]==0 && animation[1]==0,"Original building resets animation on completion");
        }
        if(c.seed==1 && c.rows==1 && !c.onCenter && !c.loose && c.groups==1 && !c.mixed) {
            const auto &b=*std::find_if(std::begin(buildings),std::end(buildings),[&](const auto &v){
                return v.seed==c.seed && v.rows==1 && v.owner==2 && v.unit==c.unit && !v.onCenter && !v.loose && v.groups==c.groups;
            });const auto id=b.id+1;
            check(game.spawnObjectForTesting(1,83,1,b.y,120-b.x)!=0,"Reveal original enemy building foundation");
            game.lookAt(b.y,120-b.x);game.render(renderer,960,544);
            check(game.pendingFoundationForTesting(id)==255,"Visible enemy building consumes deferred foundation");
            for(int y=int(b.y)-(tower?0:1);y<int(b.y)+((prefab || tower)?1:2);++y)for(int x=int(b.x)-(tower?0:1);x<int(b.x)+((prefab || tower)?1:2);++x)
                check(game.terrainAtForTesting(y,119-x)==foundation,"Enemy building paints exactly its original footprint");
        }
    }
    std::puts("PASS: owned-building placement, research chronology, costs and deferred foundations");
}
