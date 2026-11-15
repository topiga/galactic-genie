// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cmath>
// Original495f80/4ddd40 on native-generated authored geometry. Geometry helpers
// have separate original execution tests; these fixtures cover composed scenery
// and owned startup, not a fresh original run of the entire RMS script.
static void checkRmsForest21Owned(swgb::Assets &assets) {
    const auto &data=assets.dat();
    const std::string source=R"(<LAND_GENERATION> base_terrain 6 create_player_lands { terrain_type 6 base_size 3 number_of_tiles 60 }
<ELEVATION_GENERATION> create_elevation 2 { base_terrain 6 number_of_tiles 200 number_of_clumps 4 }
<TERRAIN_GENERATION> create_terrain 21 { base_terrain 6 number_of_tiles 100 number_of_clumps 4 spacing_to_other_terrain_types 1 set_avoid_player_start_areas }
<OBJECTS_GENERATION>
create_object 109 { set_place_for_every_player min_distance_to_players 0 max_distance_to_players 0 }
create_object 83 { set_place_for_every_player min_distance_to_players 6 max_distance_to_players 6 }
create_object 550 { set_place_for_every_player min_distance_to_players 7 max_distance_to_players 9 match_player_civ }
)";
    swgb::RmsScript script;std::string error;
    check(script.load(0,[&](int32_t id,std::vector<uint8_t>&out) {
        if(id)return false;out.assign(source.begin(),source.end());return true;
    },{},&error),error.c_str());
    auto checkImport=[&](uint32_t seed,const swgb::RmsMap &map,uint32_t final) {
        swgb::Game game(assets);uint32_t state=seed;
        check(game.initRms(script,{},120,state,&error,2,{1,1}),error.c_str());
        check(state==final && game.activeObjectCount()==map.scenery.size()+10,"Forest21 native import count/RNG");
        for(size_t i=0;i<map.ownedObjects.size();++i) {
            const auto id=uint32_t(map.scenery.size()+1+i);const auto &expected=map.ownedObjects[i];
            const auto position=game.objectPosition(id);const auto animation=game.objectAnimationForTesting(id);
            const auto &unit=data.civs[1].units[expected.unitId];const auto *graphic=data.graphic(unit.standingGraphic[0]);
            check(game.objectUnitId(id)==expected.unitId && game.objectPlayer(id)==expected.owner &&
                position[0]==expected.y && position[1]==120-expected.x &&
                animation[0]==expected.animationFrame*graphic->frameDuration && animation[1]==expected.initialIdleDelay,
                "Forest21 native starting IDs/positions/animation");
        }
    };
    auto hash=[](const std::vector<uint8_t>&bytes) {
        uint32_t value=2166136261u;for(auto b:bytes)value=(value^b)*16777619u;return value;
    };
    struct Owned { int owner,unit;float x,y;int pending,facet,frame;float delay; };
    struct Tree { int tile,facet; };
    {
        uint32_t rng=1u;swgb::RmsMap map;
        const auto seed=rng;
        check(swgb::generateRmsMap(script,{},120,120,data,rng,map,&error,2,{1,1}),error.c_str());
        checkImport(seed,map,rng);
        check(rng==2607337858u && map.ownedObjects.size()==10,"Forest21 owned final RNG/count");
        check(hash(map.terrain)==1015550221u,"Forest21 original terrain");
        check(hash(map.elevation)==4089223887u,"Forest21 original elevation");
        check(hash(map.slopes)==140112869u,"Forest21 original slopes");
        const Owned owned[]={
            {1,109,72.0f,14.0f,255,0,0,0.0f},
            {2,109,46.0f,82.0f,27,0,0,0.0f},
            {1,293,70.5f,8.5f,255,1,5,0.0f},
            {1,293,68.5f,8.5f,255,3,7,1.8901638984680176f},
            {1,293,66.5f,12.5f,255,2,7,0.9595935344696045f},
            {2,293,46.5f,76.5f,255,1,5,0.0f},
            {2,293,42.5f,76.5f,255,4,4,0.0f},
            {2,293,40.5f,80.5f,255,7,7,0.0f},
            {1,550,75.5f,21.5f,255,2,0,0.0f},
            {2,550,43.5f,75.5f,255,5,0,0.0f},
        };
        for(size_t i=0;i<10;++i) {
            const auto &a=map.ownedObjects[i];const auto &e=owned[i];
            check(a.owner==e.owner && a.civilization==1 && a.unitId==e.unit &&
                  a.x==e.x && a.y==e.y && a.pendingFoundation==e.pending &&
                  a.facet==e.facet && a.animationFrame==e.frame &&
                  std::abs(a.initialIdleDelay-e.delay)<.000001f,"Forest21 original owned pose");
        }
        const Tree trees[]={{4547,6},{4548,8},{4549,0},{4550,6},{4666,8},{4667,9},{4668,5},{4669,3},{4670,0},{4785,6},{4786,6},{4787,8},{4788,11},{4789,7},{4790,8},{4791,3},{4830,5},{4904,10},{4905,7},{4906,1},{4907,3},{4908,1},{4909,7},{4950,10},{4951,5},{5026,11},{5027,4},{5028,5},{5068,1},{5069,7},{5070,3},{5071,1},{5147,6},{5185,1},{5188,0},{5189,11},{5190,5},{5305,11},{5306,4},{5307,6},{5308,3},{5309,6},{5310,2},{5426,5},{5427,5},{5428,4},{5429,4},{5545,1},{5546,8},{5547,3},{5666,6},{11737,7},{11738,10},{11739,7},{11855,1},{11856,3},{11857,8},{11858,1},{11859,7},{11860,7},{11975,10},{11976,6},{11977,11},{11978,2},{11979,2},{12095,1},{12096,0},{12097,11},{12098,2},{12099,10},{12215,0},{12216,1},{12217,4},{12218,11},{12336,11},{12337,10},{13170,11},{13288,2},{13289,4},{13290,4},{13294,10},{13410,11},{13411,8},{13412,8},{13413,6},{13414,6},{13415,2},{13529,8},{13530,10},{13531,2},{13532,2},{13533,9},{13534,6},{13647,5},{13648,10},{13649,4},{13650,2},{13651,1},{13652,7},{13770,3},{13771,8},{13772,0}};
        check(map.scenery.size()==sizeof(trees)/sizeof(trees[0]),"Forest21 original scenery count");
        for(size_t i=0;i<map.scenery.size();++i) {
            const auto &a=map.scenery[i];const auto &e=trees[i];
            check(a.alive && a.unitId==413 && a.x==float(e.tile%120)+.5f &&
                  a.y==float(e.tile/120)+.5f && a.facet==e.facet,"Forest21 original scenery pose");
        }
    }
    {
        uint32_t rng=12345u;swgb::RmsMap map;
        const auto seed=rng;
        check(swgb::generateRmsMap(script,{},120,120,data,rng,map,&error,2,{1,1}),error.c_str());
        checkImport(seed,map,rng);
        check(rng==2786029445u && map.ownedObjects.size()==10,"Forest21 owned final RNG/count");
        check(hash(map.terrain)==520995590u,"Forest21 original terrain");
        check(hash(map.elevation)==2763925479u,"Forest21 original elevation");
        check(hash(map.slopes)==2713444239u,"Forest21 original slopes");
        const Owned owned[]={
            {1,109,14.0f,77.0f,255,0,0,0.0f},
            {2,109,82.0f,41.0f,27,0,0,0.0f},
            {1,83,8.5f,81.5f,255,5,4,0.0f},
            {1,293,8.5f,72.5f,255,6,7,0.0f},
            {1,293,8.5f,80.5f,255,7,2,0.0f},
            {2,293,76.5f,44.5f,255,2,2,0.0f},
            {2,293,81.5f,35.5f,255,6,4,0.0f},
            {2,293,76.5f,42.5f,255,1,7,0.005188107490539551f},
            {1,550,6.5f,84.5f,255,2,0,0.0f},
            {2,550,84.5f,33.5f,255,1,0,0.0f},
        };
        for(size_t i=0;i<10;++i) {
            const auto &a=map.ownedObjects[i];const auto &e=owned[i];
            check(a.owner==e.owner && a.civilization==1 && a.unitId==e.unit &&
                  a.x==e.x && a.y==e.y && a.pendingFoundation==e.pending &&
                  a.facet==e.facet && a.animationFrame==e.frame &&
                  std::abs(a.initialIdleDelay-e.delay)<.000001f,"Forest21 original owned pose");
        }
        const Tree trees[]={{2376,8},{2494,3},{2495,9},{2496,4},{2497,0},{2614,1},{2615,3},{2616,3},{2733,6},{2734,8},{2854,7},{2855,1},{2972,6},{2973,4},{2974,6},{2975,11},{3092,4},{3093,6},{3094,0},{3095,11},{3096,8},{3212,3},{3213,11},{3214,1},{3215,11},{3334,10},{4188,1},{4189,7},{4307,6},{4308,5},{4309,8},{4310,1},{4427,6},{4428,11},{4429,5},{4430,1},{4431,9},{4547,0},{4548,4},{4549,4},{4550,2},{4551,6},{4552,8},{4667,8},{4668,7},{4669,10},{4670,3},{4671,2},{4789,1},{4790,5},{4910,3},{7667,6},{7668,11},{7788,0},{7789,2},{7790,1},{7908,10},{7909,9},{7910,7},{7911,6},{8028,4},{8029,5},{8030,7},{8031,7},{8032,7},{8148,5},{8149,5},{8150,3},{8151,4},{8268,5},{8269,0},{8270,3},{8271,10},{8315,3},{8316,6},{8389,4},{8390,11},{8435,5},{8436,5},{8437,8},{8438,4},{8510,5},{8555,3},{8556,5},{8557,9},{8558,1},{8559,8},{8560,5},{8675,6},{8676,6},{8677,0},{8678,6},{8679,2},{8680,1},{8796,4},{8797,9},{8798,6},{8799,10},{8915,1},{8916,1},{8917,6},{8918,4},{9038,4}};
        check(map.scenery.size()==sizeof(trees)/sizeof(trees[0]),"Forest21 original scenery count");
        for(size_t i=0;i<map.scenery.size();++i) {
            const auto &a=map.scenery[i];const auto &e=trees[i];
            check(a.alive && a.unitId==413 && a.x==float(e.tile%120)+.5f &&
                  a.y==float(e.tile/120)+.5f && a.facet==e.facet,"Forest21 original scenery pose");
        }
    }
}
