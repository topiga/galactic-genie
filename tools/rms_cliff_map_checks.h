// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <iostream>
static void checkRmsCliffMap(const swgb::dat::DatFile &data) {
    using namespace swgb;
    const std::string land="<LAND_GENERATION> base_terrain 6 create_land { terrain_type 6 land_position 50 50 base_size 0 number_of_tiles 0 } ";
    const char *settings[]={"", "min_number_of_cliffs 5 max_number_of_cliffs 10 min_length_of_cliff 3 max_length_of_cliff 12 cliff_curliness 35 min_distance_cliffs 1 min_terrain_distance 1 ", "min_number_of_cliffs 0 max_number_of_cliffs 0 "};
    struct Case { int settings; uint32_t seed,rng,hash; size_t count; };
    // Actual original scheduler, land, cliffs, empty terrain and object stages.
    const Case cases[]={
        {0,1u,1150416911u,1363270808u,29},
        {0,12345u,1497412313u,959006638u,45},
        {0,4294967295u,2483271371u,1228384702u,21},
        {1,1u,1755895453u,1286295733u,55},
        {1,12345u,1772623844u,4126238780u,72},
        {1,4294967295u,2710870391u,602856561u,37},
        {2,1u,887571558u,2166136261u,0},
        {2,12345u,4126090032u,2166136261u,0},
        {2,4294967295u,2372191774u,2166136261u,0},
    };
    RmsScript script; RmsMap map; std::string error;
    auto load=[&](const std::string &text) {
        check(script.load(0,[&](int32_t id,std::vector<uint8_t> &out) {
            if(id) return false; out.assign(text.begin(),text.end()); return true;
        },{},&error),error.c_str());
    };
    for(const auto &c:cases) for(bool late:{false,true}) {
        const std::string cliff="<CLIFF_GENERATION> "+std::string(settings[c.settings]);
        load(land+(late?"<TERRAIN_GENERATION> <OBJECTS_GENERATION> "+cliff:cliff+"<TERRAIN_GENERATION> <OBJECTS_GENERATION> "));
        uint32_t rng=c.seed;
        check(generateRmsMap(script,{},120,120,data,rng,map,&error),error.c_str());
        uint32_t hash=2166136261u;
        for(const auto &o:map.cliffs)
            for(uint32_t v:{uint32_t(o.unit),uint32_t(o.facet),uint32_t(o.x*2),uint32_t(o.y*2),o.creationOrdinal})
                hash=(hash^v)*16777619u;
        if(rng!=c.rng || hash!=c.hash || map.cliffs.size()!=c.count) std::cerr<<"cliff settings "<<c.settings<<" seed "<<c.seed<<" rng "<<rng<<'/'<<c.rng<<" hash "<<hash<<'/'<<c.hash<<" count "<<map.cliffs.size()<<'/'<<c.count<<'\n';
        check(rng==c.rng && hash==c.hash && map.cliffs.size()==c.count,"original composed cliff objects and RNG");
        check(map.scenery.empty() && map.objects.empty() && map.ownedObjects.empty(),"cliff-only profile creates no later objects");
        for(auto t:map.terrain) check(t==6,"cliffs preserve grass terrain");
    }
    uint32_t rng=1;
    load(land+"<CLIFF_GENERATION> min_distance_cliffs 0");
    check(!generateRmsMap(script,{},120,120,data,rng,map,&error) && rng==1 && map.cliffs.empty(),"unsupported cliff spacing is transactional");
    load(land+"<CLIFF_GENERATION> <TERRAIN_GENERATION> create_terrain 0 { base_terrain 6 number_of_tiles 10 }");
    check(!generateRmsMap(script,{},120,120,data,rng,map,&error) && rng==1 && map.cliffs.empty(),"cliff/terrain combination remains gated");
    load(land+"<CLIFF_GENERATION> cliff_curliness nope");
    check(!generateRmsMap(script,{},120,120,data,rng,map,&error) && rng==1,"malformed cliff integer");
}
