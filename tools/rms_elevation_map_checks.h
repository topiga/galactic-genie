// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
static void checkRmsElevationMap() {
    using namespace swgb;
    dat::DatFile data;data.terrainBlock.terrains.resize(7);data.terrainBlock.terrainsUsed=7;
    for(int id:{0,3,6}) data.terrainBlock.terrains[id].enabled=1;
    const std::string land="<LAND_GENERATION> base_terrain 0 create_land { terrain_type 6 land_position 50 50 base_size 3 number_of_tiles 160 clumping_factor 8 border_fuzziness 20 } ";
    const std::string hill="<ELEVATION_GENERATION> create_elevation 2 { base_terrain 6 number_of_tiles 40 number_of_clumps 2 } ";
    const std::string paint="<TERRAIN_GENERATION> create_terrain 3 { base_terrain 6 number_of_tiles 30 number_of_clumps 3 spacing_to_other_terrain_types 1 clumping_factor 10 set_flat_terrain_only } ";
    auto hash=[](const std::vector<uint8_t> &a){uint32_t h=2166136261u;for(auto v:a)h=(h^v)*16777619u;return h;};
    RmsScript script;RmsMap map;std::string error;
    auto load=[&](const std::string &source) {
        check(script.load(0,[&](int32_t id,std::vector<uint8_t> &bytes) {
            if(id) return false;bytes.assign(source.begin(),source.end());return true;
        },{},&error),error);
    };
    // Original LAND/ELEVATION/TERRAIN dispatch plus the independently verified
    // empty object stage's final scheduler draw. No original objects created.
    const uint32_t cases[][6]={
        {1u,0,2315207044u,3869364677u,3869364677u,2477138680u},
        {1u,1,1074340216u,3869364677u,3869364677u,3479573979u},
        {1u,2,882794575u,2866335211u,1759538633u,2019025005u},
        {12345u,0,581258974u,3869364677u,3869364677u,1251667036u},
        {12345u,1,2817090761u,3869364677u,3869364677u,4183098392u},
        {12345u,2,1214363207u,1250396660u,4283944291u,296233188u},
        {4294967295u,0,40650714u,3869364677u,3869364677u,2074467178u},
        {4294967295u,1,3407644862u,3869364677u,3869364677u,67215473u},
        {4294967295u,2,349622539u,3474746597u,703997187u,2180953274u}
    };
    for(const auto &c:cases) for(bool late:{false,true}) {
        const std::string elevation=c[1]==0?"":c[1]==1?"<ELEVATION_GENERATION> ":hill;
        load(land+(late?paint+"<OBJECTS_GENERATION> "+elevation:elevation+paint));
        uint32_t rng=c[0];check(generateRmsMap(script,{},16,16,data,rng,map,&error),error);
        check(hash(map.terrain)==c[2] && rng==c[5],"original composed RMS terrain and RNG");
        if(c[1]) check(hash(map.elevation)==c[3] && hash(map.slopes)==c[4],"original composed RMS heights and slopes");
        else check(map.elevation.empty() && map.slopes.empty(),"absent elevation stage");
    }
    for(const auto &invalid:{"create_elevation 8 { base_terrain 6 }","create_elevation -1 { base_terrain 6 }",
        "create_elevation 1 { }","create_elevation 1 { base_terrain 256 }",
        "create_elevation 1 { base_terrain 6 number_of_clumps -1 }",
        "create_elevation 1 { base_terrain 6 number_of_tiles 2147483647 set_scale_by_groups }"}) {
        load(land+"<ELEVATION_GENERATION> "+invalid);uint32_t rng=1;
        check(!generateRmsMap(script,{},16,16,data,rng,map,&error) && rng==1 && map.terrain.empty(),"invalid elevation fails atomically");
    }
    for(bool groups:{false,true}) {
        const std::string prefix=land+"<ELEVATION_GENERATION> create_elevation 2 { base_terrain 6 ";
        load(prefix+"number_of_tiles 700 number_of_clumps 100 "+(groups?"set_scale_by_groups":"set_scale_by_size")+" }");
        uint32_t a=1;RmsMap scaled;check(generateRmsMap(script,{},16,16,data,a,scaled,&error),error);
        load(prefix+(groups?"number_of_tiles 17 number_of_clumps 100":"number_of_tiles 700 number_of_clumps 2")+" }");
        uint32_t b=1;check(generateRmsMap(script,{},16,16,data,b,map,&error),error);
        check(a==b && map.elevation==scaled.elevation && map.slopes==scaled.slopes,"original elevation scales tiles or clumps separately");
    }
}
