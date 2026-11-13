// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "../src/engine/rms_map.h"
#include <cmath>

static void rmsGroupedChecks(swgb::Assets &assets) {
    struct Fixture { uint32_t seed,terrainHash,rng; int unit,mode; std::vector<swgb::RmsTimberPlacement> objects; };
    const Fixture fixtures[]={
        {1u,1568528174u,3406030393u,348,1,{{1.5f,8.5f,7,0.0f},{2.5f,8.5f,0,0.0f},{1.5f,10.5f,11,0.0f},{2.5f,16.5f,7,0.0f},{1.5f,17.5f,10,0.0f},{3.5f,18.5f,4,0.0f},{3.5f,16.5f,6,0.0f},{16.5f,28.5f,5,0.0f},{18.5f,27.5f,0,0.0f},{16.5f,27.5f,4,0.0f},{19.5f,25.5f,9,0.0f},{20.5f,28.5f,1,0.0f},{16.5f,25.5f,10,0.0f}}},
        {12345u,3390887566u,1137003990u,348,1,{{2.5f,3.5f,2,0.0f},{1.5f,2.5f,8,0.0f},{2.5f,5.5f,9,0.0f},{1.5f,3.5f,3,0.0f},{4.5f,5.5f,10,0.0f},{2.5f,16.5f,7,0.0f},{2.5f,15.5f,0,0.0f},{3.5f,15.5f,5,0.0f},{4.5f,15.5f,8,0.0f},{3.5f,16.5f,6,0.0f},{29.5f,7.5f,1,0.0f},{25.5f,8.5f,4,0.0f},{27.5f,5.5f,11,0.0f}}},
        {1u,1568528174u,1762777364u,348,2,{{2.5f,11.5f,9,0.0f},{2.5f,10.5f,7,0.0f},{1.5f,10.5f,2,0.0f},{1.5f,11.5f,11,0.0f},{2.5f,9.5f,4,0.0f},{2.5f,8.5f,6,0.0f},{5.5f,12.5f,1,0.0f},{5.5f,11.5f,0,0.0f},{4.5f,11.5f,4,0.0f},{3.5f,11.5f,0,0.0f},{4.5f,19.5f,1,0.0f},{4.5f,20.5f,8,0.0f},{5.5f,20.5f,0,0.0f}}},
        {12345u,3390887566u,3980479351u,348,2,{{3.5f,5.5f,4,0.0f},{3.5f,6.5f,6,0.0f},{3.5f,7.5f,6,0.0f},{3.5f,8.5f,8,0.0f},{2.5f,7.5f,10,0.0f},{1.5f,17.5f,1,0.0f},{1.5f,18.5f,7,0.0f},{2.5f,18.5f,8,0.0f},{1.5f,19.5f,5,0.0f},{2.5f,20.5f,2,0.0f},{1.5f,20.5f,0,0.0f},{0.5f,20.5f,11,0.0f},{0.5f,21.5f,1,0.0f},{1.5f,21.5f,9,0.0f}}},
        {1u,1568528174u,1573924211u,66,1,{{1.5f,8.5f,8,5.654866695404053f},{2.5f,8.5f,4,3.1415927410125732f},{1.5f,10.5f,3,2.5132741928100586f},{3.5f,20.5f,5,3.769911289215088f},{2.5f,18.5f,4,3.1415927410125732f},{2.5f,16.5f,9,0.0f},{1.5f,17.5f,3,2.5132741928100586f},{1.5f,16.5f,7,5.026548385620117f},{18.5f,28.5f,8,5.654866695404053f},{16.5f,26.5f,5,3.769911289215088f},{16.5f,28.5f,7,5.026548385620117f},{16.5f,29.5f,3,2.5132741928100586f},{16.5f,25.5f,9,0.0f}}},
        {12345u,3390887566u,1269774474u,66,1,{{2.5f,3.5f,7,5.026548385620117f},{1.5f,2.5f,9,0.0f},{2.5f,5.5f,3,2.5132741928100586f},{1.5f,3.5f,4,3.1415927410125732f},{4.5f,5.5f,3,2.5132741928100586f},{1.5f,17.5f,0,0.6283185482025146f},{2.5f,16.5f,8,5.654866695404053f},{2.5f,15.5f,0,0.6283185482025146f},{29.5f,8.5f,0,0.6283185482025146f},{26.5f,9.5f,3,2.5132741928100586f},{25.5f,5.5f,0,0.6283185482025146f}}},
        {1u,1568528174u,192980046u,66,2,{{2.5f,11.5f,5,3.769911289215088f},{2.5f,12.5f,1,1.2566370964050293f},{1.5f,11.5f,2,1.884955644607544f},{3.5f,11.5f,4,3.1415927410125732f},{3.5f,10.5f,1,1.2566370964050293f},{2.5f,10.5f,4,3.1415927410125732f},{5.5f,12.5f,9,0.0f},{4.5f,12.5f,9,0.0f},{5.5f,11.5f,2,1.884955644607544f},{4.5f,19.5f,5,3.769911289215088f},{3.5f,19.5f,4,3.1415927410125732f},{5.5f,19.5f,5,3.769911289215088f},{6.5f,19.5f,3,2.5132741928100586f},{4.5f,18.5f,3,2.5132741928100586f}}},
        {12345u,3390887566u,4068993353u,66,2,{{3.5f,5.5f,6,4.398229598999023f},{4.5f,5.5f,4,3.1415927410125732f},{4.5f,6.5f,1,1.2566370964050293f},{4.5f,7.5f,3,2.5132741928100586f},{3.5f,7.5f,3,2.5132741928100586f},{1.5f,17.5f,9,0.0f},{2.5f,17.5f,5,3.769911289215088f},{2.5f,18.5f,8,5.654866695404053f},{3.5f,18.5f,5,3.769911289215088f},{2.5f,19.5f,7,5.026548385620117f},{28.5f,8.5f,3,2.5132741928100586f},{29.5f,8.5f,5,3.769911289215088f},{30.5f,8.5f,9,0.0f},{31.5f,8.5f,7,5.026548385620117f},{30.5f,7.5f,3,2.5132741928100586f}}},
        {1u,1568528174u,1573924211u,102,1,{{1.5f,8.5f,8,5.654866695404053f},{2.5f,8.5f,4,3.1415927410125732f},{1.5f,10.5f,3,2.5132741928100586f},{3.5f,20.5f,5,3.769911289215088f},{2.5f,18.5f,4,3.1415927410125732f},{2.5f,16.5f,9,0.0f},{1.5f,17.5f,3,2.5132741928100586f},{1.5f,16.5f,7,5.026548385620117f},{18.5f,28.5f,8,5.654866695404053f},{16.5f,26.5f,5,3.769911289215088f},{16.5f,28.5f,7,5.026548385620117f},{16.5f,29.5f,3,2.5132741928100586f},{16.5f,25.5f,9,0.0f}}},
        {12345u,3390887566u,1269774474u,102,1,{{2.5f,3.5f,7,5.026548385620117f},{1.5f,2.5f,9,0.0f},{2.5f,5.5f,3,2.5132741928100586f},{1.5f,3.5f,4,3.1415927410125732f},{4.5f,5.5f,3,2.5132741928100586f},{1.5f,17.5f,0,0.6283185482025146f},{2.5f,16.5f,8,5.654866695404053f},{2.5f,15.5f,0,0.6283185482025146f},{29.5f,8.5f,0,0.6283185482025146f},{26.5f,9.5f,3,2.5132741928100586f},{25.5f,5.5f,0,0.6283185482025146f}}},
        {1u,1568528174u,192980046u,102,2,{{2.5f,11.5f,5,3.769911289215088f},{2.5f,12.5f,1,1.2566370964050293f},{1.5f,11.5f,2,1.884955644607544f},{3.5f,11.5f,4,3.1415927410125732f},{3.5f,10.5f,1,1.2566370964050293f},{2.5f,10.5f,4,3.1415927410125732f},{5.5f,12.5f,9,0.0f},{4.5f,12.5f,9,0.0f},{5.5f,11.5f,2,1.884955644607544f},{4.5f,19.5f,5,3.769911289215088f},{3.5f,19.5f,4,3.1415927410125732f},{5.5f,19.5f,5,3.769911289215088f},{6.5f,19.5f,3,2.5132741928100586f},{4.5f,18.5f,3,2.5132741928100586f}}},
        {12345u,3390887566u,4068993353u,102,2,{{3.5f,5.5f,6,4.398229598999023f},{4.5f,5.5f,4,3.1415927410125732f},{4.5f,6.5f,1,1.2566370964050293f},{4.5f,7.5f,3,2.5132741928100586f},{3.5f,7.5f,3,2.5132741928100586f},{1.5f,17.5f,9,0.0f},{2.5f,17.5f,5,3.769911289215088f},{2.5f,18.5f,8,5.654866695404053f},{3.5f,18.5f,5,3.769911289215088f},{2.5f,19.5f,7,5.026548385620117f},{28.5f,8.5f,3,2.5132741928100586f},{29.5f,8.5f,5,3.769911289215088f},{30.5f,8.5f,9,0.0f},{31.5f,8.5f,7,5.026548385620117f},{30.5f,7.5f,3,2.5132741928100586f}}},
    };
    const std::string terrain=R"(<LAND_GENERATION> base_terrain 10
        create_player_lands { terrain_type 9 base_size 3 number_of_tiles 60 }
        <TERRAIN_GENERATION>
        create_terrain 6 { base_terrain 10 number_of_tiles 300 number_of_clumps 12 spacing_to_other_terrain_types 0 }
        create_terrain 44 { base_terrain 9 number_of_tiles 9 number_of_clumps 2 clumping_factor 6 }
        <OBJECTS_GENERATION> )";
    swgb::Game game(assets); swgb::RmsScript script; std::string error;
    auto load=[&](const std::string &source) {
        return script.load(0,[&](int32_t id,std::vector<uint8_t> &bytes) {
            if (id!=0) return false;
            bytes.assign(source.begin(),source.end());return true;
        },{},&error);
    };
    constexpr float pi=3.14159265358979323846f;
    for(const auto &fixture:fixtures) for(int order=0;order<2;++order) {
        const std::string mode=fixture.mode==1?"set_loose_grouping":"set_tight_grouping";
        const auto source=terrain+"create_object "+std::to_string(fixture.unit)+" { "+
            (order?"number_of_groups 3 number_of_objects 5 "+mode:
                   "number_of_objects 5 "+mode+" number_of_groups 3")+
            " group_variance 2 group_placement_radius 3 set_gaia_object_only }";
        check(load(source),error.c_str());
        uint32_t rng=fixture.seed;swgb::RmsMap map;
        check(swgb::generateRmsMap(script,{},32,32,assets.dat(),rng,map,&error),error.c_str());
        uint32_t hash=2166136261u;
        for(auto tile:map.terrain) hash=(hash^tile)*16777619u;
        check(hash==fixture.terrainHash && rng==fixture.rng && map.objects.size()==fixture.objects.size(),
              "Original grouped RMS terrain/count/RNG and parser-order transitions");
        for(size_t i=0;i<map.objects.size();++i) {
            const auto &p=map.objects[i], &expected=fixture.objects[i];
            check(p.unitId==fixture.unit && p.x==expected.x && p.y==expected.y &&
                  p.facet==expected.facet && p.headingRadians==expected.headingRadians,
                  "Original ordered group positions/facets/headings");
        }
        if(order) continue;
        rng=fixture.seed;check(game.initRms(script,{},32,rng,&error),error.c_str());
        check(rng==fixture.rng && game.objectCountForTesting(0,fixture.unit)==fixture.objects.size(),
              "Native grouped resource import count/RNG");
        for(size_t i=0;i<map.objects.size();++i) {
            const auto &p=map.objects[i];const auto id=uint32_t(map.scenery.size()+i+1);
            check(game.objectActive(id) && game.objectPosition(id)==std::array<float,2>{p.x,p.y} &&
                  game.objectHitPoints(id)==(fixture.unit==348?50:0) &&
                  game.objectResourceAmount(id)==(fixture.unit==348?100:fixture.unit==66?800:350),
                  "Grouped resource identity, original health and storage");
            const float facing=game.objectFacing(id);
            const float theta=std::atan2(std::cos(facing)+std::sin(facing),2*(std::cos(facing)-std::sin(facing)));
            const int angles=fixture.unit==348?12:10;
            const int facet=(int(std::lround((theta-pi*.5f)/(2*pi/angles)))%angles+angles)%angles;
            check(facet==p.facet,"Grouped resource render facet survives native import");
        }
    }
    for(const auto &bad:{"group_variance 1025","group_variance -1","group_placement_radius 1025","number_of_groups 1025"}) {
        check(load(terrain+"create_object 66 { set_tight_grouping "+bad+" }"),error.c_str());
        uint32_t rng=1;swgb::RmsMap map;
        check(!swgb::generateRmsMap(script,{},32,32,assets.dat(),rng,map,&error) && rng==1 && map.objects.empty(),
              "Grouped script bounds reject transactionally");
    }
    std::puts("PASS: original grouped RMS compositions, source-order semantics and native imports");
}
