// SPDX-License-Identifier: GPL-3.0-or-later
#include "engine/rms_map.h"
#include "core/genie_dat.h"
#include <cstdlib>
#include <iostream>
#include <limits>

static void check(bool ok,const std::string &message) {
    if (!ok) { std::cerr<<message<<'\n'; std::exit(1); }
}
#include "rms_many_owned_checks.h"

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
    check(hash==1509571036u && state==1578431114u && map.objects.empty(),"original empty-object pipeline mismatch");
    auto bordered=land;
    for (size_t pos=0;(pos=bordered.find("border_fuzziness 0",pos))!=std::string::npos;) {
        const std::string bounds="border_fuzziness 100 left_border 25 right_border 25 top_border 25 bottom_border 25";
        bordered.replace(pos,std::string("border_fuzziness 0").size(),bounds); pos+=bounds.size();
    }
    load(bordered);
    const uint32_t seeds[]={1,12345}, hashes[]={2387610166u,204622211u}, states[]={483449721u,3426167425u};
    for (size_t fixture=0;fixture<2;++fixture) {
        state=seeds[fixture];
        check(generateRmsMap(script,{},7,7,data,state,map,&error),error);
        hash=2166136261u;
        for (uint8_t tile:map.terrain) hash=(hash^tile)*16777619u;
        check(hash==hashes[fixture] && state==states[fixture],"original bordered-land pipeline mismatch");
    }
    for (const auto &invalid:{"left_border -1","right_border 101","left_border 100 right_border 100"}) {
        auto source=land; source.insert(source.find("}"),invalid);
        load(source); state=1;
        check(!generateRmsMap(script,{},7,7,data,state,map,&error) && state==1 && map.terrain.empty(),
              "invalid borders must fail transactionally");
    }
    auto separated=land;
    for (size_t pos=0;(pos=separated.find("}",pos))!=std::string::npos;) {
        const std::string distance="other_zone_avoidance_distance 2 ";
        separated.insert(pos,distance); pos+=distance.size()+1;
    }
    load(separated);
    const uint32_t separationHashes[]={2659705154u,2623728704u}, separationStates[]={483449721u,1874836596u};
    for (size_t fixture=0;fixture<2;++fixture) {
        state=seeds[fixture];
        check(generateRmsMap(script,{},7,7,data,state,map,&error),error);
        hash=2166136261u;
        for (uint8_t tile:map.terrain) hash=(hash^tile)*16777619u;
        check(hash==separationHashes[fixture] && state==separationStates[fixture],"original separated-land pipeline mismatch");
    }
    load(land);
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
    load("<PLAYER_SETUP> random_placement <LAND_GENERATION> base_terrain 0 "
         "create_player_lands { terrain_type 3 base_size 13 number_of_tiles 0 }");
    state=1993207714u;
    check(generateRmsMap(script,{"FIXED_POSITIONS"},120,120,data,state,map,&error),error);
    check(map.playerStarts.size()==2 && map.playerStarts[0].x==19 && map.playerStarts[0].y==36 &&
          map.playerStarts[1].x==42 && map.playerStarts[1].y==96,
          "fixed game setting overrides no-op directive and matches both live Windows centers");
    load(players);state=1;
    check(generateRmsMap(script,{},32,32,data,state,map,&error),error);
    check(map.playerStarts.size()==2 && map.playerStarts[0].x==18 && map.playerStarts[0].y==5 &&
          map.playerStarts[1].x==13 && map.playerStarts[1].y==20,"original owner-ordered player starts");
    const auto playerTerrain=map.terrain;
    const auto playerState=state;
    const uint32_t counts[]={1,3,8}, countHashes[]={2429689585u,1986512803u,2929584340u},
                   countStates[]={940038285u,2511621164u,1668072822u};
    for (size_t fixture=0;fixture<3;++fixture) {
        state=1;
        check(generateRmsMap(script,{},32,32,data,state,map,&error,counts[fixture]),error);
        hash=2166136261u;
        for (uint8_t tile:map.terrain) hash=(hash^tile)*16777619u;
        check(map.playerStarts.size()==counts[fixture] && hash==countHashes[fixture] && state==countStates[fixture],
              "original multiplayer land pipeline mismatch");
    }
    for (uint32_t invalid:{0u,9u,UINT32_MAX}) {
        state=1;
        check(!generateRmsMap(script,{},32,32,data,state,map,&error,invalid) && state==1 && map.terrain.empty(),
              "invalid player count must fail transactionally");
    }
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
    count.replace(count.find("number_of_tiles 61"),18,"number_of_tiles 15");
    load(percent); state=1;
    check(generateRmsMap(script,{},32,32,data,state,expected,&error,8),error);
    const auto eightPlayerState=state;
    load(count); state=1;
    check(generateRmsMap(script,{},32,32,data,state,map,&error,8) && state==eightPlayerState && map.terrain==expected.terrain,
          "land percent divides total area across configured player count");
    load(players+" create_land { terrain_type 3 land_position 50 50 }");
    state=1;
    check(!generateRmsMap(script,{},32,32,data,state,map,&error) && state==1 && map.playerStarts.empty(),
          "mixed player and nonplayer placement remains unsupported");
    load("<LAND_GENERATION> base_terrain 0 create_player_lands { terrain_type 3 land_position 50 50 }");
    check(!generateRmsMap(script,{},32,32,data,state,map,&error),"player lands must not accept explicit positions");
    const std::string terrainRow=" <TERRAIN_GENERATION> create_terrain 3 { base_terrain 0 number_of_tiles 37 number_of_clumps 4 spacing_to_other_terrain_types 1 } create_terrain 0 { base_terrain 3 number_of_tiles 9 number_of_clumps 2 clumping_factor 6 }";
    load(players+terrainRow); state=1;
    check(generateRmsMap(script,{},32,32,data,state,map,&error),error);
    hash=2166136261u;
    for (uint8_t tile:map.terrain) hash=(hash^tile)*16777619u;
    check(hash==1549600828u && state==uint32_t(1957819393u*214013u+2531011u),
          "original land and terrain row pipeline mismatch");
    load(players+" <TERRAIN_GENERATION> create_terrain 3 { base_terrain 0 number_of_tiles 97 number_of_clumps 20 set_scale_by_groups }");
    state=1;
    check(generateRmsMap(script,{},32,32,data,state,expected,&error),error);
    const auto scaledState=state;
    load(players+" <TERRAIN_GENERATION> create_terrain 3 { base_terrain 0 number_of_tiles 9 number_of_clumps 2 }");
    state=1;
    check(generateRmsMap(script,{},32,32,data,state,map,&error) && state==scaledState && map.terrain==expected.terrain,
          "terrain scaling uses map area over10000 for tile and group counts");
    auto protectedTerrain=terrainRow;
    protectedTerrain.insert(protectedTerrain.find("spacing_to_other_terrain_types"),"set_avoid_player_start_areas ");
    load(players+protectedTerrain); state=1;
    check(generateRmsMap(script,{},32,32,data,state,map,&error),error);
    hash=2166136261u;
    for (uint8_t tile:map.terrain) hash=(hash^tile)*16777619u;
    check(hash==3452956004u && state==uint32_t(1656093246u*214013u+2531011u),
          "original fixed-radius player protection and terrain pipeline mismatch");
    for (const auto &invalid:{"create_terrain 3 { number_of_tiles 2 }",
                            "create_terrain 256 { base_terrain 3 }",
                            "create_terrain -253 { base_terrain 0 }",
                            "create_terrain 3 { base_terrain 256 }",
                            "create_terrain 3 { base_terrain -256 }",
                            "create_terrain 3 { base_terrain 3 }",
                            "create_terrain 3 { base_terrain 0 unsupported_flag }",
                            "create_terrain 3 { base_terrain 0 number_of_clumps -1 }"}) {
        load(players+" <TERRAIN_GENERATION> "+invalid); state=1;
        check(!generateRmsMap(script,{},32,32,data,state,map,&error) && state==1 && map.terrain.empty(),
              "unsupported terrain row must fail without partial terrain or RNG");
    }
    data.terrainBlock.terrains.resize(45);
    data.terrainBlock.terrainsUsed=45;
    for (int id:{6,9,41,44}) data.terrainBlock.terrains[id].enabled=1;
    const std::string dry="<LAND_GENERATION> base_terrain 41 create_player_lands { terrain_type 9 base_size 3 number_of_tiles 60 } <TERRAIN_GENERATION> create_terrain 6 { base_terrain 41 number_of_tiles 37 number_of_clumps 4 spacing_to_other_terrain_types 1 } create_terrain 44 { base_terrain 9 number_of_tiles 9 number_of_clumps 2 clumping_factor 6 }";
    load(dry); state=1;
    check(generateRmsMap(script,{},32,32,data,state,expected,&error),error);
    hash=2166136261u;
    for (uint8_t tile:expected.terrain) hash=(hash^tile)*16777619u;
    check(hash==2900154801u && state==3532705190u,"original Flats dry-terrain pipeline fixture");
    const auto dryState=state;
    auto &dirt=data.terrainBlock.terrains[6];
    dirt.numTerrainUnitsUsed=1; dirt.terrainUnitId[0]=-1;
    dirt.terrainUnitDensity[0]=32767; dirt.terrainUnitCentering[0]=255;
    state=1;
    check(generateRmsMap(script,{},32,32,data,state,map,&error) && state==dryState && map.terrain==expected.terrain,
          "disabled scenery slot must ignore density/centering and consume no RNG");
    for (int invalid:{-2,0,411}) {
        dirt.terrainUnitId[0]=invalid; state=1;
        check(!generateRmsMap(script,{},32,32,data,state,map,&error) && state==1 && map.terrain.empty(),
              "unverified scenery must not be silently omitted");
    }
    dirt.terrainUnitId[0]=-1; dirt.numTerrainUnitsUsed=31;
    state=1;
    check(!generateRmsMap(script,{},32,32,data,state,map,&error) && state==1,"invalid scenery slot count");
    dirt.numTerrainUnitsUsed=1;
    for (int id:{1,22,32,35,37}) data.terrainBlock.terrains[id].enabled=1;
    data.terrainBlock.terrains[37].terrainToDraw=35;
    const std::string water="<LAND_GENERATION> base_terrain 32 create_player_lands { terrain_type 35 base_size 3 number_of_tiles 60 } <TERRAIN_GENERATION> create_terrain 22 { base_terrain 32 number_of_tiles 37 number_of_clumps 4 spacing_to_other_terrain_types 1 } create_terrain 1 { base_terrain 35 number_of_tiles 9 number_of_clumps 2 clumping_factor 6 }";
    load(water); state=1;
    check(generateRmsMap(script,{},32,32,data,state,map,&error),error);
    hash=2166136261u;
    for (auto tile:map.terrain) hash=(hash^tile)*16777619u;
    check(hash==4080120994u && state==3532705190u,"original water cleanup pipeline raster/RNG");
    data.terrainBlock.terrains[37].terrainToDraw=53;
    state=1;
    check(!generateRmsMap(script,{},32,32,data,state,map,&error) && state==1 && map.terrain.empty(),
          "invalid generated shore alias fails transactionally after cleanup");
    data={};
    data.terrainBlock.terrains.resize(28);
    data.terrainBlock.terrainsUsed=28;
    for (int id:{0,4,6,27}) data.terrainBlock.terrains[id].enabled=1;
    data.civs.resize(9); data.graphics.resize(7833); data.terrainRestrictions.resize(5);
    data.terrainRestrictions[4].passableBuildableDmgMultiplier={1};
    for (uint16_t civ=1;civ<=8;++civ) {
        data.civs[civ].units.resize(110);
        auto &u=data.civs[civ].units[109];
        u.exists=true; u.id=109; u.type=dat::UT_Building; u.cls=18; u.hitPoints=2500; u.lineOfSight=8;
        u.collisionSize[0]=u.collisionSize[1]=2; u.collisionSize[2]=4;
        u.clearanceSize[0]=u.clearanceSize[1]=2; u.terrainRestriction=4; u.hillMode=2;
        u.obstructionType=2; u.obstructionClass=3;
        u.placementTerrain[0]=u.placementTerrain[1]=u.placementSideTerrain[0]=u.placementSideTerrain[1]=-1;
        u.standingGraphic[0]=civ<=6?2271+civ:7824+civ;
        u.constructionGraphic=civ<=6?366+civ:7390+civ;
        u.foundationTerrainId=27; u.techId=22; u.headUnit=621; u.secondaryProjectileUnit=54;
        u.trainTime=100; u.constructionSound=597;
        for (auto &annex:u.annexes) annex.unitId=-1;
        auto &standing=data.graphics[u.standingGraphic[0]], &construction=data.graphics[u.constructionGraphic];
        standing.exists=true; standing.sequenceType=1; standing.frameCount=8; standing.angleCount=1;
        construction.exists=true; construction.sequenceType=11; construction.frameCount=1; construction.angleCount=3;
    }
    const std::string ownedLand="<LAND_GENERATION> base_terrain 0 create_player_lands { terrain_type 0 base_size 3 number_of_tiles 60 }";
    const std::string ownedRow=" <OBJECTS_GENERATION> create_object 109 { set_place_for_every_player number_of_objects 1 min_distance_to_players 0 max_distance_to_players 0 group_placement_radius 18 }";
    load(ownedLand+ownedRow);
    // Original full4ddd40 stage, actual foundation writes and final scheduler draw.
    const uint32_t ownedSeeds[]={1,12345,0xffffffff}, ownedFinal[]={2036296550,2943925683u,764690905};
    const int ownedX[]={18,20,20},ownedY[]={5,11,10};
    const uint32_t ownedHashes[3][3]={{2282508853u,1877077813u,2491377461u},
        {470130693u,461407749u,3220284933u},{2078377669u,3067649733u,3863289541u}};
    for (uint16_t civ=1;civ<=8;++civ) for (int seed=0;seed<3;++seed) {
        state=ownedSeeds[seed];
        check(generateRmsMap(script,{},32,32,data,state,map,&error,1,{civ}),error);
        check(state==ownedFinal[seed] && map.objects.empty() && map.scenery.empty() && map.ownedObjects.size()==1,
              "original owned109 object count/RNG mismatch");
        const auto &cc=map.ownedObjects[0];
        check(cc.owner==1 && cc.civilization==civ && cc.unitId==109 && cc.x==ownedX[seed] && cc.y==ownedY[seed],
              "original owned109 owner/civilization/position mismatch");
        hash=2166136261u;
        for (auto tile:map.terrain) hash=(hash^tile)*16777619u;
        check(hash==ownedHashes[civ==2?1:civ==6?2:0][seed],"original owned109 foundation raster mismatch");
    }
    const uint32_t twoFinal[]={1410736551u,1480076763u,3244827961u};
    const uint32_t twoHash[]={2282508853u,668798869u,2491377461u};
    const int twoX[][2]={{18,13},{5,20},{20,5}},twoY[][2]={{5,20},{20,11},{10,21}};
    for (uint16_t civ=1;civ<=8;++civ) for (int seed=0;seed<3;++seed) {
        state=ownedSeeds[seed];
        check(generateRmsMap(script,{},32,32,data,state,map,&error,2,{1,civ}),error);
        check(state==twoFinal[seed] && map.ownedObjects.size()==2,"original two-owner RNG/count");
        for (size_t owner=0;owner<2;++owner) {
            const auto &cc=map.ownedObjects[owner];
            check(cc.owner==owner+1 && cc.civilization==(owner?civ:1) &&
                  cc.x==twoX[seed][owner] && cc.y==twoY[seed][owner] &&
                  cc.pendingFoundation==(owner?27:255),
                  "original two-owner placement and deferred terrain byte");
        }
        hash=2166136261u;
        for (auto tile:map.terrain) hash=(hash^tile)*16777619u;
        check(hash==twoHash[seed],"original two-owner pre-render terrain raster");
    }
    state=1;
    check(generateRmsMap(script,{},16,16,data,state,map,&error,2,{1,2}) && map.ownedObjects.size()==2 &&
          map.ownedObjects[1].pendingFoundation==27,"Near starts retain raw pending terrain until post-startup rendering");
    manyOwnedRmsMapChecks(data);
    auto rejectsOwned=[&](uint32_t count,const std::vector<uint16_t> &civs) {
        state=12345;
        check(!generateRmsMap(script,{},32,32,data,state,map,&error,count,civs) && state==12345 &&
              map.terrain.empty() && map.ownedObjects.empty() && !error.empty(),"unsupported owned109 must fail atomically");
    };
    rejectsOwned(1,{}); rejectsOwned(1,{0}); rejectsOwned(1,{9}); rejectsOwned(2,{1}); rejectsOwned(3,{1,2});
    for (const auto &row:{"create_object 109 { set_place_for_every_player }",
                         "create_object 109 { set_place_for_every_player number_of_objects 2 min_distance_to_players 0 max_distance_to_players 0 }",
                         "create_object 109 { set_place_for_every_player number_of_objects 1 min_distance_to_players 1 max_distance_to_players 1 }",
                         "create_object 109 { set_gaia_object_only number_of_objects 1 min_distance_to_players 0 max_distance_to_players 0 }"}) {
        load(ownedLand+" <OBJECTS_GENERATION> "+row); rejectsOwned(1,{1});
    }
    load(ownedLand+ownedRow);
    data.civs[1].units[109].annexes[0].unitId=1; rejectsOwned(1,{1});
    data.civs[1].units[109].annexes[0].unitId=-1;
    data.civs[1].units[109].convertTerrain=1; rejectsOwned(1,{1});
    data.civs[1].units[109].convertTerrain=0;
    data.terrainRestrictions[4].passableBuildableDmgMultiplier[0]=std::numeric_limits<float>::infinity();
    rejectsOwned(1,{1}); data.terrainRestrictions[4].passableBuildableDmgMultiplier[0]=1;
    data.terrainBlock.terrains[27].numTerrainUnitsUsed=1;
    data.terrainBlock.terrains[27].terrainUnitId[0]=348; rejectsOwned(1,{1});
    std::cout<<"RMS integrated map tests passed\n";
}
