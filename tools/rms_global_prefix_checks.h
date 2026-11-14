// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "rms_stock_source.h"
// Original full startup on authored flat forest maps, through global deposits.
static void rmsGlobalPrefixChecks(swgb::Assets &assets) {
 struct Point {int unit;float x,y;uint8_t facet;};
 const struct {uint32_t seed,final;std::vector<Point> points;} cases[]={
{1u,858025488u,{{66,32.5f,11.5f,3},{66,32.5f,10.5f,7},{66,31.5f,10.5f,3},{66,108.5f,58.5f,0},{66,108.5f,59.5f,0},{66,108.5f,60.5f,9},{102,105.5f,84.5f,6},{102,105.5f,83.5f,5},{102,106.5f,83.5f,6}}},
{12345u,2525172882u,{{66,98.5f,86.5f,9},{66,98.5f,87.5f,1},{66,98.5f,85.5f,8},{66,15.5f,19.5f,2},{66,15.5f,18.5f,4},{66,14.5f,18.5f,9},{102,59.5f,109.5f,3},{102,58.5f,109.5f,2},{102,59.5f,108.5f,2},{102,60.5f,108.5f,3}}},
};

 const auto source=rmsStockResourceSource(true,8)+R"(
 create_object 126 { number_of_objects 2 set_place_for_every_player set_gaia_object_only min_distance_to_players 30 max_distance_to_players 40 }
 create_object 66 { number_of_groups 2 number_of_objects 3 group_placement_radius 2 set_tight_grouping min_distance_to_players 40 min_distance_group_placement 9 }
 create_object 102 { number_of_groups 1 number_of_objects 4 group_variance 1 group_placement_radius 2 set_tight_grouping min_distance_to_players 30 min_distance_group_placement 9 })";
 swgb::RmsScript script;std::string error;
 check(script.load(0,[&](int32_t id,std::vector<uint8_t>&bytes){if(id)return false;bytes.assign(source.begin(),source.end());return true;},{},&error),error.c_str());
 for(const auto &c:cases) {
  uint32_t rng=c.seed;swgb::RmsMap map;
  check(swgb::generateRmsMap(script,{},120,120,assets.dat(),rng,map,&error,2,{1,1}),error.c_str());
  check(rng==c.final && map.resourcesAfterOwned.size()==85+c.points.size(),"original global prefix count/RNG");
  swgb::Game game(assets);rng=c.seed;
  check(game.initRms(script,{},120,rng,&error,2,{1,1}),error.c_str());
  check(rng==c.final && game.activeObjectCount()==map.scenery.size()+95+c.points.size(),"global resource import count/RNG");
  for(size_t i=0;i<c.points.size();++i) {
   const auto &e=c.points[i];const auto &o=map.resourcesAfterOwned[85+i];
   check(o.unitId==e.unit && o.x==e.x && o.y==e.y && o.facet==e.facet,"original global prefix pose");
   const uint32_t id=uint32_t(map.scenery.size()+96+i);const auto pos=game.objectPosition(id);
   check(game.objectUnitId(id)==e.unit && game.objectPlayer(id)==0 && pos[0]==e.x && pos[1]==e.y && game.objectResourceAmount(id)==(e.unit==66?800:350),"global resource native ID/position/amount");
  }
 }
 std::puts("PASS: original global deposits through full parser and native import");
}
