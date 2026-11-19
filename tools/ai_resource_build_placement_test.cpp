// SPDX-License-Identifier: GPL-3.0-or-later
// Original588b30 scan and58e2d0 grid controls, 2026-11-18.
#include "../src/engine/ai_build_placement.h"
#include "../src/engine/ai_prefab_grid.h"
#include "../src/engine/rms_walls.h"
#include "../src/core/genie_start_placement.h"
#include <cassert>
#include <cstdio>
#include <vector>
#include <utility>
using Point=std::pair<int,int>;
struct ScanResult {swgb::AiBuildPlacementResult point;int draws=0;std::vector<Point> visited;};
ScanResult scan(int category,int test) {
 ScanResult out;
 int footprint=test==2?2:test==3?3:1,lo=test==6?0:2,hi=test==6?2:4,origin=test==6?0:3;
 out.point=swgb::aiScanBuildingPlacement(category,64,64,lo,lo,hi,hi,origin,origin,footprint,
  [&](int x,int y){return test==5 && x==3 && y==3?255:test==8 && x==4 && y==4?9:1;},
  [&]{++out.draws;return test==1?1u:0u;},
  [&](int x,int y){return test==7 && x==3 && y==3;},
  [&](float x,float y){out.visited.push_back({int(x),int(y)});return !(test==4 && int(x)==3 && int(y)==3);},
  [](int,int,int){return false;});return out;
}
ScanResult penalty(int category,int test) {
 ScanResult out;int lo=test==6?0:test==0?3:test==4?21:20,hi=test==6?2:lo,origin=test==6?0:3;
 out.point=swgb::aiScanBuildingPlacement(category,64,64,lo,lo,hi,hi,origin,origin,2,
  [&](int x,int y){return test==0 && x==1 && y==1?255:(test==1 || test==3 || test==4)?60:1;},
  [&]{++out.draws;return 0u;},[&](int x,int y){return test==5 && x==20 && y==20;},
  [&](float x,float y){out.visited.push_back({int(x),int(y)});return true;},
  [&](int x,int y,int radius){return (test==3 || test==4) && std::abs(x-10)<=radius && std::abs(y-10)<=radius;});return out;
}
// Final Windows scoring grid from P2's first seed-2 carbon processor search.
void carbonSiteScores() {
 struct Site { int x,y,score; uint8_t cells[16]; };
 const Site sites[]={
  {37,33,3813,{254,254,254,220,254,254,254,209,254,254,240,197,254,254,224,185}},
  {41,27,1948,{158,127,100,93,165,129,101,94,165,129,101,94,169,129,101,94}},
  {27,23,1700,{138,138,17,0,156,156,40,0,175,187,87,0,228,254,126,0}}
 };
 for(const auto &site:sites) {
  const auto result=swgb::aiScanBuildingPlacement(11,120,120,site.x,site.y,site.x,site.y,47,13,2,
   [&](int x,int y){return site.cells[(y-site.y+2)*4+x-site.x+2];},
   []{return 0u;},[](int,int){return false;},[](float,float){return true;},
   [](int,int,int){return false;});
  assert(result.x==site.x && result.y==site.y && result.score==site.score);
 }
}
uint32_t fingerprint(const swgb::AiBuildPlacementGrid &grid) {
 uint32_t hash=2166136261u;for(auto value:grid.cells())hash=(hash^value)*16777619u;return hash;
}
// Fresh original58b100/58c8e0 controls: food-nova and class-audit, 2026-11-18.
void foodNovaChecks() {
 struct ResourceCase {int category,cls,bucket;float x,y;bool disabled,foodDepot;uint32_t hash;};
 const ResourceCase resources[]={
  {1,27,0,16.0f,16.0f,false,false,3322049246u},
  {1,28,0,16.0f,16.0f,false,false,3570042246u},
  {1,29,2,16.0f,16.0f,false,false,3490044638u},
  {1,30,3,16.0f,16.0f,false,false,1980717710u},
  {1,31,1,16.0f,16.0f,false,false,1980717710u},
  {1,4,0,16.0f,16.0f,false,false,3570042246u},
  {2,27,0,16.0f,16.0f,false,false,1844462806u},
  {2,28,0,16.0f,16.0f,false,false,3570042246u},
  {2,29,2,16.0f,16.0f,false,false,3981575254u},
  {2,30,3,16.0f,16.0f,false,false,3981575254u},
  {2,31,1,16.0f,16.0f,false,false,2985515090u},
  {2,4,0,16.0f,16.0f,false,false,3570042246u},
  {1,27,0,0.25f,31.75f,false,false,1778116230u},
  {1,27,0,16.0f,16.0f,true,false,900439493u},
  {1,27,0,16.0f,16.0f,false,true,3322049246u},
  {1,28,0,0.25f,31.75f,false,false,1472089094u},
  {1,28,0,16.0f,16.0f,true,false,900439493u},
  {1,28,0,16.0f,16.0f,false,true,3570042246u},
  {1,29,2,0.25f,31.75f,false,false,227054318u},
  {1,29,2,16.0f,16.0f,true,false,900439493u},
  {1,29,2,16.0f,16.0f,false,true,3490044638u},
  {1,30,3,0.25f,31.75f,false,false,2147406854u},
  {1,30,3,16.0f,16.0f,true,false,900439493u},
  {1,30,3,16.0f,16.0f,false,true,1980717710u},
  {1,31,1,0.25f,31.75f,false,false,2147406854u},
  {1,31,1,16.0f,16.0f,true,false,900439493u},
  {1,31,1,16.0f,16.0f,false,true,1980717710u},
  {1,4,0,0.25f,31.75f,false,false,1472089094u},
  {1,4,0,16.0f,16.0f,true,false,900439493u},
  {1,4,0,16.0f,16.0f,false,true,3570042246u},
  {2,27,0,0.25f,31.75f,false,false,2802443876u},
  {2,27,0,16.0f,16.0f,true,false,900439493u},
  {2,27,0,16.0f,16.0f,false,true,1844462806u},
  {2,28,0,0.25f,31.75f,false,false,1472089094u},
  {2,28,0,16.0f,16.0f,true,false,900439493u},
  {2,28,0,16.0f,16.0f,false,true,3570042246u},
  {2,29,2,0.25f,31.75f,false,false,563219462u},
  {2,29,2,16.0f,16.0f,true,false,900439493u},
  {2,29,2,16.0f,16.0f,false,true,3981575254u},
  {2,30,3,0.25f,31.75f,false,false,563219462u},
  {2,30,3,16.0f,16.0f,true,false,900439493u},
  {2,30,3,16.0f,16.0f,false,true,3981575254u},
  {2,31,1,0.25f,31.75f,false,false,2737008576u},
  {2,31,1,16.0f,16.0f,true,false,900439493u},
  {2,31,1,16.0f,16.0f,false,true,2985515090u},
  {2,4,0,0.25f,31.75f,false,false,1472089094u},
  {2,4,0,16.0f,16.0f,true,false,900439493u},
  {2,4,0,16.0f,16.0f,false,true,3046527686u},
  {1,0,0,16.0f,16.0f,false,false,3570042246u},
  {1,1,0,16.0f,16.0f,false,false,3570042246u},
  {1,2,0,16.0f,16.0f,false,false,3570042246u},
  {1,3,0,16.0f,16.0f,false,false,3570042246u},
  {1,5,0,16.0f,16.0f,false,false,3570042246u},
  {1,6,0,16.0f,16.0f,false,false,3570042246u},
  {1,7,0,16.0f,16.0f,false,false,3570042246u},
  {1,8,0,16.0f,16.0f,false,false,3570042246u},
  {1,9,0,16.0f,16.0f,false,false,3570042246u},
  {1,10,0,16.0f,16.0f,false,false,3570042246u},
  {1,11,0,16.0f,16.0f,false,false,3570042246u},
  {1,12,0,16.0f,16.0f,false,false,3570042246u},
  {1,13,0,16.0f,16.0f,false,false,3570042246u},
  {1,14,0,16.0f,16.0f,false,false,3570042246u},
  {1,15,0,16.0f,16.0f,false,false,3570042246u},
  {1,16,0,16.0f,16.0f,false,false,3570042246u},
  {1,17,0,16.0f,16.0f,false,false,3570042246u},
  {1,18,0,16.0f,16.0f,false,false,3570042246u},
  {1,19,0,16.0f,16.0f,false,false,3570042246u},
  {1,20,0,16.0f,16.0f,false,false,3570042246u},
  {1,21,0,16.0f,16.0f,false,false,3570042246u},
  {1,22,0,16.0f,16.0f,false,false,3570042246u},
  {1,23,0,16.0f,16.0f,false,false,3570042246u},
  {1,24,0,16.0f,16.0f,false,false,3570042246u},
  {1,25,0,16.0f,16.0f,false,false,3570042246u},
  {1,26,0,16.0f,16.0f,false,false,3570042246u},
  {1,29,0,16.0f,16.0f,false,false,3490044638u},
  {1,30,0,16.0f,16.0f,false,false,1980717710u},
  {1,31,0,16.0f,16.0f,false,false,1980717710u},
  {1,32,0,16.0f,16.0f,false,false,3570042246u},
  {1,33,0,16.0f,16.0f,false,false,3570042246u},
  {1,34,0,16.0f,16.0f,false,false,3570042246u},
  {1,35,0,16.0f,16.0f,false,false,3570042246u},
  {1,36,0,16.0f,16.0f,false,false,3570042246u},
  {1,37,0,16.0f,16.0f,false,false,3570042246u},
  {1,38,0,16.0f,16.0f,false,false,3570042246u},
  {1,39,0,16.0f,16.0f,false,false,3570042246u},
  {1,40,0,16.0f,16.0f,false,false,3570042246u},
  {1,41,0,16.0f,16.0f,false,false,3570042246u},
  {1,42,0,16.0f,16.0f,false,false,3570042246u},
  {1,43,0,16.0f,16.0f,false,false,3570042246u},
  {1,44,0,16.0f,16.0f,false,false,3570042246u},
  {1,45,0,16.0f,16.0f,false,false,3570042246u},
  {1,46,0,16.0f,16.0f,false,false,3570042246u},
  {1,47,0,16.0f,16.0f,false,false,3570042246u},
  {1,48,0,16.0f,16.0f,false,false,3570042246u},
  {1,49,0,16.0f,16.0f,false,false,3570042246u},
  {1,50,0,16.0f,16.0f,false,false,3570042246u},
  {1,51,0,16.0f,16.0f,false,false,3570042246u},
  {1,52,0,16.0f,16.0f,false,false,3570042246u},
  {1,53,0,16.0f,16.0f,false,false,3570042246u},
  {1,54,0,16.0f,16.0f,false,false,3570042246u},
  {1,55,0,16.0f,16.0f,false,false,3570042246u},
  {1,56,0,16.0f,16.0f,false,false,3570042246u},
  {1,57,0,16.0f,16.0f,false,false,3570042246u},
  {1,58,0,16.0f,16.0f,false,false,3570042246u},
  {1,59,0,16.0f,16.0f,false,false,3570042246u},
  {1,60,0,16.0f,16.0f,false,false,3570042246u},
  {1,61,0,16.0f,16.0f,false,false,3570042246u},
  {1,62,0,16.0f,16.0f,false,false,3570042246u},
  {1,63,0,16.0f,16.0f,false,false,3570042246u},
  {1,64,0,16.0f,16.0f,false,false,3570042246u},
  {2,0,0,16.0f,16.0f,false,false,3570042246u},
  {2,0,0,16.0f,16.0f,false,true,3570042246u},
  {2,1,0,16.0f,16.0f,false,false,3570042246u},
  {2,1,0,16.0f,16.0f,false,true,3570042246u},
  {2,2,0,16.0f,16.0f,false,false,3570042246u},
  {2,2,0,16.0f,16.0f,false,true,3570042246u},
  {2,3,0,16.0f,16.0f,false,false,3570042246u},
  {2,3,0,16.0f,16.0f,false,true,3570042246u},
  {2,5,0,16.0f,16.0f,false,false,3570042246u},
  {2,5,0,16.0f,16.0f,false,true,3046527686u},
  {2,6,0,16.0f,16.0f,false,false,3570042246u},
  {2,6,0,16.0f,16.0f,false,true,3570042246u},
  {2,7,0,16.0f,16.0f,false,false,3570042246u},
  {2,7,0,16.0f,16.0f,false,true,3570042246u},
  {2,8,0,16.0f,16.0f,false,false,3570042246u},
  {2,8,0,16.0f,16.0f,false,true,3570042246u},
  {2,9,0,16.0f,16.0f,false,false,3570042246u},
  {2,9,0,16.0f,16.0f,false,true,3570042246u},
  {2,10,0,16.0f,16.0f,false,false,3570042246u},
  {2,10,0,16.0f,16.0f,false,true,3570042246u},
  {2,11,0,16.0f,16.0f,false,false,3570042246u},
  {2,11,0,16.0f,16.0f,false,true,3570042246u},
  {2,12,0,16.0f,16.0f,false,false,3570042246u},
  {2,12,0,16.0f,16.0f,false,true,3570042246u},
  {2,13,0,16.0f,16.0f,false,false,3570042246u},
  {2,13,0,16.0f,16.0f,false,true,3570042246u},
  {2,14,0,16.0f,16.0f,false,false,3570042246u},
  {2,14,0,16.0f,16.0f,false,true,3570042246u},
  {2,15,0,16.0f,16.0f,false,false,3570042246u},
  {2,15,0,16.0f,16.0f,false,true,3570042246u},
  {2,16,0,16.0f,16.0f,false,false,3570042246u},
  {2,16,0,16.0f,16.0f,false,true,3570042246u},
  {2,17,0,16.0f,16.0f,false,false,3570042246u},
  {2,17,0,16.0f,16.0f,false,true,3570042246u},
  {2,18,0,16.0f,16.0f,false,false,3570042246u},
  {2,18,0,16.0f,16.0f,false,true,3570042246u},
  {2,19,0,16.0f,16.0f,false,false,3570042246u},
  {2,19,0,16.0f,16.0f,false,true,3570042246u},
  {2,20,0,16.0f,16.0f,false,false,3570042246u},
  {2,20,0,16.0f,16.0f,false,true,3570042246u},
  {2,21,0,16.0f,16.0f,false,false,3570042246u},
  {2,21,0,16.0f,16.0f,false,true,3570042246u},
  {2,22,0,16.0f,16.0f,false,false,3570042246u},
  {2,22,0,16.0f,16.0f,false,true,3570042246u},
  {2,23,0,16.0f,16.0f,false,false,3570042246u},
  {2,23,0,16.0f,16.0f,false,true,3570042246u},
  {2,24,0,16.0f,16.0f,false,false,3570042246u},
  {2,24,0,16.0f,16.0f,false,true,3570042246u},
  {2,25,0,16.0f,16.0f,false,false,2087151446u},
  {2,25,0,16.0f,16.0f,false,true,2087151446u},
  {2,26,0,16.0f,16.0f,false,false,3570042246u},
  {2,26,0,16.0f,16.0f,false,true,3570042246u},
  {2,29,0,16.0f,16.0f,false,false,3981575254u},
  {2,29,0,16.0f,16.0f,false,true,3981575254u},
  {2,30,0,16.0f,16.0f,false,false,3981575254u},
  {2,30,0,16.0f,16.0f,false,true,3981575254u},
  {2,31,0,16.0f,16.0f,false,false,2985515090u},
  {2,31,0,16.0f,16.0f,false,true,2985515090u},
  {2,32,0,16.0f,16.0f,false,false,3570042246u},
  {2,32,0,16.0f,16.0f,false,true,3570042246u},
  {2,33,0,16.0f,16.0f,false,false,3570042246u},
  {2,33,0,16.0f,16.0f,false,true,3570042246u},
  {2,34,0,16.0f,16.0f,false,false,3570042246u},
  {2,34,0,16.0f,16.0f,false,true,3570042246u},
  {2,35,0,16.0f,16.0f,false,false,3570042246u},
  {2,35,0,16.0f,16.0f,false,true,3570042246u},
  {2,36,0,16.0f,16.0f,false,false,3570042246u},
  {2,36,0,16.0f,16.0f,false,true,3570042246u},
  {2,37,0,16.0f,16.0f,false,false,3570042246u},
  {2,37,0,16.0f,16.0f,false,true,3570042246u},
  {2,38,0,16.0f,16.0f,false,false,3570042246u},
  {2,38,0,16.0f,16.0f,false,true,3570042246u},
  {2,39,0,16.0f,16.0f,false,false,3570042246u},
  {2,39,0,16.0f,16.0f,false,true,3570042246u},
  {2,40,0,16.0f,16.0f,false,false,3570042246u},
  {2,40,0,16.0f,16.0f,false,true,3570042246u},
  {2,41,0,16.0f,16.0f,false,false,3570042246u},
  {2,41,0,16.0f,16.0f,false,true,3570042246u},
  {2,42,0,16.0f,16.0f,false,false,3570042246u},
  {2,42,0,16.0f,16.0f,false,true,3570042246u},
  {2,43,0,16.0f,16.0f,false,false,3570042246u},
  {2,43,0,16.0f,16.0f,false,true,3570042246u},
  {2,44,0,16.0f,16.0f,false,false,3570042246u},
  {2,44,0,16.0f,16.0f,false,true,3570042246u},
  {2,45,0,16.0f,16.0f,false,false,3570042246u},
  {2,45,0,16.0f,16.0f,false,true,3570042246u},
  {2,46,0,16.0f,16.0f,false,false,3570042246u},
  {2,46,0,16.0f,16.0f,false,true,3570042246u},
  {2,47,0,16.0f,16.0f,false,false,3570042246u},
  {2,47,0,16.0f,16.0f,false,true,3570042246u},
  {2,48,0,16.0f,16.0f,false,false,3570042246u},
  {2,48,0,16.0f,16.0f,false,true,3570042246u},
  {2,49,0,16.0f,16.0f,false,false,3570042246u},
  {2,49,0,16.0f,16.0f,false,true,3570042246u},
  {2,50,0,16.0f,16.0f,false,false,3570042246u},
  {2,50,0,16.0f,16.0f,false,true,3570042246u},
  {2,51,0,16.0f,16.0f,false,false,3570042246u},
  {2,51,0,16.0f,16.0f,false,true,3570042246u},
  {2,52,0,16.0f,16.0f,false,false,3570042246u},
  {2,52,0,16.0f,16.0f,false,true,3570042246u},
  {2,53,0,16.0f,16.0f,false,false,3570042246u},
  {2,53,0,16.0f,16.0f,false,true,3570042246u},
  {2,54,0,16.0f,16.0f,false,false,3570042246u},
  {2,54,0,16.0f,16.0f,false,true,3570042246u},
  {2,55,0,16.0f,16.0f,false,false,3570042246u},
  {2,55,0,16.0f,16.0f,false,true,3570042246u},
  {2,56,0,16.0f,16.0f,false,false,3570042246u},
  {2,56,0,16.0f,16.0f,false,true,3570042246u},
  {2,57,0,16.0f,16.0f,false,false,3570042246u},
  {2,57,0,16.0f,16.0f,false,true,3570042246u},
  {2,58,0,16.0f,16.0f,false,false,3570042246u},
  {2,58,0,16.0f,16.0f,false,true,3570042246u},
  {2,59,0,16.0f,16.0f,false,false,3570042246u},
  {2,59,0,16.0f,16.0f,false,true,3570042246u},
  {2,60,0,16.0f,16.0f,false,false,3570042246u},
  {2,60,0,16.0f,16.0f,false,true,3046527686u},
  {2,61,0,16.0f,16.0f,false,false,3570042246u},
  {2,61,0,16.0f,16.0f,false,true,3570042246u},
  {2,62,0,16.0f,16.0f,false,false,3570042246u},
  {2,62,0,16.0f,16.0f,false,true,3570042246u},
  {2,63,0,16.0f,16.0f,false,false,3570042246u},
  {2,63,0,16.0f,16.0f,false,true,3570042246u},
  {2,64,0,16.0f,16.0f,false,false,3570042246u},
  {2,64,0,16.0f,16.0f,false,true,3570042246u},
 };
 for(const auto &r:resources) {swgb::AiBuildPlacementGrid grid(32,r.category);grid.resource(r.bucket,r.cls,r.x,r.y,.5f,.5f,r.disabled,r.foodDepot);assert(fingerprint(grid)==r.hash);}
 struct TerrainCase {int category,terrain;uint32_t mask,hash;};
 const TerrainCase terrains[]={
  {1,0,0u,900439493u},
  {2,0,0u,900439493u},
  {1,0,1u,3570042246u},
  {2,0,1u,3570042246u},
  {1,0,16384u,3570042246u},
  {2,0,16384u,3570042246u},
  {1,0,32768u,900439493u},
  {2,0,32768u,900439493u},
  {1,1,0u,545573246u},
  {2,1,0u,900439493u},
  {1,1,1u,545573246u},
  {2,1,1u,3570042246u},
  {1,1,16384u,545573246u},
  {2,1,16384u,3570042246u},
  {1,1,32768u,545573246u},
  {2,1,32768u,900439493u},
  {1,6,0u,900439493u},
  {2,6,0u,900439493u},
  {1,6,1u,3570042246u},
  {2,6,1u,3570042246u},
  {1,6,16384u,3570042246u},
  {2,6,16384u,3570042246u},
  {1,6,32768u,900439493u},
  {2,6,32768u,900439493u},
  {1,21,0u,900439493u},
  {2,21,0u,900439493u},
  {1,21,1u,3570042246u},
  {2,21,1u,3570042246u},
  {1,21,16384u,3570042246u},
  {2,21,16384u,3570042246u},
  {1,21,32768u,900439493u},
  {2,21,32768u,900439493u},
  {1,22,0u,545573246u},
  {2,22,0u,900439493u},
  {1,22,1u,545573246u},
  {2,22,1u,3570042246u},
  {1,22,16384u,545573246u},
  {2,22,16384u,3570042246u},
  {1,22,32768u,545573246u},
  {2,22,32768u,900439493u},
  {1,23,0u,545573246u},
  {2,23,0u,900439493u},
  {1,23,1u,545573246u},
  {2,23,1u,3570042246u},
  {1,23,16384u,545573246u},
  {2,23,16384u,3570042246u},
  {1,23,32768u,545573246u},
  {2,23,32768u,900439493u},
  {1,27,0u,900439493u},
  {2,27,0u,900439493u},
  {1,27,1u,3570042246u},
  {2,27,1u,3570042246u},
  {1,27,16384u,3570042246u},
  {2,27,16384u,3570042246u},
  {1,27,32768u,900439493u},
  {2,27,32768u,900439493u},
 };
 for(const auto &r:terrains) {swgb::AiBuildPlacementGrid grid(32,r.category);grid.terrain(16,16,r.terrain,r.mask);assert(fingerprint(grid)==r.hash);}
 struct OwnedCase {int category,prototype;uint32_t hash;};
 const OwnedCase owned[]={
  {1,109,3047606665u},
  {1,70,4290577461u},
  {1,68,4166922645u},
  {1,12,4290577461u},
  {1,87,4290577461u},
  {1,335,4290577461u},
  {1,562,4166922645u},
  {1,563,4290577461u},
  {1,584,4166922645u},
  {1,585,4290577461u},
  {2,109,3047606665u},
  {2,70,4290577461u},
  {2,68,4166922645u},
  {2,12,4290577461u},
  {2,87,4290577461u},
  {2,335,4290577461u},
  {2,562,4166922645u},
  {2,563,4290577461u},
  {2,584,4166922645u},
  {2,585,4290577461u},
 };
 for(const auto &r:owned) {swgb::AiBuildPlacementGrid grid(32,r.category);grid.ownedBuilding(r.prototype,16,16,2,6,10,10);assert(fingerprint(grid)==r.hash); }
 {const auto r=scan(1,0);assert(r.point.x==3 && r.point.y==3 && r.point.score==9 && r.draws==5);assert((r.visited==std::vector<Point>{{2,2},{3,3}}));}
 {const auto r=scan(1,1);assert(r.point.x==4 && r.point.y==4 && r.point.score==9 && r.draws==8);assert((r.visited==std::vector<Point>{{2,2},{2,3},{2,4},{3,2},{3,3},{3,4},{4,2},{4,3},{4,4}}));}
 {const auto r=scan(1,2);assert(r.point.x==3 && r.point.y==3 && r.point.score==16 && r.draws==5);assert((r.visited==std::vector<Point>{{2,2},{3,3}}));}
 {const auto r=scan(1,3);assert(r.point.x==3 && r.point.y==3 && r.point.score==25 && r.draws==5);assert((r.visited==std::vector<Point>{{2,2},{3,3}}));}
 {const auto r=scan(1,4);assert(r.point.x==2 && r.point.y==2 && r.point.score==9 && r.draws==7);assert((r.visited==std::vector<Point>{{2,2},{3,3}}));}
 {const auto r=scan(1,5);assert(r.point.x==-1 && r.point.y==-1 && r.point.score==-1 && r.draws==0);assert((r.visited==std::vector<Point>{}));}
 {const auto r=scan(1,6);assert(r.point.x==1 && r.point.y==1 && r.point.score==9 && r.draws==0);assert((r.visited==std::vector<Point>{{1,1}}));}
 {const auto r=scan(1,7);assert(r.point.x==2 && r.point.y==2 && r.point.score==9 && r.draws==7);assert((r.visited==std::vector<Point>{{2,2}}));}
 {const auto r=scan(2,0);assert(r.point.x==3 && r.point.y==3 && r.point.score==9 && r.draws==5);assert((r.visited==std::vector<Point>{{2,2},{3,3}}));}
 {const auto r=scan(2,1);assert(r.point.x==4 && r.point.y==4 && r.point.score==9 && r.draws==8);assert((r.visited==std::vector<Point>{{2,2},{2,3},{2,4},{3,2},{3,3},{3,4},{4,2},{4,3},{4,4}}));}
 {const auto r=scan(2,2);assert(r.point.x==3 && r.point.y==3 && r.point.score==16 && r.draws==5);assert((r.visited==std::vector<Point>{{2,2},{3,3}}));}
 {const auto r=scan(2,3);assert(r.point.x==3 && r.point.y==3 && r.point.score==25 && r.draws==5);assert((r.visited==std::vector<Point>{{2,2},{3,3}}));}
 {const auto r=scan(2,4);assert(r.point.x==2 && r.point.y==2 && r.point.score==9 && r.draws==7);assert((r.visited==std::vector<Point>{{2,2},{3,3}}));}
 {const auto r=scan(2,5);assert(r.point.x==-1 && r.point.y==-1 && r.point.score==-1 && r.draws==0);assert((r.visited==std::vector<Point>{}));}
 {const auto r=scan(2,6);assert(r.point.x==1 && r.point.y==1 && r.point.score==9 && r.draws==0);assert((r.visited==std::vector<Point>{{1,1}}));}
 {const auto r=scan(2,7);assert(r.point.x==2 && r.point.y==2 && r.point.score==9 && r.draws==7);assert((r.visited==std::vector<Point>{{2,2}}));}
}

int main() {
 carbonSiteScores();
 const swgb::GenieStartFootprint footprint{0,0,1,1};
 int heights[2][2]={{1,6},{2,3}};
 auto hillAllows=[&]{return swgb::genieHillMode3Allows(footprint,[&](int x,int y){return heights[y][x];});};
 assert(hillAllows());
 heights[0][0]=0;assert(!hillAllows());
 heights[0][0]=1;heights[1][1]=4;assert(!hillAllows());
 const auto tree=swgb::pathMapFootprint(35.5f,30.5f,.5f,.5f);
 assert(tree.left==35 && tree.right==36 && tree.top==30 && tree.bottom==31);
 const auto narrow=swgb::pathMapFootprint(5.5f,6.5f,.25f,1.f);
 assert(narrow.left==narrow.right && narrow.top==5 && narrow.bottom==7);

 foodNovaChecks();
 {const auto result=penalty(1,0);assert(result.point.x==-1 && result.point.y==-1 && result.point.score==-1 && result.draws==0);assert((result.visited==std::vector<Point>{}));}
 {const auto result=penalty(2,0);assert(result.point.x==-1 && result.point.y==-1 && result.point.score==-1 && result.draws==0);assert((result.visited==std::vector<Point>{}));}
 {const auto result=penalty(1,1);assert(result.point.x==20 && result.point.y==20 && result.point.score==958 && result.draws==0);assert((result.visited==std::vector<Point>{{20,20}}));}
 {const auto result=penalty(2,1);assert(result.point.x==20 && result.point.y==20 && result.point.score==958 && result.draws==0);assert((result.visited==std::vector<Point>{{20,20}}));}
 {const auto result=penalty(1,2);assert(result.point.x==20 && result.point.y==20 && result.point.score==16 && result.draws==0);assert((result.visited==std::vector<Point>{{20,20}}));}
 {const auto result=penalty(2,2);assert(result.point.x==20 && result.point.y==20 && result.point.score==16 && result.draws==0);assert((result.visited==std::vector<Point>{{20,20}}));}
 {const auto result=penalty(1,3);assert(result.point.x==20 && result.point.y==20 && result.point.score==96 && result.draws==0);assert((result.visited==std::vector<Point>{{20,20}}));}
 {const auto result=penalty(2,3);assert(result.point.x==20 && result.point.y==20 && result.point.score==96 && result.draws==0);assert((result.visited==std::vector<Point>{{20,20}}));}
 {const auto result=penalty(1,4);assert(result.point.x==21 && result.point.y==21 && result.point.score==957 && result.draws==0);assert((result.visited==std::vector<Point>{{21,21}}));}
 {const auto result=penalty(2,4);assert(result.point.x==21 && result.point.y==21 && result.point.score==957 && result.draws==0);assert((result.visited==std::vector<Point>{{21,21}}));}
 {const auto result=penalty(1,5);assert(result.point.x==-1 && result.point.y==-1 && result.point.score==-1 && result.draws==0);assert((result.visited==std::vector<Point>{}));}
 {const auto result=penalty(2,5);assert(result.point.x==-1 && result.point.y==-1 && result.point.score==-1 && result.draws==0);assert((result.visited==std::vector<Point>{}));}
 {const auto result=penalty(1,6);assert(result.point.x==2 && result.point.y==2 && result.point.score==16 && result.draws==0);assert((result.visited==std::vector<Point>{{2,2}}));}
 {const auto result=penalty(2,6);assert(result.point.x==2 && result.point.y==2 && result.point.score==16 && result.draws==0);assert((result.visited==std::vector<Point>{{2,2}}));}
 {const auto result=scan(8,0);assert(result.point.x==3 && result.point.y==3 && result.point.score==1 && result.draws==5);assert((result.visited==std::vector<Point>{{2,2},{3,3}}));}
 {const auto result=scan(8,1);assert(result.point.x==4 && result.point.y==4 && result.point.score==1 && result.draws==8);assert((result.visited==std::vector<Point>{{2,2},{2,3},{2,4},{3,2},{3,3},{3,4},{4,2},{4,3},{4,4}}));}
 {const auto result=scan(8,2);assert(result.point.x==3 && result.point.y==3 && result.point.score==4 && result.draws==5);assert((result.visited==std::vector<Point>{{2,2},{3,3}}));}
 {const auto result=scan(8,3);assert(result.point.x==3 && result.point.y==3 && result.point.score==9 && result.draws==5);assert((result.visited==std::vector<Point>{{2,2},{3,3}}));}
 {const auto result=scan(8,4);assert(result.point.x==2 && result.point.y==2 && result.point.score==1 && result.draws==7);assert((result.visited==std::vector<Point>{{2,2},{3,3}}));}
 {const auto result=scan(8,5);assert(result.point.x==2 && result.point.y==2 && result.point.score==1 && result.draws==7);assert((result.visited==std::vector<Point>{{2,2}}));}
 {const auto result=scan(8,6);assert(result.point.x==0 && result.point.y==0 && result.point.score==1 && result.draws==2);assert((result.visited==std::vector<Point>{{0,0}}));}
 {const auto result=scan(8,7);assert(result.point.x==2 && result.point.y==2 && result.point.score==1 && result.draws==7);assert((result.visited==std::vector<Point>{{2,2}}));}
 {const auto result=scan(8,8);assert(result.point.x==4 && result.point.y==4 && result.point.score==9 && result.draws==5);assert((result.visited==std::vector<Point>{{2,2},{3,3},{4,4}}));}
 {const auto result=scan(10,0);assert(result.point.x==3 && result.point.y==3 && result.point.score==1 && result.draws==5);assert((result.visited==std::vector<Point>{{2,2},{3,3}}));}
 {const auto result=scan(10,1);assert(result.point.x==4 && result.point.y==4 && result.point.score==1 && result.draws==8);assert((result.visited==std::vector<Point>{{2,2},{2,3},{2,4},{3,2},{3,3},{3,4},{4,2},{4,3},{4,4}}));}
 {const auto result=scan(10,2);assert(result.point.x==3 && result.point.y==3 && result.point.score==4 && result.draws==5);assert((result.visited==std::vector<Point>{{2,2},{3,3}}));}
 {const auto result=scan(10,3);assert(result.point.x==3 && result.point.y==3 && result.point.score==9 && result.draws==5);assert((result.visited==std::vector<Point>{{2,2},{3,3}}));}
 {const auto result=scan(10,4);assert(result.point.x==2 && result.point.y==2 && result.point.score==1 && result.draws==7);assert((result.visited==std::vector<Point>{{2,2},{3,3}}));}
 {const auto result=scan(10,5);assert(result.point.x==2 && result.point.y==2 && result.point.score==1 && result.draws==7);assert((result.visited==std::vector<Point>{{2,2}}));}
 {const auto result=scan(10,6);assert(result.point.x==0 && result.point.y==0 && result.point.score==1 && result.draws==2);assert((result.visited==std::vector<Point>{{0,0}}));}
 {const auto result=scan(10,7);assert(result.point.x==2 && result.point.y==2 && result.point.score==1 && result.draws==7);assert((result.visited==std::vector<Point>{{2,2}}));}
 {const auto result=scan(10,8);assert(result.point.x==4 && result.point.y==4 && result.point.score==9 && result.draws==5);assert((result.visited==std::vector<Point>{{2,2},{3,3},{4,4}}));}
 {const auto result=scan(11,0);assert(result.point.x==3 && result.point.y==3 && result.point.score==9 && result.draws==5);assert((result.visited==std::vector<Point>{{2,2},{3,3}}));}
 {const auto result=scan(11,1);assert(result.point.x==4 && result.point.y==4 && result.point.score==9 && result.draws==8);assert((result.visited==std::vector<Point>{{2,2},{2,3},{2,4},{3,2},{3,3},{3,4},{4,2},{4,3},{4,4}}));}
 {const auto result=scan(11,2);assert(result.point.x==3 && result.point.y==3 && result.point.score==16 && result.draws==5);assert((result.visited==std::vector<Point>{{2,2},{3,3}}));}
 {const auto result=scan(11,3);assert(result.point.x==3 && result.point.y==3 && result.point.score==25 && result.draws==5);assert((result.visited==std::vector<Point>{{2,2},{3,3}}));}
 {const auto result=scan(11,4);assert(result.point.x==2 && result.point.y==2 && result.point.score==9 && result.draws==7);assert((result.visited==std::vector<Point>{{2,2},{3,3}}));}
 {const auto result=scan(11,5);assert(result.point.x==-1 && result.point.y==-1 && result.point.score==-1 && result.draws==0);assert((result.visited==std::vector<Point>{}));}
 {const auto result=scan(11,6);assert(result.point.x==1 && result.point.y==1 && result.point.score==9 && result.draws==0);assert((result.visited==std::vector<Point>{{1,1}}));}
 {const auto result=scan(11,7);assert(result.point.x==2 && result.point.y==2 && result.point.score==9 && result.draws==7);assert((result.visited==std::vector<Point>{{2,2}}));}
 {const auto result=scan(11,8);assert(result.point.x==3 && result.point.y==3 && result.point.score==17 && result.draws==5);assert((result.visited==std::vector<Point>{{2,2},{3,3}}));}
 {const auto result=scan(13,0);assert(result.point.x==3 && result.point.y==3 && result.point.score==9 && result.draws==5);assert((result.visited==std::vector<Point>{{2,2},{3,3}}));}
 {const auto result=scan(13,1);assert(result.point.x==4 && result.point.y==4 && result.point.score==9 && result.draws==8);assert((result.visited==std::vector<Point>{{2,2},{2,3},{2,4},{3,2},{3,3},{3,4},{4,2},{4,3},{4,4}}));}
 {const auto result=scan(13,2);assert(result.point.x==3 && result.point.y==3 && result.point.score==16 && result.draws==5);assert((result.visited==std::vector<Point>{{2,2},{3,3}}));}
 {const auto result=scan(13,3);assert(result.point.x==3 && result.point.y==3 && result.point.score==25 && result.draws==5);assert((result.visited==std::vector<Point>{{2,2},{3,3}}));}
 {const auto result=scan(13,4);assert(result.point.x==2 && result.point.y==2 && result.point.score==9 && result.draws==7);assert((result.visited==std::vector<Point>{{2,2},{3,3}}));}
 {const auto result=scan(13,5);assert(result.point.x==-1 && result.point.y==-1 && result.point.score==-1 && result.draws==0);assert((result.visited==std::vector<Point>{}));}
 {const auto result=scan(13,6);assert(result.point.x==1 && result.point.y==1 && result.point.score==9 && result.draws==0);assert((result.visited==std::vector<Point>{{1,1}}));}
 {const auto result=scan(13,7);assert(result.point.x==2 && result.point.y==2 && result.point.score==9 && result.draws==7);assert((result.visited==std::vector<Point>{{2,2}}));}
 {const auto result=scan(13,8);assert(result.point.x==3 && result.point.y==3 && result.point.score==17 && result.draws==5);assert((result.visited==std::vector<Point>{{2,2},{3,3}}));}
 {const auto result=penalty(11,0);assert(result.point.x==-1 && result.point.y==-1 && result.point.score==-1 && result.draws==0);assert((result.visited==std::vector<Point>{}));}
 {const auto result=penalty(13,0);assert(result.point.x==-1 && result.point.y==-1 && result.point.score==-1 && result.draws==0);assert((result.visited==std::vector<Point>{}));}
 {const auto result=penalty(11,1);assert(result.point.x==20 && result.point.y==20 && result.point.score==958 && result.draws==0);assert((result.visited==std::vector<Point>{{20,20}}));}
 {const auto result=penalty(13,1);assert(result.point.x==20 && result.point.y==20 && result.point.score==958 && result.draws==0);assert((result.visited==std::vector<Point>{{20,20}}));}
 {const auto result=penalty(11,2);assert(result.point.x==20 && result.point.y==20 && result.point.score==16 && result.draws==0);assert((result.visited==std::vector<Point>{{20,20}}));}
 {const auto result=penalty(13,2);assert(result.point.x==20 && result.point.y==20 && result.point.score==16 && result.draws==0);assert((result.visited==std::vector<Point>{{20,20}}));}
 {const auto result=penalty(11,3);assert(result.point.x==20 && result.point.y==20 && result.point.score==96 && result.draws==0);assert((result.visited==std::vector<Point>{{20,20}}));}
 {const auto result=penalty(13,3);assert(result.point.x==20 && result.point.y==20 && result.point.score==96 && result.draws==0);assert((result.visited==std::vector<Point>{{20,20}}));}
 {const auto result=penalty(11,4);assert(result.point.x==21 && result.point.y==21 && result.point.score==957 && result.draws==0);assert((result.visited==std::vector<Point>{{21,21}}));}
 {const auto result=penalty(13,4);assert(result.point.x==21 && result.point.y==21 && result.point.score==957 && result.draws==0);assert((result.visited==std::vector<Point>{{21,21}}));}
 {const auto result=penalty(11,5);assert(result.point.x==-1 && result.point.y==-1 && result.point.score==-1 && result.draws==0);assert((result.visited==std::vector<Point>{}));}
 {const auto result=penalty(13,5);assert(result.point.x==-1 && result.point.y==-1 && result.point.score==-1 && result.draws==0);assert((result.visited==std::vector<Point>{}));}
 {const auto result=penalty(11,6);assert(result.point.x==2 && result.point.y==2 && result.point.score==16 && result.draws==0);assert((result.visited==std::vector<Point>{{2,2}}));}
 {const auto result=penalty(13,6);assert(result.point.x==2 && result.point.y==2 && result.point.score==16 && result.draws==0);assert((result.visited==std::vector<Point>{{2,2}}));}
 {swgb::AiBuildPlacementGrid grid(32,10);grid.ownedBuilding(109,16,16,2,6,10,10);assert(fingerprint(grid)==463919817u);}
 {swgb::AiBuildPlacementGrid grid(32,10);grid.ownedBuilding(70,16,16,2,6,10,10);assert(fingerprint(grid)==3295819637u);}
 {swgb::AiBuildPlacementGrid grid(32,10);grid.ownedBuilding(68,16,16,2,6,10,10);assert(fingerprint(grid)==3145056629u);}
 {swgb::AiBuildPlacementGrid grid(32,10);grid.ownedBuilding(12,16,16,2,6,10,10);assert(fingerprint(grid)==536633461u);}
 {swgb::AiBuildPlacementGrid grid(32,10);grid.ownedBuilding(87,16,16,2,6,10,10);assert(fingerprint(grid)==404997749u);}
 {swgb::AiBuildPlacementGrid grid(32,10);grid.ownedBuilding(335,16,16,2,6,10,10);assert(fingerprint(grid)==2503165557u);}
 {swgb::AiBuildPlacementGrid grid(32,10);grid.ownedBuilding(562,16,16,2,6,10,10);assert(fingerprint(grid)==3145056629u);}
 {swgb::AiBuildPlacementGrid grid(32,10);grid.ownedBuilding(563,16,16,2,6,10,10);assert(fingerprint(grid)==3145056629u);}
 {swgb::AiBuildPlacementGrid grid(32,10);grid.ownedBuilding(584,16,16,2,6,10,10);assert(fingerprint(grid)==3145056629u);}
 {swgb::AiBuildPlacementGrid grid(32,10);grid.ownedBuilding(585,16,16,2,6,10,10);assert(fingerprint(grid)==3145056629u);}
 {swgb::AiBuildPlacementGrid grid(32,11);grid.ownedBuilding(109,16,16,2,6,10,10);assert(fingerprint(grid)==3047606665u);}
 {swgb::AiBuildPlacementGrid grid(32,11);grid.ownedBuilding(70,16,16,2,6,10,10);assert(fingerprint(grid)==4290577461u);}
 {swgb::AiBuildPlacementGrid grid(32,11);grid.ownedBuilding(68,16,16,2,6,10,10);assert(fingerprint(grid)==4166922645u);}
 {swgb::AiBuildPlacementGrid grid(32,11);grid.ownedBuilding(12,16,16,2,6,10,10);assert(fingerprint(grid)==4290577461u);}
 {swgb::AiBuildPlacementGrid grid(32,11);grid.ownedBuilding(87,16,16,2,6,10,10);assert(fingerprint(grid)==4290577461u);}
 {swgb::AiBuildPlacementGrid grid(32,11);grid.ownedBuilding(335,16,16,2,6,10,10);assert(fingerprint(grid)==4290577461u);}
 {swgb::AiBuildPlacementGrid grid(32,11);grid.ownedBuilding(562,16,16,2,6,10,10);assert(fingerprint(grid)==4166922645u);}
 {swgb::AiBuildPlacementGrid grid(32,11);grid.ownedBuilding(563,16,16,2,6,10,10);assert(fingerprint(grid)==4290577461u);}
 {swgb::AiBuildPlacementGrid grid(32,11);grid.ownedBuilding(584,16,16,2,6,10,10);assert(fingerprint(grid)==4166922645u);}
 {swgb::AiBuildPlacementGrid grid(32,11);grid.ownedBuilding(585,16,16,2,6,10,10);assert(fingerprint(grid)==4290577461u);}
 {swgb::AiBuildPlacementGrid grid(32,13);grid.ownedBuilding(109,16,16,2,6,10,10);assert(fingerprint(grid)==3047606665u);}
 {swgb::AiBuildPlacementGrid grid(32,13);grid.ownedBuilding(70,16,16,2,6,10,10);assert(fingerprint(grid)==4290577461u);}
 {swgb::AiBuildPlacementGrid grid(32,13);grid.ownedBuilding(68,16,16,2,6,10,10);assert(fingerprint(grid)==4166922645u);}
 {swgb::AiBuildPlacementGrid grid(32,13);grid.ownedBuilding(12,16,16,2,6,10,10);assert(fingerprint(grid)==4290577461u);}
 {swgb::AiBuildPlacementGrid grid(32,13);grid.ownedBuilding(87,16,16,2,6,10,10);assert(fingerprint(grid)==4290577461u);}
 {swgb::AiBuildPlacementGrid grid(32,13);grid.ownedBuilding(335,16,16,2,6,10,10);assert(fingerprint(grid)==4290577461u);}
 {swgb::AiBuildPlacementGrid grid(32,13);grid.ownedBuilding(562,16,16,2,6,10,10);assert(fingerprint(grid)==4166922645u);}
 {swgb::AiBuildPlacementGrid grid(32,13);grid.ownedBuilding(563,16,16,2,6,10,10);assert(fingerprint(grid)==4290577461u);}
 {swgb::AiBuildPlacementGrid grid(32,13);grid.ownedBuilding(584,16,16,2,6,10,10);assert(fingerprint(grid)==4166922645u);}
 {swgb::AiBuildPlacementGrid grid(32,13);grid.ownedBuilding(585,16,16,2,6,10,10);assert(fingerprint(grid)==4290577461u);}
 {swgb::AiBuildPlacementGrid grid(32,11);grid.resource(0,27,16.0f,16.0f,0.5f,0.5f,false);assert(fingerprint(grid)==3322049246u);}
 {swgb::AiBuildPlacementGrid grid(32,11);grid.resource(0,27,0.25f,31.75f,0.5f,0.5f,false);assert(fingerprint(grid)==1778116230u);}
 {swgb::AiBuildPlacementGrid grid(32,11);grid.resource(0,27,16.0f,16.0f,0.5f,0.5f,true);assert(fingerprint(grid)==900439493u);}
 {swgb::AiBuildPlacementGrid grid(32,11);grid.resource(0,28,16.0f,16.0f,0.5f,0.5f,false);assert(fingerprint(grid)==3570042246u);}
 {swgb::AiBuildPlacementGrid grid(32,11);grid.resource(0,28,0.25f,31.75f,0.5f,0.5f,false);assert(fingerprint(grid)==1472089094u);}
 {swgb::AiBuildPlacementGrid grid(32,11);grid.resource(0,28,16.0f,16.0f,0.5f,0.5f,true);assert(fingerprint(grid)==900439493u);}
 {swgb::AiBuildPlacementGrid grid(32,11);grid.resource(2,29,16.0f,16.0f,0.5f,0.5f,false);assert(fingerprint(grid)==3981575254u);}
 {swgb::AiBuildPlacementGrid grid(32,11);grid.resource(2,29,0.25f,31.75f,0.5f,0.5f,false);assert(fingerprint(grid)==563219462u);}
 {swgb::AiBuildPlacementGrid grid(32,11);grid.resource(2,29,16.0f,16.0f,0.5f,0.5f,true);assert(fingerprint(grid)==900439493u);}
 {swgb::AiBuildPlacementGrid grid(32,11);grid.resource(3,30,16.0f,16.0f,0.5f,0.5f,false);assert(fingerprint(grid)==3981575254u);}
 {swgb::AiBuildPlacementGrid grid(32,11);grid.resource(3,30,0.25f,31.75f,0.5f,0.5f,false);assert(fingerprint(grid)==563219462u);}
 {swgb::AiBuildPlacementGrid grid(32,11);grid.resource(3,30,16.0f,16.0f,0.5f,0.5f,true);assert(fingerprint(grid)==900439493u);}
 {swgb::AiBuildPlacementGrid grid(32,11);grid.resource(1,31,16.0f,16.0f,0.5f,0.5f,false);assert(fingerprint(grid)==1309521158u);}
 {swgb::AiBuildPlacementGrid grid(32,11);grid.resource(1,31,0.25f,31.75f,0.5f,0.5f,false);assert(fingerprint(grid)==1883788060u);}
 {swgb::AiBuildPlacementGrid grid(32,11);grid.resource(1,31,16.0f,16.0f,0.5f,0.5f,true);assert(fingerprint(grid)==900439493u);}
 {swgb::AiBuildPlacementGrid grid(32,11);grid.resource(0,4,16.0f,16.0f,0.5f,0.5f,false);assert(fingerprint(grid)==3570042246u);}
 {swgb::AiBuildPlacementGrid grid(32,11);grid.resource(0,4,0.25f,31.75f,0.5f,0.5f,false);assert(fingerprint(grid)==1472089094u);}
 {swgb::AiBuildPlacementGrid grid(32,11);grid.resource(0,4,16.0f,16.0f,0.5f,0.5f,true);assert(fingerprint(grid)==900439493u);}
 {swgb::AiBuildPlacementGrid grid(32,13);grid.resource(0,27,16.0f,16.0f,0.5f,0.5f,false);assert(fingerprint(grid)==3322049246u);}
 {swgb::AiBuildPlacementGrid grid(32,13);grid.resource(0,27,0.25f,31.75f,0.5f,0.5f,false);assert(fingerprint(grid)==1778116230u);}
 {swgb::AiBuildPlacementGrid grid(32,13);grid.resource(0,27,16.0f,16.0f,0.5f,0.5f,true);assert(fingerprint(grid)==900439493u);}
 {swgb::AiBuildPlacementGrid grid(32,13);grid.resource(0,28,16.0f,16.0f,0.5f,0.5f,false);assert(fingerprint(grid)==3570042246u);}
 {swgb::AiBuildPlacementGrid grid(32,13);grid.resource(0,28,0.25f,31.75f,0.5f,0.5f,false);assert(fingerprint(grid)==1472089094u);}
 {swgb::AiBuildPlacementGrid grid(32,13);grid.resource(0,28,16.0f,16.0f,0.5f,0.5f,true);assert(fingerprint(grid)==900439493u);}
 {swgb::AiBuildPlacementGrid grid(32,13);grid.resource(2,29,16.0f,16.0f,0.5f,0.5f,false);assert(fingerprint(grid)==1980717710u);}
 {swgb::AiBuildPlacementGrid grid(32,13);grid.resource(2,29,0.25f,31.75f,0.5f,0.5f,false);assert(fingerprint(grid)==2147406854u);}
 {swgb::AiBuildPlacementGrid grid(32,13);grid.resource(2,29,16.0f,16.0f,0.5f,0.5f,true);assert(fingerprint(grid)==900439493u);}
 {swgb::AiBuildPlacementGrid grid(32,13);grid.resource(3,30,16.0f,16.0f,0.5f,0.5f,false);assert(fingerprint(grid)==3490044638u);}
 {swgb::AiBuildPlacementGrid grid(32,13);grid.resource(3,30,0.25f,31.75f,0.5f,0.5f,false);assert(fingerprint(grid)==227054318u);}
 {swgb::AiBuildPlacementGrid grid(32,13);grid.resource(3,30,16.0f,16.0f,0.5f,0.5f,true);assert(fingerprint(grid)==900439493u);}
 {swgb::AiBuildPlacementGrid grid(32,13);grid.resource(1,31,16.0f,16.0f,0.5f,0.5f,false);assert(fingerprint(grid)==1980717710u);}
 {swgb::AiBuildPlacementGrid grid(32,13);grid.resource(1,31,0.25f,31.75f,0.5f,0.5f,false);assert(fingerprint(grid)==2147406854u);}
 {swgb::AiBuildPlacementGrid grid(32,13);grid.resource(1,31,16.0f,16.0f,0.5f,0.5f,true);assert(fingerprint(grid)==900439493u);}
 {swgb::AiBuildPlacementGrid grid(32,13);grid.resource(0,4,16.0f,16.0f,0.5f,0.5f,false);assert(fingerprint(grid)==3570042246u);}
 {swgb::AiBuildPlacementGrid grid(32,13);grid.resource(0,4,0.25f,31.75f,0.5f,0.5f,false);assert(fingerprint(grid)==1472089094u);}
 {swgb::AiBuildPlacementGrid grid(32,13);grid.resource(0,4,16.0f,16.0f,0.5f,0.5f,true);assert(fingerprint(grid)==900439493u);}
 std::puts("PASS: original resource-depot scan, owned and resource-grid controls");
}
