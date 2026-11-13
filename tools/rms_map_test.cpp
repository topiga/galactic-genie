// SPDX-License-Identifier: GPL-3.0-or-later
#include "engine/rms_map.h"
#include "core/genie_dat.h"
#include <cstdlib>
#include <iostream>

static void check(bool ok,const std::string &message) {
    if (!ok) { std::cerr<<message<<'\n'; std::exit(1); }
}
int main() {
    using namespace swgb;
    dat::DatFile data;
    data.terrainBlock.terrains.resize(4);
    data.terrainBlock.terrainsUsed=4;
    data.terrainBlock.terrains[0].enabled=data.terrainBlock.terrains[3].enabled=1;
    const std::string land=R"(<LAND_GENERATION> base_terrain 0
        create_land { terrain_type 3 land_position 30 30 base_size 0 number_of_tiles 10 border_fuzziness 0 zone 0 }
        create_land { terrain_type 3 land_position 60 60 base_size 0 number_of_tiles 10 border_fuzziness 0 zone 1 })";
    RmsScript script;
    RmsMap map;
    std::string error;
    auto load=[&](const std::string &source) {
        check(script.load(0,[&](int32_t id,std::vector<uint8_t> &bytes) {
            if (id!=0) return false;
            bytes.assign(source.begin(),source.end()); return true;
        },{},&error),error);
    };
    load(land);
    uint32_t state=1;
    check(generateRmsMap(script,{},7,7,data,state,map,&error),error);
    uint32_t hash=2166136261u;
    for (uint8_t tile:map.terrain) hash=(hash^tile)*16777619u;
    // Original composed-stage oracle: object entry827320339, no object rows;
    // only the final scheduler log draw follows. TerrainFNV from original stages.
    check(hash==1509571036u && state==1578431114u && map.timber.empty(),"original empty-object pipeline mismatch");
    state=2745024u; // State after the first original CRT draw from seed1.
    RmsMap expected;
    check(generateRmsMap(script,{},7,7,data,state,expected,&error),error);
    const auto expectedState=state;
    const std::string randomPrefix="start_random percent_chance 100 #define PICK end_random ";
    load(randomPrefix+land);
    state=1;
    check(generateRmsMap(script,{},7,7,data,state,map,&error) && state==expectedState && map.terrain==expected.terrain,
          "preprocessing and generation must share one RNG stream");
    load(randomPrefix+land+" <ELEVATION_GENERATION>");
    state=1;
    check(!generateRmsMap(script,{},7,7,data,state,map,&error) && state==1 && map.terrain.empty(),
          "unsupported stage must not return partial map/RNG");
    auto missing=land;
    missing.erase(missing.find("border_fuzziness 0"),std::string("border_fuzziness 0").size());
    load(missing);
    state=1;
    check(generateRmsMap(script,{},7,7,data,state,map,&error) && state==1578431114u,
          "default fuzziness20 must be accepted with original full-map bounds");
    state=1;
    load(land);
    data.terrainBlock.terrains[3].numTerrainUnitsUsed=1;
    check(!generateRmsMap(script,{},7,7,data,state,map,&error) && state==1,
          "automatic terrain objects must not be silently omitted");
    data.terrainBlock.terrains[3].numTerrainUnitsUsed=0;
    check(!generateRmsMap(script,{},0,7,data,state,map,&error),"invalid map dimensions");
    load(land+" <OBJECTS_GENERATION> create_object 83 { number_of_objects 1 }");
    check(!generateRmsMap(script,{},7,7,data,state,map,&error),"unsupported unit type");
    load(land+" <OBJECTS_GENERATION> create_object 348 { number_of_objects 1 }");
    check(!generateRmsMap(script,{},7,7,data,state,map,&error) && state==1 && map.terrain.empty(),
          "late prototype failure must not commit generated terrain or RNG");
    const std::string players="<LAND_GENERATION> base_terrain 0 create_player_lands { terrain_type 3 base_size 3 number_of_tiles 60 }";
    load(players);
    state=1;
    check(!generateRmsMap(script,{"FIXED_POSITIONS"},32,32,data,state,map,&error) && state==1,
          "fixed-position context must not silently use randomized starts");
    check(generateRmsMap(script,{},32,32,data,state,map,&error),error);
    check(map.playerStarts.size()==2 && map.playerStarts[0].x==18 && map.playerStarts[0].y==5 &&
          map.playerStarts[1].x==13 && map.playerStarts[1].y==20,"original owner-ordered player starts");
    const auto playerTerrain=map.terrain;
    const auto playerState=state;
    load("<PLAYER_SETUP> random_placement "+players); state=1;
    check(generateRmsMap(script,{},32,32,data,state,map,&error) && state==playerState && map.terrain==playerTerrain,
          "RMS random_placement directive is a no-op, not a game-setting setter");
    auto percent=players;
    percent.replace(percent.find("number_of_tiles 60"),18,"land_percent 12");
    auto count=players;
    count.replace(count.find("number_of_tiles 60"),18,"number_of_tiles 61");
    load(percent); state=1;
    check(generateRmsMap(script,{},32,32,data,state,expected,&error),error);
    const auto percentState=state;
    load(count); state=1;
    check(generateRmsMap(script,{},32,32,data,state,map,&error) && state==percentState && map.terrain==expected.terrain,
          "land percent divides total area across both player lands");
    load(players+" create_land { terrain_type 3 land_position 50 50 }");
    state=1;
    check(!generateRmsMap(script,{},32,32,data,state,map,&error) && state==1 && map.playerStarts.empty(),
          "mixed player and nonplayer placement remains unsupported");
    load("<LAND_GENERATION> base_terrain 0 create_player_lands { terrain_type 3 land_position 50 50 }");
    check(!generateRmsMap(script,{},32,32,data,state,map,&error),"player lands must not accept explicit positions");
    std::cout<<"RMS integrated map tests passed\n";
}
