// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <iostream>
static void checkRmsCliffMap(const swgb::dat::DatFile &data) {
    using namespace swgb;
    const std::string land="<LAND_GENERATION> base_terrain 6 create_land { terrain_type 6 land_position 50 50 base_size 0 number_of_tiles 0 } ";
    const char *settings[]={
        "",
        "min_number_of_cliffs 5 max_number_of_cliffs 10 min_length_of_cliff 3 max_length_of_cliff 12 cliff_curliness 35 min_distance_cliffs 1 min_terrain_distance 1 ",
        "min_number_of_cliffs 0 max_number_of_cliffs 0 ",
        "min_number_of_cliffs 2 max_number_of_cliffs 2 min_length_of_cliff 0 max_length_of_cliff 0 cliff_curliness 0 min_terrain_distance 0 min_distance_cliffs 1 ",
        "min_number_of_cliffs 2 max_number_of_cliffs 2 min_length_of_cliff 1 max_length_of_cliff 1 cliff_curliness 100 min_terrain_distance 0 min_distance_cliffs 1 ",
        "min_number_of_cliffs 2 max_number_of_cliffs 2 min_length_of_cliff 2 max_length_of_cliff 2 cliff_curliness 100 min_terrain_distance 0 min_distance_cliffs 1 ",
        "min_number_of_cliffs 2 max_number_of_cliffs 2 min_length_of_cliff 3 max_length_of_cliff 3 cliff_curliness 100 min_terrain_distance 0 min_distance_cliffs 1 ",
        "min_number_of_cliffs 20 max_number_of_cliffs 20 min_length_of_cliff 30 max_length_of_cliff 40 cliff_curliness 100 min_terrain_distance 0 min_distance_cliffs 1 ",
        "min_number_of_cliffs 30 max_number_of_cliffs 40 min_length_of_cliff 3 max_length_of_cliff 30 cliff_curliness 0 min_terrain_distance 0 min_distance_cliffs 1 ",
        "min_number_of_cliffs 10 max_number_of_cliffs 20 min_length_of_cliff 3 max_length_of_cliff 20 cliff_curliness 50 min_terrain_distance 0 min_distance_cliffs 39 ",
        "min_number_of_cliffs 999 max_number_of_cliffs 999 min_length_of_cliff 0 max_length_of_cliff 2 cliff_curliness 100 min_terrain_distance 0 min_distance_cliffs 1 "
    };
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
        {3,1u,2558158352u,2166136261u,0},
        {3,12345u,1025198890u,2166136261u,0},
        {3,4294967295u,3669407112u,2166136261u,0},
        {4,1u,2558158352u,2166136261u,0},
        {4,12345u,1025198890u,2166136261u,0},
        {4,4294967295u,3669407112u,2166136261u,0},
        {5,1u,2558158352u,2166136261u,0},
        {5,12345u,1025198890u,2166136261u,0},
        {5,4294967295u,3669407112u,2166136261u,0},
        {6,1u,2144960216u,3308703961u,8},
        {6,12345u,4114177586u,1754853289u,8},
        {6,4294967295u,1800325706u,4057489208u,8},
        {7,1u,2851926737u,1712362764u,420},
        {7,12345u,3986125029u,577991049u,394},
        {7,4294967295u,2171071764u,4241318935u,399},
        {8,1u,699734975u,4002586549u,423},
        {8,12345u,2369775109u,500297054u,522},
        {8,4294967295u,2297857372u,1813709853u,418},
        {9,1u,443464005u,3313979162u,17},
        {9,12345u,379872204u,4015112052u,10},
        {9,4294967295u,1562644537u,231032273u,7},
        {10,1u,2848727207u,2166136261u,0},
        {10,12345u,2365967353u,2166136261u,0},
        {10,4294967295u,4263982527u,2166136261u,0},
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
    struct TerrainCase { int tiles,clumps,spacing,clumping; bool flat;
        uint32_t seed,rng,objects,terrain; size_t count; };
    const TerrainCase terrainCases[]={
        {30,3,1,10,true,1u,646082904u,1363270808u,3484421045u,29},
        {30,3,1,10,true,12345u,2638519242u,959006638u,3095328181u,45},
        {30,3,1,10,true,4294967295u,1620821508u,1228384702u,3173278085u,21},
        {2000,20,2,20,false,1u,3225468973u,1363270808u,365659973u,29},
        {2000,20,2,20,false,12345u,2742977745u,959006638u,1539307699u,45},
        {2000,20,2,20,false,4294967295u,1218767571u,1228384702u,679768181u,21},
        {8000,1,0,0,false,1u,4127068469u,1363270808u,1009525507u,29},
        {8000,1,0,0,false,12345u,2886244795u,959006638u,779798787u,45},
        {8000,1,0,0,false,4294967295u,137764506u,1228384702u,4214891795u,21},
    };
    for(const auto &c:terrainCases) {
        load(land+"<CLIFF_GENERATION> <TERRAIN_GENERATION> create_terrain 0 { base_terrain 6 number_of_tiles "+
            std::to_string(c.tiles)+" number_of_clumps "+std::to_string(c.clumps)+
            " spacing_to_other_terrain_types "+std::to_string(c.spacing)+" clumping_factor "+
            std::to_string(c.clumping)+(c.flat?" set_flat_terrain_only ":" ")+"}");
        uint32_t rng=c.seed;
        check(generateRmsMap(script,{},120,120,data,rng,map,&error),error.c_str());
        uint32_t objects=2166136261u,terrain=2166136261u;
        for(const auto &o:map.cliffs)
            for(uint32_t v:{uint32_t(o.unit),uint32_t(o.facet),uint32_t(o.x*2),uint32_t(o.y*2),o.creationOrdinal})
                objects=(objects^v)*16777619u;
        for(auto tile:map.terrain) terrain=(terrain^tile)*16777619u;
        if(rng!=c.rng || objects!=c.objects || terrain!=c.terrain)
            std::cerr<<"cliff/terrain "<<c.tiles<<' '<<c.seed<<" rng "<<rng<<'/'<<c.rng
                <<" objects "<<objects<<'/'<<c.objects<<" terrain "<<terrain<<'/'<<c.terrain<<'\n';
        check(rng==c.rng && objects==c.objects && terrain==c.terrain && map.cliffs.size()==c.count,
            "original cliff/grass composition objects, terrain and RNG");
        check(map.scenery.empty() && map.objects.empty(),"grass painting adds no scenery");
    }
    struct ElevationCase { int tiles,clumps; uint32_t seed,rng,objects,heights,slopes; size_t count; };
    const ElevationCase elevationCases[]={
        {400,8,1u,2790043587u,291107893u,73028170u,3169292009u,29},
        {400,8,12345u,3576478055u,1991487999u,1897583699u,1760096298u,55},
        {400,8,4294967295u,875337137u,1565698685u,3593664539u,3804388132u,20},
        {1000,2,1u,506459290u,1053432523u,2939230450u,463713662u,27},
        {1000,2,12345u,3863891629u,2160117617u,2456656715u,1555305034u,30},
        {1000,2,4294967295u,178198308u,1763354238u,92000700u,4288308406u,35},
        {8000,30,1u,2269132341u,3231355347u,1601223514u,3045030720u,47},
        {8000,30,12345u,356477077u,1829178315u,3368565405u,202350159u,42},
        {8000,30,4294967295u,3507099122u,2453755317u,2749370310u,3751637635u,50},
    };
    for(const auto &c:elevationCases) for(bool late:{false,true}) {
        const std::string elevation="<ELEVATION_GENERATION> create_elevation 1 { base_terrain 6 number_of_tiles "+
            std::to_string(c.tiles)+" number_of_clumps "+std::to_string(c.clumps)+" } ";
        load(land+(late?"<CLIFF_GENERATION> "+elevation:elevation+"<CLIFF_GENERATION> "));
        uint32_t rng=c.seed;
        check(generateRmsMap(script,{},120,120,data,rng,map,&error),error.c_str());
        uint32_t objects=2166136261u,heights=2166136261u,slopes=2166136261u;
        for(const auto &o:map.cliffs)
            for(uint32_t v:{uint32_t(o.unit),uint32_t(o.facet),uint32_t(o.x*2),uint32_t(o.y*2),o.creationOrdinal})
                objects=(objects^v)*16777619u;
        for(auto h:map.elevation) heights=(heights^h)*16777619u;
        for(auto slope:map.slopes) slopes=(slopes^slope)*16777619u;
        if(rng!=c.rng || objects!=c.objects || heights!=c.heights || slopes!=c.slopes)
            std::cerr<<"cliff/elevation "<<c.tiles<<' '<<c.seed<<" rng "<<rng<<'/'<<c.rng
                <<" objects "<<objects<<'/'<<c.objects<<" heights "<<heights<<'/'<<c.heights
                <<" slopes "<<slopes<<'/'<<c.slopes<<'\n';
        check(rng==c.rng && objects==c.objects && heights==c.heights && slopes==c.slopes && map.cliffs.size()==c.count,
            "original elevation/cliff composition objects, heights, slopes and RNG");
        for(auto tile:map.terrain) check(tile==6,"elevated cliffs preserve grass terrain");
        check(map.scenery.empty() && map.objects.empty(),"elevated grass adds no scenery");
    }
    uint32_t rng=1;
    load(land+"<CLIFF_GENERATION> min_distance_cliffs 0");
    check(!generateRmsMap(script,{},120,120,data,rng,map,&error) && rng==1 && map.cliffs.empty(),"unsupported cliff spacing is transactional");
    load(land+"<CLIFF_GENERATION> <TERRAIN_GENERATION> create_terrain 3 { base_terrain 6 number_of_tiles 10 }");
    check(!generateRmsMap(script,{},120,120,data,rng,map,&error) && rng==1 && map.cliffs.empty(),"cliff/terrain combination remains gated");
    for(const char *rows:{
        "create_terrain 0 { base_terrain 6 number_of_tiles 10 set_avoid_player_start_areas }",
        "create_terrain 0 { base_terrain 6 number_of_tiles 10 } create_terrain 0 { base_terrain 6 number_of_tiles 20 }"}) {
        load(land+"<CLIFF_GENERATION> <TERRAIN_GENERATION> "+rows);
        check(!generateRmsMap(script,{},120,120,data,rng,map,&error) && rng==1 && map.cliffs.empty(),
            "unverified cliff terrain-row combinations remain transactional");
    }
    for(const char *elevation:{
        "<ELEVATION_GENERATION>",
        "<ELEVATION_GENERATION> create_elevation 2 { base_terrain 6 number_of_tiles 400 }",
        "<ELEVATION_GENERATION> create_elevation 1 { base_terrain 6 number_of_tiles 400 } <TERRAIN_GENERATION> create_terrain 0 { base_terrain 6 number_of_tiles 30 }"}) {
        load(land+"<CLIFF_GENERATION> "+elevation);
        check(!generateRmsMap(script,{},120,120,data,rng,map,&error) && rng==1 && map.cliffs.empty(),
            "unverified cliff elevation combinations remain transactional");
    }
    load(land+"<CLIFF_GENERATION> cliff_curliness nope");
    check(!generateRmsMap(script,{},120,120,data,rng,map,&error) && rng==1,"malformed cliff integer");
}
