// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "rms_connections_allied_map_checks.h"
// Original scheduler LAND/(ELEVATION)/TERRAIN/CONNECTIONS composition, two
// original-verified randomized player starts; empty-object final draw separate.
// Local rms-land-elevation-terrain-connections-oracle-2026-11-14 documents the
// non-scenery rectangle painter/unused-connectivity/visual harness boundaries.
static void checkRmsConnectionsMap() {
 checkRmsAlliedConnectionsMap();
 using namespace swgb;
 dat::DatFile data;data.terrainBlock.terrains.resize(7);data.terrainBlock.terrainsUsed=7;
 for(int id:{0,3,6})data.terrainBlock.terrains[id].enabled=1;
 const std::string land="<LAND_GENERATION> base_terrain 0 create_player_lands { terrain_type 6 base_size 3 number_of_tiles 60 clumping_factor 8 border_fuzziness 20 } ";
 const std::string hill="<ELEVATION_GENERATION> create_elevation 2 { base_terrain 6 number_of_tiles 40 number_of_clumps 2 } ";
 const std::string paint="<TERRAIN_GENERATION> create_terrain 3 { base_terrain 6 number_of_tiles 30 number_of_clumps 3 spacing_to_other_terrain_types 1 clumping_factor 10 set_flat_terrain_only } ";
 const std::string connect="<CONNECTION_GENERATION> create_connect_all_players_land { replace_terrain 0 3 replace_terrain 6 3 terrain_cost 0 2 terrain_size 0 2 1 terrain_size 6 0 0 } create_connect_teams_lands { replace_terrain 3 6 terrain_cost 3 3 terrain_size 3 0 0 } ";
 auto hash=[](const std::vector<uint8_t> &a){uint32_t h=2166136261u;for(auto v:a)h=(h^v)*16777619u;return h;};
 const uint32_t cases[][10]={
  {1u,0u,4259500469u,4034287389u,2166136261u,2166136261u,18u,5u,13u,20u},
  {1u,1u,2883117480u,4269819755u,521235909u,521235909u,18u,5u,13u,20u},
  {12345u,0u,727088494u,2580742486u,2166136261u,2166136261u,5u,20u,20u,11u},
  {12345u,1u,727088494u,2597059203u,521235909u,521235909u,5u,20u,20u,11u},
  {4294967295u,0u,3851874315u,752019259u,2166136261u,2166136261u,20u,10u,5u,21u},
  {4294967295u,1u,1954158615u,3540526962u,521235909u,521235909u,20u,10u,5u,21u},
 };
 for(const std::string &ending:{std::string(),std::string(" <OBJECTS_GENERATION>")}) {
  RmsMap maps[2];uint32_t states[2]={1,1};
  for(int braced=0;braced<2;++braced) {
   const auto source=land+"<CONNECTION_GENERATION> create_connect_all_players_land"+(braced?" {}":"")+ending;
   RmsScript script;std::string error;
   check(script.load(0,[&](int32_t id,std::vector<uint8_t> &bytes){if(id)return false;bytes.assign(source.begin(),source.end());return true;},{},&error),error);
   check(generateRmsMap(script,{},32,32,data,states[braced],maps[braced],&error),error);
  }
  check(states[0]==states[1] && maps[0].terrain==maps[1].terrain,"Original bare connection retains default settings at EOF or before a section");
 }
 for(const std::string &suffix:{std::string(" { terrain_cost 0 2"),std::string(" terrain_cost 0 2")}) {
  const auto source=land+"<CONNECTION_GENERATION> create_connect_all_players_land"+suffix;
  RmsScript script;std::string error;RmsMap map;uint32_t rng=1;
  const bool loaded=script.load(0,[&](int32_t id,std::vector<uint8_t> &bytes){if(id)return false;bytes.assign(source.begin(),source.end());return true;},{},&error);
  check((!loaded || !generateRmsMap(script,{},32,32,data,rng,map,&error)) && rng==1 && map.terrain.empty(),"Malformed explicit connection settings reject atomically");
 }
 for(const auto &c:cases) for(bool late:{false,true}) {
  // Section order must not change the scheduler's stage priority.
  auto source=land+(late?connect:std::string())+(c[1]?hill:std::string())+paint+
      (late?std::string():connect);
  RmsScript script;std::string error;RmsMap map;
  check(script.load(0,[&](int32_t id,std::vector<uint8_t> &bytes){
   if(id)return false;bytes.assign(source.begin(),source.end());return true;
  },{},&error),error);
  uint32_t rng=c[0];check(generateRmsMap(script,{},32,32,data,rng,map,&error),error);
  check(rng==c[2] && hash(map.terrain)==c[3],"original composed connection terrain/RNG");
  check(hash(map.elevation)==c[4] && hash(map.slopes)==c[5],"connection preserves composed elevation/slopes");
  check(map.playerStarts.size()==2 && map.playerStarts[0].x==int(c[6]) &&
    map.playerStarts[0].y==int(c[7]) && map.playerStarts[1].x==int(c[8]) &&
    map.playerStarts[1].y==int(c[9]),"original connection owner centers");
 }
}
