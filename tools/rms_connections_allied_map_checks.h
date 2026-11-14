// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "engine/rms_connections.h"
// Original settings parser, actual allied diplomacy query + selection, and
// composed original LAND/(ELEVATION)/TERRAIN/CONNECTION scheduler.
// Private rms-allied-land-elevation-terrain-connections-oracle-2026-11-14.
static void checkRmsAlliedConnectionsMap() {
 using namespace swgb;
 dat::DatFile data;data.terrainBlock.terrains.resize(7);data.terrainBlock.terrainsUsed=7;
 for(int id:{0,3,6})data.terrainBlock.terrains[id].enabled=1;
 const std::string land="<LAND_GENERATION> base_terrain 0 create_player_lands { terrain_type 6 base_size 3 number_of_tiles 60 clumping_factor 8 border_fuzziness 20 } ";
 const std::string hill="<ELEVATION_GENERATION> create_elevation 2 { base_terrain 6 number_of_tiles 40 number_of_clumps 2 } ";
 const std::string paint="<TERRAIN_GENERATION> create_terrain 3 { base_terrain 6 number_of_tiles 30 number_of_clumps 3 spacing_to_other_terrain_types 1 clumping_factor 10 set_flat_terrain_only } ";
 const std::string connect="<CONNECTION_GENERATION> create_connect_all_players_land { replace_terrain 0 3 replace_terrain 6 3 terrain_cost 0 2 terrain_size 0 2 1 terrain_size 6 0 0 } create_connect_teams_lands { replace_terrain 3 6 terrain_cost 3 3 terrain_size 3 0 0 } ";
 auto hash=[](const std::vector<uint8_t> &a){uint32_t h=2166136261u;for(auto v:a)h=(h^v)*16777619u;return h;};
 const uint32_t cases[][7]={
  {1u,0u,2421829089u,2919830680u,2166136261u,2166136261u,2u},
  {1u,1u,1068483884u,1957169301u,521235909u,521235909u,2u},
  {12345u,0u,2663986606u,3744207316u,2166136261u,2166136261u,2u},
  {12345u,1u,2663986606u,2739743952u,521235909u,521235909u,2u},
  {4294967295u,0u,1598444019u,1751990573u,2166136261u,2166136261u,2u},
  {4294967295u,1u,1598444019u,4192396729u,521235909u,521235909u,2u},
  {1u,0u,2622029702u,891581341u,2166136261u,2166136261u,4u},
  {1u,1u,1237465157u,2275079441u,521235909u,521235909u,4u},
  {12345u,0u,1951223305u,878487276u,2166136261u,2166136261u,4u},
  {12345u,1u,2891252991u,2831376011u,521235909u,521235909u,4u},
 };
 for(const auto &c:cases) {
  auto source=land+(c[1]?hill:std::string())+paint+connect;
  RmsScript script;std::string error;RmsMap map;
  check(script.load(0,[&](int32_t id,std::vector<uint8_t> &bytes){
   if(id)return false;bytes.assign(source.begin(),source.end());return true;
  },{},&error),error);
  std::vector<std::vector<uint8_t>> diplomacy(c[6]+1,std::vector<uint8_t>(c[6]+1,3));
  for(size_t player=1;player<diplomacy.size();++player)diplomacy[player][player]=0;
  diplomacy[2][1]=0;
  if(c[6]==2)diplomacy[1][2]=0;else diplomacy[4][3]=0;
  uint32_t rng=c[0];
  check(generateRmsMap(script,{},32,32,data,rng,map,&error,c[6],{},diplomacy),error);
  check(rng==c[2] && hash(map.terrain)==c[3],"original allied composed terrain/RNG");
  check(hash(map.elevation)==c[4] && hash(map.slopes)==c[5],"allied connections preserve elevation/slopes");
  if(c[0]==1 && c[6]==4 && !c[1]) {
   // Earlier-to-later allies leave connection ordinals distinct, but change
   // original fixed player placement. Reject them too, before any RNG work.
   auto reverse=diplomacy;
   for(size_t player=1;player<reverse.size();++player)
    for(size_t other=1;other<reverse.size();++other)reverse[player][other]=player<=other?0:3;
   std::vector<int> teams;check(resolveRmsConnectionTeams(reverse,teams),"reverse diplomacy");
   check(teams==std::vector<int>({0,1,2,3,4}),"reverse allies keep distinct connection teams");
   rng=c[0];check(!generateRmsMap(script,{"FIXED_POSITIONS"},32,32,data,rng,map,&error,4,{},reverse),"reject allied fixed starts");
   check(rng==c[0] && map.terrain.empty() && map.playerStarts.empty() &&
     error.find("fixed-position")!=std::string::npos,"fixed rejection preserves RNG and clears output");
   for(bool wrongSize:{false,true}) {
    auto invalid=diplomacy;
    if(wrongSize)invalid.pop_back();else invalid[2].pop_back();
    map.terrain={7};rng=c[0];
    check(!generateRmsMap(script,{},32,32,data,rng,map,&error,4,{},invalid),"reject invalid diplomacy matrix");
    check(rng==c[0] && map.terrain.empty() && error.find("diplomacy")!=std::string::npos,"invalid matrix rolls back");
   }
  }
 }
}
