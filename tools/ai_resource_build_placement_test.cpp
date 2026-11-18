// SPDX-License-Identifier: GPL-3.0-or-later
// Original588b30 scan and58e2d0 grid controls, 2026-11-18.
#include "../src/engine/ai_build_placement.h"
#include "../src/engine/ai_prefab_grid.h"
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
uint32_t fingerprint(const swgb::AiBuildPlacementGrid &grid) {
 uint32_t hash=2166136261u;for(auto value:grid.cells())hash=(hash^value)*16777619u;return hash;
}
int main() {
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
