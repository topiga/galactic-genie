// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cmath>
// Controlled original Forest prefix: forage3, both Nerf rows, then large hunt at16..22.
static void rmsLargeHuntChecks(swgb::Assets &assets,const std::string &dataDir) {
 swgb::ResourceSet resources;swgb::RmsScript script;std::string error;
 check(resources.add(dataDir+"/gamedata_x1.drs",&error) && script.load(54204,resources,&error),error.c_str());
 const std::unordered_set<std::string> defines{"TINY_MAP","FIXED_POSITIONS","DEFAULT_RESOURCES"};
 struct Animation{uint8_t frame;float delay,heading;};
 const struct{int unit,count;bool grouped;uint32_t rng,poses;std::vector<Animation> animations;bool distant=false;} cases[]={
{822,2,true,1777230500u,1964833593u,{{9,0.373876095f,4.71238899f},{2,0.f,3.14159274f},{9,0.0606158301f,1.57079637f},{9,0.477223992f,0.f}}},
{822,4,true,1427887360u,3213888073u,{{9,0.373876095f,4.71238899f},{2,0.f,3.14159274f},{2,0.f,3.92699075f},{2,0.f,1.57079637f},{1,0.f,4.71238899f},{5,0.f,2.3561945f},{4,0.f,4.71238899f},{9,0.f,1.57079637f}}},
{1249,1,false,3550983994u,288820904u,{{6,0.f,0.785398185f},{2,0.f,0.f}}},
{1249,2,false,1585104208u,2409764850u,{{6,0.f,0.785398185f},{9,0.373876095f,4.71238899f},{9,0.f,0.785398185f},{9,0.362825364f,0.785398185f}}},
{822,3,true,3597322543u,4039699298u,{{5,0.f,1.57079637f},{4,0.f,0.785398185f},{9,0.412164629f,0.f},{3,0.f,2.3561945f},{9,0.f,1.57079637f}},true},
{1249,3,true,3597322543u,433503753u,{{5,0.f,1.57079637f},{4,0.f,0.785398185f},{9,0.412164629f,0.f},{3,0.f,2.3561945f},{9,0.f,1.57079637f}},true},
{822,1,false,3550983994u,481104102u,{{6,0.f,0.785398185f},{2,0.f,0.f}}},
{1249,2,true,1777230500u,1048510013u,{{9,0.373876095f,4.71238899f},{2,0.f,3.14159274f},{9,0.0606158301f,1.57079637f},{9,0.477223992f,0.f}}}};
 for(const auto &c:cases) {
  uint32_t rng=16160;std::vector<swgb::RmsToken> tokens;
  check(script.preprocess(defines,tokens,&error,&rng) && rng==1993207714u,"large hunt preprocessing RNG");
  std::string source;unsigned rows=0,forageChanges=0;
  for(size_t i=0;i<tokens.size();) {
   if(tokens[i].text!="create_object"){source+=tokens[i++].text+" ";continue;}
   check(i+2<tokens.size() && tokens[i+2].text=="{","original object block");
   size_t end=i+3;while(end<tokens.size() && tokens[end].text!="}")++end;
   check(end<tokens.size(),"terminated original object block");++rows;
   if(rows<=12) {
    for(size_t t=i;t<=end;++t) {
     if(tokens[i+1].text=="59" && tokens[t].text=="number_of_objects") {
      check(t+1<end && tokens[t+1].text=="12","original forage quantity");tokens[t+1].text="3";++forageChanges;
     }
     source+=tokens[t].text+" ";
    }
   } else if(rows==13) {
    source+="create_object "+std::to_string(c.unit)+" { number_of_objects "+std::to_string(c.count)+" ";
    if(c.grouped)source+="set_loose_grouping ";
    source+="set_gaia_object_only set_place_for_every_player ";
    source+=c.distant?"group_variance 1 min_distance_to_players 35 min_distance_group_placement 5 } ":"min_distance_to_players 16 max_distance_to_players 22 } ";
   }
   i=end+1;
  }
  check(rows==23 && forageChanges==1,"controlled13-row prefix retains later elevation/connections");
  swgb::RmsScript prefix;swgb::RmsMap map;
  check(prefix.load(0,[&](int32_t id,std::vector<uint8_t>&bytes){if(id)return false;bytes.assign(source.begin(),source.end());return true;},{},&error),error.c_str());
  check(swgb::generateRmsMap(prefix,defines,120,120,assets.dat(),rng,map,&error,2,{1,1}),error.c_str());
  check(rng==c.rng && map.scenery.size()==7554 && map.ownedObjects.size()==10 && map.resourcesAfterOwned.size()==75+c.animations.size(),"large hunt original counts/RNG");
  auto hashBytes=[](const auto&bytes){uint32_t hash=2166136261u;for(auto v:bytes)hash=(hash^v)*16777619u;return hash;};
  check(hashBytes(map.terrain)==3348257725u && hashBytes(map.elevation)==908132044u,"large hunt original terrain/elevation");
  uint32_t hash=2166136261u;
  for(const auto&o:map.resourcesAfterOwned)for(auto v:{uint32_t(o.unitId),0u,uint32_t(o.x*2),uint32_t(o.y*2),uint32_t(o.facet)})hash=(hash^v)*16777619u;
  check(hash==c.poses,"large hunt original positions/facets/order including border and neighbour rejection");
  swgb::Game game(assets);rng=1993207714u;
  check(game.initRms(prefix,defines,120,rng,&error,2,{1,1}) && rng==c.rng,error.c_str());
  for(size_t i=0;i<c.animations.size();++i) {
   const auto &o=map.resourcesAfterOwned[75+i];const auto &e=c.animations[i];
   check(o.unitId==c.unit && o.animationFrame==e.frame && std::abs(o.initialIdleDelay-e.delay)<.000001f && o.headingRadians==e.heading,"large hunt original seven-draw constructor");
   const uint32_t id=uint32_t(map.scenery.size()+map.ownedObjects.size()+76+i);const auto a=game.objectAnimationForTesting(id);const auto pos=game.objectPosition(id);
   check(game.objectUnitId(id)==c.unit && pos[0]==o.x && pos[1]==o.y && std::abs(a[0]-e.frame*.12f)<.000001f && std::abs(a[1]-e.delay)<.000001f,"large hunt native import position and animation");
  }
 }
 std::puts("PASS: original near/distant large hunt, clearance, variance and native import");
}
