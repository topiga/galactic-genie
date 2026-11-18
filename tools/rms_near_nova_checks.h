// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
static void rmsNearNovaChecks(swgb::Assets &assets) {
    const std::string grass=R"(<LAND_GENERATION>
base_terrain 0
create_player_lands { terrain_type 0 base_size 3 number_of_tiles 60 }
)";
    const std::string forest=R"(<LAND_GENERATION>
base_terrain 41
create_player_lands { terrain_type 9 base_size 3 number_of_tiles 60 }
<TERRAIN_GENERATION>
create_terrain 10 { base_terrain 41 number_of_tiles 37 number_of_clumps 4 spacing_to_other_terrain_types 1 set_avoid_player_start_areas }
create_terrain 6 { base_terrain 9 number_of_tiles 9 number_of_clumps 2 clumping_factor 6 }
)";
    const std::string starts=R"(<OBJECTS_GENERATION>
create_object 109 { set_place_for_every_player min_distance_to_players 0 max_distance_to_players 0 }
create_object 83 { set_place_for_every_player min_distance_to_players 6 max_distance_to_players 6 }
create_object 550 { set_place_for_every_player min_distance_to_players 7 max_distance_to_players 9 match_player_civ }
)";
    const std::string nova=R"(create_object 66 { number_of_objects 7 group_placement_radius 3 set_tight_grouping set_gaia_object_only set_place_for_every_player min_distance_to_players 12 max_distance_to_players 16 min_distance_group_placement 7 max_distance_to_other_zones 7 }
)";
    const auto objects=starts+nova;
    swgb::RmsScript scripts[2];std::string error;
    auto load=[&](const std::string &source,swgb::RmsScript &script) {
        return script.load(0,[&](int32_t id,std::vector<uint8_t>&bytes) {
            if(id)return false;bytes.assign(source.begin(),source.end());return true;
        },{},&error);
    };
    for(int profile=0;profile<2;++profile) check(load((profile?forest:grass)+objects,scripts[profile]),error.c_str());
    struct Deposit { float x,y;int facet;float heading; };
    const struct { bool forest;uint32_t seed,state,trees,terrainHash;Deposit deposits[14]; } cases[]={
        {false,1u,517217389u,0u,2282508853u,{{4.5f,3.5f,9,0.0f},{4.5f,2.5f,0,0.6283185482025146f},{3.5f,2.5f,4,3.1415927410125732f},{4.5f,1.5f,7,5.026548385620117f},{4.5f,0.5f,3,2.5132741928100586f},{3.5f,1.5f,4,3.1415927410125732f},{2.5f,1.5f,1,1.2566370964050293f},{27.5f,22.5f,4,3.1415927410125732f},{28.5f,22.5f,8,5.654866695404053f},{28.5f,21.5f,0,0.6283185482025146f},{27.5f,21.5f,4,3.1415927410125732f},{28.5f,20.5f,5,3.769911289215088f},{29.5f,20.5f,9,0.0f},{27.5f,20.5f,3,2.5132741928100586f}}},
        {false,12345u,2022414310u,0u,668798869u,{{17.5f,29.5f,1,1.2566370964050293f},{18.5f,29.5f,7,5.026548385620117f},{18.5f,30.5f,0,0.6283185482025146f},{19.5f,30.5f,4,3.1415927410125732f},{20.5f,30.5f,1,1.2566370964050293f},{19.5f,31.5f,4,3.1415927410125732f},{18.5f,31.5f,5,3.769911289215088f},{4.5f,2.5f,6,4.398229598999023f},{3.5f,2.5f,3,2.5132741928100586f},{2.5f,2.5f,2,1.884955644607544f},{1.5f,2.5f,4,3.1415927410125732f},{2.5f,3.5f,3,2.5132741928100586f},{1.5f,3.5f,5,3.769911289215088f},{1.5f,4.5f,2,1.884955644607544f}}},
        {true,1u,622263173u,37u,2495521043u,{{4.5f,1.5f,2,1.884955644607544f},{5.5f,1.5f,3,2.5132741928100586f},{5.5f,2.5f,2,1.884955644607544f},{4.5f,2.5f,4,3.1415927410125732f},{4.5f,3.5f,7,5.026548385620117f},{3.5f,3.5f,4,3.1415927410125732f},{3.5f,2.5f,0,0.6283185482025146f},{0.5f,24.5f,6,4.398229598999023f},{1.5f,24.5f,8,5.654866695404053f},{1.5f,25.5f,4,3.1415927410125732f},{0.5f,25.5f,6,4.398229598999023f},{1.5f,26.5f,0,0.6283185482025146f},{0.5f,26.5f,2,1.884955644607544f},{1.5f,23.5f,5,3.769911289215088f}}},
        {true,12345u,855669726u,38u,1986626585u,{{1.5f,6.5f,2,1.884955644607544f},{0.5f,6.5f,6,4.398229598999023f},{1.5f,5.5f,7,5.026548385620117f},{0.5f,5.5f,8,5.654866695404053f},{0.5f,4.5f,5,3.769911289215088f},{0.5f,3.5f,0,0.6283185482025146f},{2.5f,5.5f,9,0.0f},{21.5f,24.5f,3,2.5132741928100586f},{21.5f,23.5f,7,5.026548385620117f},{21.5f,25.5f,3,2.5132741928100586f},{20.5f,25.5f,7,5.026548385620117f},{22.5f,25.5f,6,4.398229598999023f},{20.5f,24.5f,0,0.6283185482025146f},{19.5f,24.5f,2,1.884955644607544f}}},
    };
    swgb::Game game(assets);
    for(const auto &f:cases) {
        uint32_t state=f.seed;swgb::RmsMap map;
        check(swgb::generateRmsMap(scripts[f.forest],{},32,32,assets.dat(),state,map,&error,2,{1,1}),error.c_str());
        check(state==f.state && map.resourcesAfterOwned.size()==14 && map.ownedObjects.size()==10 && map.scenery.size()==f.trees,
              "Original four-row Nova startup RNG and counts");
        uint32_t hash=2166136261u;for(auto t:map.terrain)hash=(hash^t)*16777619u;
        check(hash==f.terrainHash,"Nova leaves original CC terrain unchanged");
        for(size_t i=0;i<14;++i) {
            const auto &a=map.resourcesAfterOwned[i];const auto &e=f.deposits[i];
            check(a.unitId==66 && a.x==e.x && a.y==e.y && a.facet==e.facet && std::abs(a.headingRadians-e.heading)<.000001f,
                  "Original per-player Nova queue, spacing, occupancy and three-draw appearance");
        }
        uint32_t limited=f.state;std::vector<swgb::RmsTimberPlacement> partial(1);
        swgb::RmsResourceGroups group;group.mode=2;group.perGroup=7;
        check(!swgb::generateRmsGroupedResourcePlacements(32,32,map.terrain,assets.dat(),66,group,limited,
              partial,&error,{},1,map.playerStarts) && limited==f.state && partial.empty(),
              "Near-Nova work exhaustion preserves RNG and clears partial output");
        state=f.seed;
        check(game.initRms(scripts[f.forest],{},32,state,&error,2,{1,1}),error.c_str());
        check(state==f.state && game.activeObjectCount()==f.trees+24,"Nova native import count and RNG");
        check(game.pendingFoundationForTesting(f.trees+1)==255 && game.pendingFoundationForTesting(f.trees+2)==27,
              "Appending Nova preserves original owned-object import IDs and pending foundations");
        for(uint32_t i=0;i<14;++i) {
            const auto id=f.trees+11+i;const auto pos=game.objectPosition(id);const auto &e=f.deposits[i];
            check(game.objectUnitId(id)==66 && game.objectPlayer(id)==0 && pos[0]==e.y && pos[1]==32-e.x,
                  "Nova imports after starting scouts with original IDs offset1");
        }
        for(uint32_t owner=1;owner<=2;++owner) {
            const auto id=f.trees+8+owner;
            check(game.objectUnitId(id)==550 && game.objectPlayer(id)==int(owner),"Nova preserves scout IDs");
            check(game.populationUsedForTesting(owner)==4 && game.populationCapacityForTesting(owner)==5 &&
                  game.resource(owner,11)==4 && game.resource(owner,37)==3 && game.resource(owner,40)==1 &&
                  game.resource(owner,98)==575 && game.resource(owner,164)==200 && game.resource(owner,165)==375,
                  "Gaia Nova does not alter starting player accounting");
        }
    }
    for(const auto &change:std::vector<std::pair<std::string,std::string>>{
        {"number_of_objects 7","number_of_objects 8"},{"number_of_objects 7","number_of_objects 7 number_of_groups 2"},{"group_placement_radius 3","group_placement_radius 4"},
        {"max_distance_to_players 16","max_distance_to_players 17"},{"min_distance_to_players 12","min_distance_to_players 11"},
        {"min_distance_group_placement 7","min_distance_group_placement 6"},{"max_distance_to_other_zones 7",""},
        {"set_tight_grouping","set_loose_grouping"},{"set_gaia_object_only",""},{"create_object 66","create_object 102"}}) {
        auto row=nova;row.replace(row.find(change.first),change.first.size(),change.second);
        swgb::RmsScript variant;check(load(grass+starts+row,variant),error.c_str());
        uint32_t state=12345;swgb::RmsMap map;
        if(change.first=="set_gaia_object_only") {
            check(!swgb::generateRmsMap(variant,{},32,32,assets.dat(),state,map,&error,2,{1,1}) &&
                  state==12345 && map.terrain.empty() && map.resourcesAfterOwned.empty(),
                  "Owned Nova resource construction remains unsupported and rejects atomically");
        } else {
            // The general object controller now accepts these modifiers. This
            // is a supported-path smoke check, not another original golden.
            check(swgb::generateRmsMap(variant,{},32,32,assets.dat(),state,map,&error,2,{1,1}),error.c_str());
            check(map.terrain.size()==32*32 && map.ownedObjects.size()==10 && !map.resourcesAfterOwned.empty(),
                  "General resource modifiers retain terrain, starting actors and resource output");
            const int expectedUnit=change.first=="create_object 66"?102:66;
            for(const auto &object:map.resourcesAfterOwned)
                check(object.unitId==expectedUnit && std::isfinite(object.x) && std::isfinite(object.y) &&
                      object.x>=0 && object.x<32 && object.y>=0 && object.y<32,
                      "General resource modifier output has the requested prototype and valid map coordinates");
        }
    }
    uint32_t state=12345;swgb::RmsMap map;
    check(!swgb::generateRmsMap(scripts[0],{},32,32,assets.dat(),state,map,&error,2,{1,2}) && state==12345,
          "Unproven Nova civilization composition remains rejected");
    check(swgb::generateRmsMap(scripts[0],{},64,64,assets.dat(),state,map,&error,2,{1,1}),error.c_str());
    check(map.terrain.size()==64*64 && map.ownedObjects.size()==10 && map.resourcesAfterOwned.size()==14,
          "General resource controller supports64-square Nova startup (smoke, not original golden)");
    state=12345;
    check(!swgb::generateRmsMap(scripts[0],{"FIXED_POSITIONS"},32,32,assets.dat(),state,map,&error,2,{1,1}) && state==12345,
          "Unproven fixed-start Nova composition remains rejected");
    std::puts("PASS: original composed per-player Nova after CC/workers/scouts on grass and forest");
}
