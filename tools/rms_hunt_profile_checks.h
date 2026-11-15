// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cmath>
// Controlled original Forest prefix: forage3, both Nerf rows, then independently verified hunt rows.
static void rmsHuntProfileChecks(swgb::Assets &assets,const std::string &dataDir) {
 swgb::ResourceSet resources;swgb::RmsScript script;std::string error;
 check(resources.add(dataDir+"/gamedata_x1.drs",&error) && script.load(54204,resources,&error),error.c_str());
 const std::unordered_set<std::string> defines{"TINY_MAP","FIXED_POSITIONS","DEFAULT_RESOURCES"};
 struct Animation{uint8_t frame;float delay,heading;};
 const struct{int unit,count;bool grouped;uint32_t rng,poses;std::vector<Animation> animations;bool distant=false,omitNerfs=false;} cases[]={
{822,2,true,1777230500u,1964833593u,{{9,0.373876095f,4.71238899f},{2,0.f,3.14159274f},{9,0.0606158301f,1.57079637f},{9,0.477223992f,0.f}}},
{822,4,true,1427887360u,3213888073u,{{9,0.373876095f,4.71238899f},{2,0.f,3.14159274f},{2,0.f,3.92699075f},{2,0.f,1.57079637f},{1,0.f,4.71238899f},{5,0.f,2.3561945f},{4,0.f,4.71238899f},{9,0.f,1.57079637f}}},
{1249,1,false,3550983994u,288820904u,{{6,0.f,0.785398185f},{2,0.f,0.f}}},
{1249,2,false,1585104208u,2409764850u,{{6,0.f,0.785398185f},{9,0.373876095f,4.71238899f},{9,0.f,0.785398185f},{9,0.362825364f,0.785398185f}}},
{822,3,true,3597322543u,4039699298u,{{5,0.f,1.57079637f},{4,0.f,0.785398185f},{9,0.412164629f,0.f},{3,0.f,2.3561945f},{9,0.f,1.57079637f}},true},
{1249,3,true,3597322543u,433503753u,{{5,0.f,1.57079637f},{4,0.f,0.785398185f},{9,0.412164629f,0.f},{3,0.f,2.3561945f},{9,0.f,1.57079637f}},true},
{822,1,false,3550983994u,481104102u,{{6,0.f,0.785398185f},{2,0.f,0.f}}},
{1249,2,true,1777230500u,1048510013u,{{9,0.373876095f,4.71238899f},{2,0.f,3.14159274f},{9,0.0606158301f,1.57079637f},{9,0.477223992f,0.f}}},
{1363,4,true,2967105290u,66631807u,{{3,0.f,2.3561945f},{3,0.f,3.92699075f},{2,0.f,3.14159274f},{4,0.f,0.f},{10,0.468173444f,3.14159274f},{1,0.f,4.71238899f},{5,0.f,4.71238899f}}},
{1363,8,true,3099826770u,4071141167u,{{3,0.f,2.3561945f},{3,0.f,3.92699075f},{2,0.f,3.14159274f},{4,0.f,0.f},{2,0.f,1.57079637f},{0,0.f,5.497787f},{10,0.372138381f,0.785398185f},{10,0.318538725f,4.71238899f},{5,0.f,3.92699075f},{7,0.f,3.92699075f},{4,0.f,3.92699075f},{4,0.f,4.71238899f},{4,0.f,5.497787f},{4,0.f,0.785398185f},{7,0.f,3.14159274f},{3,0.f,2.3561945f}}},
{1365,4,true,2967105290u,78062853u,{{3,0.f,2.3561945f},{3,0.f,3.92699075f},{2,0.f,3.14159274f},{4,0.f,0.f},{11,0.466075003f,3.14159274f},{1,0.f,4.71238899f},{5,0.f,4.71238899f}}},
{1365,8,true,3099826770u,716593567u,{{3,0.f,2.3561945f},{3,0.f,3.92699075f},{2,0.f,3.14159274f},{4,0.f,0.f},{2,0.f,1.57079637f},{0,0.f,5.497787f},{11,0.36370796f,0.785398185f},{11,0.306574255f,4.71238899f},{6,0.f,3.92699075f},{7,0.f,3.92699075f},{4,0.f,3.92699075f},{4,0.f,4.71238899f},{4,0.f,5.497787f},{5,0.f,0.785398185f},{8,0.f,3.14159274f},{3,0.f,2.3561945f}}},
{1363,4,true,2951609664u,1284335974u,{{10,0.f,0.f},{10,0.181623548f,3.14159274f},{2,0.f,0.785398185f},{8,0.f,1.57079637f},{6,0.f,0.785398185f},{6,0.f,1.57079637f},{10,0.281657726f,0.785398185f},{2,0.f,4.71238899f}},false,true},
{1363,8,true,4160012160u,1264886363u,{{10,0.f,0.f},{10,0.181623548f,3.14159274f},{2,0.f,0.785398185f},{8,0.f,1.57079637f},{2,0.f,5.497787f},{9,0.f,5.497787f},{9,0.f,0.f},{2,0.f,3.14159274f},{0,0.f,3.14159274f},{10,0.199786335f,4.71238899f},{4,0.f,3.92699075f},{10,0.f,0.785398185f},{9,0.f,3.14159274f},{10,0.287934154f,4.71238899f},{5,0.f,0.f},{7,0.f,2.3561945f}},false,true},
{1365,4,true,2951609664u,1532897798u,{{11,0.f,0.f},{11,0.160631716f,3.14159274f},{2,0.f,0.785398185f},{9,0.f,1.57079637f},{7,0.f,0.785398185f},{6,0.f,1.57079637f},{11,0.267261535f,0.785398185f},{2,0.f,4.71238899f}},false,true},
{1365,8,true,4160012160u,2964882311u,{{11,0.f,0.f},{11,0.160631716f,3.14159274f},{2,0.f,0.785398185f},{9,0.f,1.57079637f},{2,0.f,5.497787f},{10,0.f,5.497787f},{10,0.f,0.f},{3,0.f,3.14159274f},{0,0.f,3.14159274f},{11,0.17999205f,4.71238899f},{4,0.f,3.92699075f},{11,0.f,0.785398185f},{10,0.f,3.14159274f},{11,0.273951799f,4.71238899f},{5,0.f,0.f},{8,0.f,2.3561945f}},false,true},
{1365,2,true,891822066u,2619764314u,{{3,0.f,2.3561945f},{3,0.f,3.92699075f},{11,0.f,5.497787f},{10,0.f,3.14159274f}},false,false},
{1365,2,true,3061237336u,4122497869u,{{11,0.f,0.f},{11,0.160631716f,3.14159274f},{2,0.f,0.f}},false,true},
{1249,1,true,1885517358u,3833361832u,{{9,0.373876095f,4.71238899f},{8,0.f,3.92699075f}},false},
{1249,1,true,1126655240u,639628253u,{{5,0.f,1.57079637f},{9,0.481063187f,4.71238899f}},true},
{1363,3,true,2341451770u,1550327383u,{{3,0.f,2.3561945f},{3,0.f,3.92699075f},{2,0.f,3.14159274f},{9,0.f,3.14159274f},{10,0.468173444f,3.14159274f}},false},
{822,1,true,1885517358u,880263998u,{{9,0.373876095f,4.71238899f},{8,0.f,3.92699075f}},false},
{822,6,true,4044577126u,660531056u,{{5,0.f,1.57079637f},{4,0.f,0.785398185f},{9,0.412164629f,0.f},{1,0.f,2.3561945f},{9,0.f,0.f},{1,0.f,0.785398185f},{9,0.472191453f,5.497787f},{7,0.f,5.497787f},{1,0.f,3.92699075f},{4,0.f,0.785398185f},{2,0.f,4.71238899f},{4,0.f,3.14159274f}},true},
{1249,6,true,4044577126u,2226817864u,{{5,0.f,1.57079637f},{4,0.f,0.785398185f},{9,0.412164629f,0.f},{1,0.f,2.3561945f},{9,0.f,0.f},{1,0.f,0.785398185f},{9,0.472191453f,5.497787f},{7,0.f,5.497787f},{1,0.f,3.92699075f},{4,0.f,0.785398185f},{2,0.f,4.71238899f},{4,0.f,3.14159274f}},true}};
 for(const auto &c:cases) {
  uint32_t rng=16160;std::vector<swgb::RmsToken> tokens;
  check(script.preprocess(defines,tokens,&error,&rng) && rng==1993207714u,"Forest hunt preprocessing RNG");
  std::string source;unsigned rows=0,forageChanges=0;
  for(size_t i=0;i<tokens.size();) {
   if(tokens[i].text!="create_object"){source+=tokens[i++].text+" ";continue;}
   check(i+2<tokens.size() && tokens[i+2].text=="{","original object block");
   size_t end=i+3;while(end<tokens.size() && tokens[end].text!="}")++end;
   check(end<tokens.size(),"terminated original object block");++rows;
   if(rows<=12 && !(c.omitNerfs && rows>=11)) {
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
    if(c.unit==1363 || c.unit==1365)source+="group_variance 1 min_distance_to_players 14 max_distance_to_players 30 } ";
    else source+=c.distant?"group_variance 1 min_distance_to_players 35 min_distance_group_placement 5 } ":"min_distance_to_players 16 max_distance_to_players 22 } ";
   }
   i=end+1;
  }
  check(rows==23 && forageChanges==1,"controlled13-row prefix retains later elevation/connections");
  swgb::RmsScript prefix;swgb::RmsMap map;
  check(prefix.load(0,[&](int32_t id,std::vector<uint8_t>&bytes){if(id)return false;bytes.assign(source.begin(),source.end());return true;},{},&error),error.c_str());
  check(swgb::generateRmsMap(prefix,defines,120,120,assets.dat(),rng,map,&error,2,{1,1}),error.c_str());
  const size_t preceding=c.omitNerfs?59:75;
  check(rng==c.rng && map.scenery.size()==7554 && map.ownedObjects.size()==10 && map.resourcesAfterOwned.size()==preceding+c.animations.size(),"Forest hunt original counts/RNG");
  auto hashBytes=[](const auto&bytes){uint32_t hash=2166136261u;for(auto v:bytes)hash=(hash^v)*16777619u;return hash;};
  check(hashBytes(map.terrain)==3348257725u && hashBytes(map.elevation)==908132044u,"Forest hunt original terrain/elevation");
  uint32_t hash=2166136261u;
  for(const auto&o:map.resourcesAfterOwned)for(auto v:{uint32_t(o.unitId),0u,uint32_t(o.x*2),uint32_t(o.y*2),uint32_t(o.facet)})hash=(hash^v)*16777619u;
  check(hash==c.poses,"Forest hunt original positions/facets/order including border and neighbour rejection");
  swgb::Game game(assets);rng=1993207714u;
  check(game.initRms(prefix,defines,120,rng,&error,2,{1,1}) && rng==c.rng,error.c_str());
  for(size_t i=0;i<c.animations.size();++i) {
   const auto &o=map.resourcesAfterOwned[preceding+i];const auto &e=c.animations[i];
   check(o.unitId==c.unit && o.animationFrame==e.frame && std::abs(o.initialIdleDelay-e.delay)<.000001f && o.headingRadians==e.heading,"Forest hunt original hunting-animal constructor");
   const uint32_t id=uint32_t(map.scenery.size()+map.ownedObjects.size()+preceding+1+i);const auto a=game.objectAnimationForTesting(id);const auto pos=game.objectPosition(id);
   check(game.objectUnitId(id)==c.unit && pos[0]==o.x && pos[1]==o.y && std::abs(a[0]-e.frame*.12f)<.000001f && std::abs(a[1]-e.delay)<.000001f,"Forest hunt native import position and animation");
  }
 }
 std::puts("PASS: original Forest hunt species, quantities, clearance, variance and native import");
}
