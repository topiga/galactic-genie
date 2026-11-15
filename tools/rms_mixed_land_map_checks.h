// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Original full land parser/finalizer -> original scheduler LAND/TERRAIN/CONNECTIONS.
// Private rms-mixed-land-geometry-oracle-2026-11-14 documents callback boundaries.
static void checkRmsMixedLandMap() {
 using namespace swgb;
 dat::DatFile data;data.terrainBlock.terrains.resize(7);data.terrainBlock.terrainsUsed=7;
 for(int id:{0,3,6})data.terrainBlock.terrains[id].enabled=1;
 struct Case { const char *pattern;bool explicitPosition,fixed;uint32_t seed,rng,hash;
   std::vector<RmsMapLand> lands; bool bordered=false; };
 const Case cases[]={
  // Original parser/finalizer and full geometry scheduler with asymmetric player borders.
  {"PL",false,false,1u,3730534061u,2002963911u,{{{18,13},1,1},{{17,21},1,2},{{11,5},0,0}},true},
  {"LP",false,false,1u,2446670145u,194102839u,{{{11,5},0,0},{{18,13},1,1},{{17,21},1,2}},true},
  {"PLL",false,false,1u,3269723840u,971035952u,{{{18,13},1,1},{{17,21},1,2},{{11,5},0,0},{{14,4},0,0}},true},
  {"LPL",false,false,1u,412038212u,2677160795u,{{{11,5},0,0},{{18,13},1,1},{{17,21},1,2},{{14,4},0,0}},true},
  {"PL",false,false,12345u,2854476751u,2616810034u,{{{10,23},1,1},{{21,15},1,2},{{8,11},0,0}},true},
  {"LP",false,false,12345u,3737628077u,2396644978u,{{{8,11},0,0},{{10,23},1,1},{{21,15},1,2}},true},
  {"PLL",false,false,12345u,49349888u,388353435u,{{{10,23},1,1},{{21,15},1,2},{{8,11},0,0},{{14,3},0,0}},true},
  {"LPL",false,false,12345u,3537413851u,2088109215u,{{{8,11},0,0},{{10,23},1,1},{{21,15},1,2},{{14,3},0,0}},true},
  {"PL",false,true,1u,2601248637u,1924379209u,{{{21,13},1,1},{{15,23},1,2},{{11,5},0,0}},true},
  {"LP",false,true,1u,194215839u,1002063173u,{{{11,5},0,0},{{21,13},1,1},{{15,23},1,2}},true},
  {"PLL",false,true,1u,1538889932u,2269637328u,{{{21,13},1,1},{{15,23},1,2},{{11,5},0,0},{{14,4},0,0}},true},
  {"LPL",false,true,1u,797187600u,2619444601u,{{{11,5},0,0},{{21,13},1,1},{{15,23},1,2},{{14,4},0,0}},true},
  {"PL",false,true,12345u,3621606893u,2043618816u,{{{13,24},1,1},{{21,12},1,2},{{10,20},0,0}},true},
  {"LP",false,true,12345u,1995999676u,4212520775u,{{{10,20},0,0},{{13,24},1,1},{{21,12},1,2}},true},
  {"PLL",false,true,12345u,3630617685u,2007497634u,{{{13,24},1,1},{{21,12},1,2},{{10,20},0,0},{{26,20},0,0}},true},
  {"LPL",false,true,12345u,887252540u,3037875008u,{{{10,20},0,0},{{13,24},1,1},{{21,12},1,2},{{26,20},0,0}},true},
  {"PL",false,false,1u,389168431u,1430324621u,{{{18,5},1,1},{{13,20},1,2},{{16,10},0,0}}},
  {"PL",true,false,1u,259500580u,3813667181u,{{{18,5},1,1},{{13,20},1,2},{{16,16},0,0}}},
  {"LP",false,false,1u,4230473717u,2432034677u,{{{16,10},0,0},{{18,5},1,1},{{13,20},1,2}}},
  {"LP",true,false,1u,1563097828u,3618920279u,{{{16,16},0,0},{{18,5},1,1},{{13,20},1,2}}},
  {"PLL",false,false,1u,16267981u,2537930819u,{{{18,5},1,1},{{13,20},1,2},{{16,10},0,0},{{11,5},0,0}}},
  {"LPL",false,false,1u,3041927395u,320459634u,{{{16,10},0,0},{{18,5},1,1},{{13,20},1,2},{{11,5},0,0}}},
  {"PL",false,false,12345u,462096214u,2759840788u,{{{5,20},1,1},{{20,11},1,2},{{8,11},0,0}}},
  {"LP",false,false,12345u,4136115793u,168452932u,{{{8,11},0,0},{{5,20},1,1},{{20,11},1,2}}},
  {"PLL",false,false,12345u,3630617685u,4160332618u,{{{5,20},1,1},{{20,11},1,2},{{8,11},0,0},{{14,3},0,0}}},
  {"LPL",false,false,12345u,60543112u,3506872491u,{{{8,11},0,0},{{5,20},1,1},{{20,11},1,2},{{14,3},0,0}}},
  {"PL",false,true,1u,1656068489u,2751319966u,{{{25,8},1,1},{{10,24},1,2},{{11,5},0,0}}},
  {"PL",true,true,1u,4039482443u,3311351663u,{{{25,8},1,1},{{10,24},1,2},{{16,16},0,0}}},
  {"LP",false,true,1u,996532122u,2138984529u,{{{11,5},0,0},{{25,8},1,1},{{10,24},1,2}}},
  {"LP",true,true,1u,107325602u,861763646u,{{{16,16},0,0},{{25,8},1,1},{{10,24},1,2}}},
  {"PLL",false,true,1u,4094222005u,1974075738u,{{{25,8},1,1},{{10,24},1,2},{{11,5},0,0},{{14,4},0,0}}},
  {"LPL",false,true,1u,115941666u,1449452384u,{{{11,5},0,0},{{25,8},1,1},{{10,24},1,2},{{14,4},0,0}}},
  {"PL",false,true,12345u,1268107786u,2564229730u,{{{5,22},1,1},{{22,3},1,2},{{10,20},0,0}}},
  {"LP",false,true,12345u,2857285784u,4183946536u,{{{10,20},0,0},{{5,22},1,1},{{22,3},1,2}}},
  {"PLL",false,true,12345u,3147649256u,2939326726u,{{{5,22},1,1},{{22,3},1,2},{{10,20},0,0},{{26,20},0,0}}},
  {"LPL",false,true,12345u,2371569151u,1246399738u,{{{10,20},0,0},{{5,22},1,1},{{22,3},1,2},{{26,20},0,0}}},
 };

 const std::string paint="<TERRAIN_GENERATION> create_terrain 3 { base_terrain 6 number_of_tiles 30 number_of_clumps 3 spacing_to_other_terrain_types 1 clumping_factor 10 set_flat_terrain_only } ";
 const std::string connect="<CONNECTION_GENERATION> create_connect_all_players_land { replace_terrain 0 3 replace_terrain 6 3 terrain_cost 0 2 terrain_size 0 2 1 terrain_size 6 0 0 } ";
 for(const auto &c:cases) {
  std::string source="<LAND_GENERATION> base_terrain 0 ";
  const std::string player="create_player_lands { terrain_type 6 base_size 3 number_of_tiles 60 "+
   std::string(c.bordered?"left_border 20 right_border 5 top_border 30 bottom_border 10 ":"")+"} ";
  const std::string neutral="create_land { terrain_type 3 base_size 1 number_of_tiles 40 "+
    std::string(c.explicitPosition?"land_position 50 50 ":"")+"} ";
  for(const char *p=c.pattern;*p;++p)source+=*p=='P'?player:neutral;
  source+=paint+connect;
  RmsScript script;RmsMap map;std::string error;
  check(script.load(0,[&](int32_t id,std::vector<uint8_t> &bytes){
   if(id)return false;bytes.assign(source.begin(),source.end());return true;
  },{},&error),error);
  std::unordered_set<std::string> defines;if(c.fixed)defines.insert("FIXED_POSITIONS");
  uint32_t rng=c.seed;check(generateRmsMap(script,defines,32,32,data,rng,map,&error),error);
  uint32_t hash=2166136261u;for(auto v:map.terrain)hash=(hash^v)*16777619u;
  check(hash==c.hash && rng==c.rng,"original mixed-land geometry/RNG");
  check(map.lands.size()==c.lands.size() && map.playerStarts.size()==2,"mixed land ownership count");
  size_t playerIndex=0;
  for(size_t i=0;i<c.lands.size();++i) {
   const auto &expected=c.lands[i];const auto &actual=map.lands[i];
   check(actual.id==expected.id && actual.owner==expected.owner &&
     actual.center.x==expected.center.x && actual.center.y==expected.center.y,"original mixed land metadata/order");
   if(expected.owner>0) {
    check(map.playerStarts[playerIndex].x==expected.center.x && map.playerStarts[playerIndex].y==expected.center.y,"owned start uses owner not row ordinal");
    ++playerIndex;
   }
  }
 }
 for(const char *bounds:{"left_border -1","right_border 101","left_border 100",
     "left_border 60 right_border 60","left_border 25 right_border 25 base_size 13","bottom_border 100"}) {
  const std::string source="<LAND_GENERATION> base_terrain 0 create_player_lands { terrain_type 6 base_size 3 number_of_tiles 60 "+std::string(bounds)+" }";
  RmsScript script;RmsMap map;std::string error;
  check(script.load(0,[&](int32_t id,std::vector<uint8_t> &bytes){if(id)return false;bytes.assign(source.begin(),source.end());return true;},{},&error),error);
  for(bool fixed:{false,true}) {
   std::unordered_set<std::string> defines;if(fixed)defines.insert("FIXED_POSITIONS");
   uint32_t rng=123;map.terrain={1};
   check(!generateRmsMap(script,defines,32,32,data,rng,map,&error) && rng==123 && map.terrain.empty() && !error.empty(),"invalid player borders must fail atomically");
  }
 }
}
