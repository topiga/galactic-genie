// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Actual4de830/5cfad0, captured Empire87; rms-owned87-bounded-findings-2026-11-16.json.
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
    struct Case { uint32_t seed,rows,count,rng,objects,terrain,heights,slopes; };
    const Case cases[]={
        {1u,1u,5989u,1745024857u,1456092772u,3952391958u,1017236165u,1017236165u},
        {12345u,1u,5973u,3433587427u,3999233898u,2021000264u,1017236165u,1017236165u},
        {16160u,1u,6065u,3908054248u,1674239946u,2635766779u,1017236165u,1017236165u},
        {1u,2u,5991u,2727130514u,258802950u,3531810289u,1017236165u,1017236165u},
    };
    struct Building { uint32_t seed,rows,id,owner; float x,y; };
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
    };
    for(const auto &c:cases) {
        const std::string source=baseline+(c.rows==2?extra:"");
        swgb::RmsScript script;std::string error;
        check(script.load(0,[&](int32_t id,std::vector<uint8_t> &bytes){if(id)return false;bytes.assign(source.begin(),source.end());return true;},{},&error),error.c_str());
        swgb::RmsMap map;uint32_t rng=c.seed;
        if(c.seed==1 && c.rows==1) {
            auto scaled=source;scaled.insert(scaled.rfind('}')," set_scaling_to_player_number ");
            swgb::RmsScript scaledScript;
            check(scaledScript.load(0,[&](int32_t id,std::vector<uint8_t> &bytes){if(id)return false;bytes.assign(scaled.begin(),scaled.end());return true;},{},&error),error.c_str());
            check(!swgb::generateRmsMap(scaledScript,{"FIXED_POSITIONS"},120,120,assets.dat(),rng,map,&error,2,{1,1}) && rng==c.seed && map.terrain.empty(),
                  "Unverified scaled owned87 quantity rejects atomically");
            auto bad=assets.dat();bad.civs[1].units[87].foundationTerrainId=7;
            check(!swgb::generateRmsMap(script,{"FIXED_POSITIONS"},120,120,bad,rng,map,&error,2,{1,1}) && rng==c.seed && map.terrain.empty(),
                  "Unsupported owned87 foundation rejects atomically");
            check(!swgb::generateRmsMap(script,{},120,120,assets.dat(),rng,map,&error,2,{1,1}) && rng==c.seed && map.terrain.empty(),
                  "Unverified randomized owned87 starts reject atomically");
        }
        check(swgb::generateRmsMap(script,{"FIXED_POSITIONS"},120,120,assets.dat(),rng,map,&error,2,{1,1}),error.c_str());
        check(rng==c.rng && map.resourcesAfterOwned.size()==c.rows*2,"Original owned87 row count and completion RNG");
        uint32_t hash=2166136261u,count=0,next=0;
        auto record=[&](uint32_t id,uint32_t owner,int unit,int facet,float x,float y) {
            for(uint32_t v:{id,owner,uint32_t(unit),uint32_t(facet),uint32_t(x*2),uint32_t(y*2)})hash=(hash^v)*16777619u;
            ++count;
        };
        for(const auto &o:map.cliffs){record(o.creationOrdinal,0,o.unit,o.facet,o.x,o.y);next=o.creationOrdinal+1;}
        for(const auto &o:map.scenery){if(o.alive)record(next,0,o.unitId,o.facet,o.x,o.y);++next;}
        for(const auto &o:map.ownedObjects)record(next++,o.owner,o.unitId,o.facet,o.x,o.y);
        for(const auto &o:map.resourcesAfterOwned)record(next++,o.owner,o.unitId,o.facet,o.x,o.y);
        check(count==c.count && hash==c.objects,"Original complete87 startup object order and positions");
        auto gridHash=[](const auto &grid){uint32_t h=2166136261u;if(grid.empty()){for(int i=0;i<14400;++i)h*=16777619u;}else for(auto value:grid)h=(h^value)*16777619u;return h;};
        check(gridHash(map.terrain)==c.terrain && gridHash(map.elevation)==c.heights && gridHash(map.slopes)==c.slopes,"Original87 foundation terrain and flat grids");
        swgb::Game game(assets);rng=c.seed;
        check(game.initRms(script,{"FIXED_POSITIONS"},120,rng,&error,2,{1,1}),error.c_str());
        check(rng==c.rng && game.activeObjectCount()==c.count,"Owned87 native import count/RNG");
        for(int owner:{1,2}) {
            check(game.resource(owner,98)==575+160*c.rows && game.resource(owner,164)==200 && game.resource(owner,165)==375+160*c.rows &&
                  game.resource(owner,4)==1 && game.resource(owner,11)==4 && game.resource(owner,21)==3 && game.resource(owner,99)==0,
                  "Original87 cost and research counters without duplicate completion grants");
            check(game.technologyOrderForTesting(owner)==std::vector<int>({0,486,22,451,31,85,209}) && game.unitAvailableForTesting(owner,307),
                  "Original CC then87 research chronology and effect70 enable307");
        }
        for(const auto &b:buildings)if(b.seed==c.seed && b.rows==c.rows) {
            const auto id=b.id+1;const auto position=game.objectPosition(id);
            check(game.objectUnitId(id)==87 && game.objectPlayer(id)==int(b.owner) && game.objectHitPoints(id)==1500 &&
                  position[0]==b.y && position[1]==120-b.x && game.pendingFoundationForTesting(id)==(b.owner==1?255:27),
                  "Original87 ownership, health, position and deferred foundation");
            const auto animation=game.objectAnimationForTesting(id);
            check(animation[0]==0 && animation[1]==0,"Original87 resets animation on completion");
        }
        if(c.seed==1 && c.rows==1) {
            const auto &b=buildings[1];const auto id=b.id+1;
            check(game.spawnObjectForTesting(1,83,1,b.y,120-b.x)!=0,"Reveal original enemy87 foundation");
            game.lookAt(b.y,120-b.x);game.render(renderer,960,544);
            check(game.pendingFoundationForTesting(id)==255,"Visible enemy87 consumes deferred foundation");
            for(int y=int(b.y)-1;y<=int(b.y)+1;++y)for(int x=int(b.x)-1;x<=int(b.x)+1;++x)
                check(game.terrainAtForTesting(y,119-x)==27,"Enemy87 paints exactly its original3x3 footprint");
        }
    }
    std::puts("PASS: owned87 placement, research chronology, costs and deferred foundations");
}
