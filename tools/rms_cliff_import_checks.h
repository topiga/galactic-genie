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
}
