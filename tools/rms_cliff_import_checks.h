// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
static void checkRmsCliffImport(swgb::Assets &assets) {
    using namespace swgb;
    const std::string source="<LAND_GENERATION> base_terrain 6 create_land { terrain_type 6 land_position 50 50 base_size 0 number_of_tiles 0 } <CLIFF_GENERATION>";
    RmsScript script; RmsMap map; std::string error;
    check(script.load(0,[&](int32_t id,std::vector<uint8_t> &out) {
        if(id) return false;out.assign(source.begin(),source.end());return true;
    },{},&error),error.c_str());
    uint32_t rng=1;
    check(generateRmsMap(script,{},120,120,assets.dat(),rng,map,&error),error.c_str());
    Game game(assets);rng=1;
    check(game.initRms(script,{},120,rng,&error),error.c_str());
    check(rng==1150416911u && game.activeObjectCount()==map.cliffs.size(),"original cliff native import count/RNG");
    for(const auto &o:map.cliffs) {
        const uint32_t id=o.creationOrdinal+1;
        const auto p=game.objectPosition(id);
        check(game.objectUnitId(id)==o.unit && game.objectPlayer(id)==0 && p[0]==o.x && p[1]==o.y,"native cliff original IDs/types/positions");
        const float pi=3.14159265358979323846f,theta=pi*.5f+o.facet*(2*pi/25);
        const float sx=std::cos(theta)*.5f,sy=std::sin(theta);
        const float expected=std::atan2(sy-sx,sy+sx),actual=game.objectFacing(id);
        check(std::abs(std::sin(actual)-std::sin(expected))<.00001f &&
              std::abs(std::cos(actual)-std::cos(expected))<.00001f,"native cliff original facet projection");
    }
    struct RaisedCase { int levels; uint32_t rng,heightHash; size_t count; };
    const RaisedCase cases[]={{1,2269132341u,2088011581u,47},{7,2215531909u,4157776466u,33}};
    for(const auto &c:cases) {
        const std::string raised=source+" <ELEVATION_GENERATION> create_elevation "+std::to_string(c.levels)+
            " { base_terrain 6 number_of_tiles 8000 number_of_clumps 30 }";
        check(script.load(0,[&](int32_t id,std::vector<uint8_t> &out) {
            if(id) return false;out.assign(raised.begin(),raised.end());return true;
        },{},&error),error.c_str());
        rng=1;check(generateRmsMap(script,{},120,120,assets.dat(),rng,map,&error),error.c_str());
        rng=1;check(game.initRms(script,{},120,rng,&error),error.c_str());
        check(rng==c.rng && game.activeObjectCount()==c.count,"elevated cliff native import count/RNG");
        uint32_t heightHash=2166136261u;
        for(const auto &o:map.cliffs) {
            const auto p=game.objectPosition(o.creationOrdinal+1);
            check(p[0]==o.x && p[1]==o.y,"elevated cliff native import positions");
            const float height=game.elevationForTesting(p[0],p[1]);
            check(std::isfinite(height) && height>=0 && height<=c.levels && height==std::floor(height),
                "original cliff creation height is an exact level");
            heightHash=(heightHash^(o.creationOrdinal+1))*16777619u;
            heightHash=(heightHash^uint32_t(height*256))*16777619u;
        }
        check(heightHash==c.heightHash,"native cliff rendering heights match original object z values");
    }
    const int forestTypes[][3]={{21,413,12},{18,348,12},{20,349,11}};
    for(const auto &spec:forestTypes) {
        const std::string forest=source+" <TERRAIN_GENERATION> create_terrain "+std::to_string(spec[0])+" { base_terrain 6 number_of_tiles 2000 number_of_clumps 20 spacing_to_other_terrain_types 2 clumping_factor 20 }";
        check(script.load(0,[&](int32_t id,std::vector<uint8_t> &out) {
            if(id) return false;out.assign(forest.begin(),forest.end());return true;
        },{},&error),error.c_str());
        rng=1;check(generateRmsMap(script,{},120,120,assets.dat(),rng,map,&error),error.c_str());
        rng=1;check(game.initRms(script,{},120,rng,&error),error.c_str());
        check(rng==1567879473u && game.activeObjectCount()==2065,"cliff/forest native import count/RNG");
        uint32_t nextId=map.cliffs.back().creationOrdinal+2;
        for(const auto &tree:map.scenery) {
            const auto p=game.objectPosition(nextId);
            check(game.objectUnitId(nextId)==spec[1] && game.objectPlayer(nextId)==0 && p[0]==tree.x && p[1]==tree.y,
                "native trees preserve original IDs after cliff replacements and rejected overlaps");
            const float pi=3.14159265358979323846f,theta=pi*.5f+tree.facet*(2*pi/spec[2]);
            const float sx=std::cos(theta)*.5f,sy=std::sin(theta),expected=std::atan2(sy-sx,sy+sx);
            const float actual=game.objectFacing(nextId);
            check(std::abs(std::sin(actual)-std::sin(expected))<.00001f &&
                std::abs(std::cos(actual)-std::cos(expected))<.00001f,"native forest import preserves original11/12-angle facets");
            ++nextId;
        }
    }
}
