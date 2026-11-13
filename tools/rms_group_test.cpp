// SPDX-License-Identifier: GPL-3.0-or-later
#include "engine/rms_objects.h"
#include "core/genie_dat.h"
#include <cassert>
#include <cstring>
#include <iostream>
static swgb::dat::DatFile fixtureData() {
 swgb::dat::DatFile d;d.civs.resize(1);d.civs[0].units.resize(349);d.graphics.resize(5283);d.terrainRestrictions.resize(12);
 for(int id:{348,66,102}) {
  auto &u=d.civs[0].units[id];u.exists=true;u.id=id;u.type=id==348?10:30;u.cls=id==348?31:id==66?29:30;
  u.hitPoints=id==348?50:0;u.standingGraphic[0]=id==348?5282:id==66?5134:5232;
  u.collisionSize[0]=u.collisionSize[1]=u.clearanceSize[0]=u.clearanceSize[1]=.5f;u.collisionSize[2]=id==348?2:1;
  u.placementSideTerrain[0]=u.placementSideTerrain[1]=u.placementTerrain[0]=u.placementTerrain[1]=-1;
  u.terrainRestriction=id==348?11:8;u.obstructionType=2;u.obstructionClass=1;u.enableAutoGather=1;u.createDoppelgangerOnDeath=id==348?2:1;
  for(auto &v:u.resourceStorages)v={-1,0,0};u.resourceStorages[0]={int16_t(id==348?1:id==66?3:2),float(id==348?100:id==66?800:350),0};
  auto &g=d.graphics[u.standingGraphic[0]];g.exists=true;g.sequenceType=6;g.angleCount=id==348?12:10;g.frameCount=1;
 }
 d.terrainRestrictions[8].passableBuildableDmgMultiplier={1.f,0.f,0.f,1.f,0.f,1.f,1.f,0.f,0.f,1.f,1.f,1.f,1.f,1.f,1.f,0.f,0.f,1.f,1.f,1.f,1.f,1.f,0.f,0.f,0.00999999978f,0.00999999978f,1.f,1.f,0.f,0.f,0.f,0.f,1.f,1.f,1.f,0.f,1.f,0.f,1.f,1.f,0.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,0.f,1.f,1.f,0.f,1.f};
 d.terrainRestrictions[11].passableBuildableDmgMultiplier={1.f,0.f,0.f,1.f,0.f,1.f,1.f,0.f,0.f,1.f,1.f,1.f,1.f,1.f,1.f,0.f,0.f,1.f,1.f,1.f,1.f,1.f,0.f,0.f,0.00999999978f,0.00999999978f,1.f,1.f,0.f,0.f,0.f,0.f,1.f,1.f,1.f,0.f,1.f,0.f,0.00999999978f,0.00999999978f,0.f,1.f,0.00999999978f,1.f,0.00999999978f,1.f,1.f,1.f,0.f,1.f,1.f,0.f,1.f};
return d;
}
int main(){auto data=fixtureData();std::string error;std::vector<swgb::RmsTimberPlacement> out;
{ uint32_t rng=1u; swgb::RmsResourceGroups cfg{2,2,5,0,2};
assert(swgb::generateRmsGroupedResourcePlacements(7,7,{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},data,348,cfg,rng,out,&error,{}));
assert(rng==3999086192u && out.size()==10);
{uint32_t bits;std::memcpy(&bits,&out[0].headingRadians,4);assert(out[0].x==0.5f && out[0].y==4.5f && out[0].facet==6 && bits==0u && out[0].unitId==348);}
{uint32_t bits;std::memcpy(&bits,&out[1].headingRadians,4);assert(out[1].x==0.5f && out[1].y==3.5f && out[1].facet==4 && bits==0u && out[1].unitId==348);}
{uint32_t bits;std::memcpy(&bits,&out[2].headingRadians,4);assert(out[2].x==0.5f && out[2].y==5.5f && out[2].facet==11 && bits==0u && out[2].unitId==348);}
{uint32_t bits;std::memcpy(&bits,&out[3].headingRadians,4);assert(out[3].x==0.5f && out[3].y==6.5f && out[3].facet==0 && bits==0u && out[3].unitId==348);}
{uint32_t bits;std::memcpy(&bits,&out[4].headingRadians,4);assert(out[4].x==1.5f && out[4].y==6.5f && out[4].facet==4 && bits==0u && out[4].unitId==348);}
{uint32_t bits;std::memcpy(&bits,&out[5].headingRadians,4);assert(out[5].x==4.5f && out[5].y==3.5f && out[5].facet==7 && bits==0u && out[5].unitId==348);}
{uint32_t bits;std::memcpy(&bits,&out[6].headingRadians,4);assert(out[6].x==3.5f && out[6].y==3.5f && out[6].facet==0 && bits==0u && out[6].unitId==348);}
{uint32_t bits;std::memcpy(&bits,&out[7].headingRadians,4);assert(out[7].x==4.5f && out[7].y==4.5f && out[7].facet==6 && bits==0u && out[7].unitId==348);}
{uint32_t bits;std::memcpy(&bits,&out[8].headingRadians,4);assert(out[8].x==3.5f && out[8].y==4.5f && out[8].facet==11 && bits==0u && out[8].unitId==348);}
{uint32_t bits;std::memcpy(&bits,&out[9].headingRadians,4);assert(out[9].x==4.5f && out[9].y==2.5f && out[9].facet==1 && bits==0u && out[9].unitId==348);}
}
{ uint32_t rng=12345u; swgb::RmsResourceGroups cfg{2,2,5,0,2};
assert(swgb::generateRmsGroupedResourcePlacements(7,7,{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},data,348,cfg,rng,out,&error,{}));
assert(rng==3325886315u && out.size()==10);
{uint32_t bits;std::memcpy(&bits,&out[0].headingRadians,4);assert(out[0].x==2.5f && out[0].y==2.5f && out[0].facet==7 && bits==0u && out[0].unitId==348);}
{uint32_t bits;std::memcpy(&bits,&out[1].headingRadians,4);assert(out[1].x==1.5f && out[1].y==2.5f && out[1].facet==9 && bits==0u && out[1].unitId==348);}
{uint32_t bits;std::memcpy(&bits,&out[2].headingRadians,4);assert(out[2].x==0.5f && out[2].y==2.5f && out[2].facet==4 && bits==0u && out[2].unitId==348);}
{uint32_t bits;std::memcpy(&bits,&out[3].headingRadians,4);assert(out[3].x==0.5f && out[3].y==3.5f && out[3].facet==3 && bits==0u && out[3].unitId==348);}
{uint32_t bits;std::memcpy(&bits,&out[4].headingRadians,4);assert(out[4].x==0.5f && out[4].y==4.5f && out[4].facet==0 && bits==0u && out[4].unitId==348);}
{uint32_t bits;std::memcpy(&bits,&out[5].headingRadians,4);assert(out[5].x==4.5f && out[5].y==3.5f && out[5].facet==0 && bits==0u && out[5].unitId==348);}
{uint32_t bits;std::memcpy(&bits,&out[6].headingRadians,4);assert(out[6].x==4.5f && out[6].y==2.5f && out[6].facet==2 && bits==0u && out[6].unitId==348);}
{uint32_t bits;std::memcpy(&bits,&out[7].headingRadians,4);assert(out[7].x==3.5f && out[7].y==3.5f && out[7].facet==5 && bits==0u && out[7].unitId==348);}
{uint32_t bits;std::memcpy(&bits,&out[8].headingRadians,4);assert(out[8].x==2.5f && out[8].y==3.5f && out[8].facet==4 && bits==0u && out[8].unitId==348);}
{uint32_t bits;std::memcpy(&bits,&out[9].headingRadians,4);assert(out[9].x==5.5f && out[9].y==2.5f && out[9].facet==0 && bits==0u && out[9].unitId==348);}
}
{ uint32_t rng=1u; swgb::RmsResourceGroups cfg{1,2,5,0,2};
assert(swgb::generateRmsGroupedResourcePlacements(7,7,{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},data,348,cfg,rng,out,&error,{}));
assert(rng==1226606285u && out.size()==10);
{uint32_t bits;std::memcpy(&bits,&out[0].headingRadians,4);assert(out[0].x==0.5f && out[0].y==2.5f && out[0].facet==6 && bits==0u && out[0].unitId==348);}
{uint32_t bits;std::memcpy(&bits,&out[1].headingRadians,4);assert(out[1].x==1.5f && out[1].y==2.5f && out[1].facet==3 && bits==0u && out[1].unitId==348);}
{uint32_t bits;std::memcpy(&bits,&out[2].headingRadians,4);assert(out[2].x==2.5f && out[2].y==2.5f && out[2].facet==0 && bits==0u && out[2].unitId==348);}
{uint32_t bits;std::memcpy(&bits,&out[3].headingRadians,4);assert(out[3].x==0.5f && out[3].y==3.5f && out[3].facet==1 && bits==0u && out[3].unitId==348);}
{uint32_t bits;std::memcpy(&bits,&out[4].headingRadians,4);assert(out[4].x==1.5f && out[4].y==3.5f && out[4].facet==4 && bits==0u && out[4].unitId==348);}
{uint32_t bits;std::memcpy(&bits,&out[5].headingRadians,4);assert(out[5].x==4.5f && out[5].y==2.5f && out[5].facet==0 && bits==0u && out[5].unitId==348);}
{uint32_t bits;std::memcpy(&bits,&out[6].headingRadians,4);assert(out[6].x==2.5f && out[6].y==1.5f && out[6].facet==0 && bits==0u && out[6].unitId==348);}
{uint32_t bits;std::memcpy(&bits,&out[7].headingRadians,4);assert(out[7].x==3.5f && out[7].y==1.5f && out[7].facet==4 && bits==0u && out[7].unitId==348);}
{uint32_t bits;std::memcpy(&bits,&out[8].headingRadians,4);assert(out[8].x==4.5f && out[8].y==1.5f && out[8].facet==6 && bits==0u && out[8].unitId==348);}
{uint32_t bits;std::memcpy(&bits,&out[9].headingRadians,4);assert(out[9].x==5.5f && out[9].y==1.5f && out[9].facet==6 && bits==0u && out[9].unitId==348);}
}
{ uint32_t rng=12345u; swgb::RmsResourceGroups cfg{1,2,5,0,2};
assert(swgb::generateRmsGroupedResourcePlacements(7,7,{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},data,348,cfg,rng,out,&error,{}));
assert(rng==2382629253u && out.size()==10);
{uint32_t bits;std::memcpy(&bits,&out[0].headingRadians,4);assert(out[0].x==0.5f && out[0].y==1.5f && out[0].facet==3 && bits==0u && out[0].unitId==348);}
{uint32_t bits;std::memcpy(&bits,&out[1].headingRadians,4);assert(out[1].x==1.5f && out[1].y==1.5f && out[1].facet==9 && bits==0u && out[1].unitId==348);}
{uint32_t bits;std::memcpy(&bits,&out[2].headingRadians,4);assert(out[2].x==0.5f && out[2].y==0.5f && out[2].facet==1 && bits==0u && out[2].unitId==348);}
{uint32_t bits;std::memcpy(&bits,&out[3].headingRadians,4);assert(out[3].x==1.5f && out[3].y==0.5f && out[3].facet==3 && bits==0u && out[3].unitId==348);}
{uint32_t bits;std::memcpy(&bits,&out[4].headingRadians,4);assert(out[4].x==2.5f && out[4].y==0.5f && out[4].facet==3 && bits==0u && out[4].unitId==348);}
{uint32_t bits;std::memcpy(&bits,&out[5].headingRadians,4);assert(out[5].x==3.5f && out[5].y==0.5f && out[5].facet==5 && bits==0u && out[5].unitId==348);}
{uint32_t bits;std::memcpy(&bits,&out[6].headingRadians,4);assert(out[6].x==4.5f && out[6].y==0.5f && out[6].facet==0 && bits==0u && out[6].unitId==348);}
{uint32_t bits;std::memcpy(&bits,&out[7].headingRadians,4);assert(out[7].x==5.5f && out[7].y==0.5f && out[7].facet==3 && bits==0u && out[7].unitId==348);}
{uint32_t bits;std::memcpy(&bits,&out[8].headingRadians,4);assert(out[8].x==6.5f && out[8].y==0.5f && out[8].facet==7 && bits==0u && out[8].unitId==348);}
{uint32_t bits;std::memcpy(&bits,&out[9].headingRadians,4);assert(out[9].x==3.5f && out[9].y==1.5f && out[9].facet==1 && bits==0u && out[9].unitId==348);}
}
{ uint32_t rng=1u; swgb::RmsResourceGroups cfg{2,2,5,0,2};
assert(swgb::generateRmsGroupedResourcePlacements(7,7,{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},data,66,cfg,rng,out,&error,{}));
assert(rng==3707600809u && out.size()==10);
{uint32_t bits;std::memcpy(&bits,&out[0].headingRadians,4);assert(out[0].x==0.5f && out[0].y==4.5f && out[0].facet==9 && bits==0u && out[0].unitId==66);}
{uint32_t bits;std::memcpy(&bits,&out[1].headingRadians,4);assert(out[1].x==1.5f && out[1].y==4.5f && out[1].facet==3 && bits==1075894652u && out[1].unitId==66);}
{uint32_t bits;std::memcpy(&bits,&out[2].headingRadians,4);assert(out[2].x==1.5f && out[2].y==3.5f && out[2].facet==4 && bits==1078530011u && out[2].unitId==66);}
{uint32_t bits;std::memcpy(&bits,&out[3].headingRadians,4);assert(out[3].x==1.5f && out[3].y==5.5f && out[3].facet==2 && bits==1072776762u && out[3].unitId==66);}
{uint32_t bits;std::memcpy(&bits,&out[4].headingRadians,4);assert(out[4].x==0.5f && out[4].y==5.5f && out[4].facet==2 && bits==1072776762u && out[4].unitId==66);}
{uint32_t bits;std::memcpy(&bits,&out[5].headingRadians,4);assert(out[5].x==4.5f && out[5].y==3.5f && out[5].facet==4 && bits==1078530011u && out[5].unitId==66);}
{uint32_t bits;std::memcpy(&bits,&out[6].headingRadians,4);assert(out[6].x==3.5f && out[6].y==3.5f && out[6].facet==7 && bits==1084283260u && out[6].unitId==66);}
{uint32_t bits;std::memcpy(&bits,&out[7].headingRadians,4);assert(out[7].x==4.5f && out[7].y==2.5f && out[7].facet==1 && bits==1067506044u && out[7].unitId==66);}
{uint32_t bits;std::memcpy(&bits,&out[8].headingRadians,4);assert(out[8].x==5.5f && out[8].y==2.5f && out[8].facet==9 && bits==0u && out[8].unitId==66);}
{uint32_t bits;std::memcpy(&bits,&out[9].headingRadians,4);assert(out[9].x==6.5f && out[9].y==2.5f && out[9].facet==5 && bits==1081165370u && out[9].unitId==66);}
}
{ uint32_t rng=12345u; swgb::RmsResourceGroups cfg{2,2,5,0,2};
assert(swgb::generateRmsGroupedResourcePlacements(7,7,{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},data,66,cfg,rng,out,&error,{}));
assert(rng==1627187356u && out.size()==10);
{uint32_t bits;std::memcpy(&bits,&out[0].headingRadians,4);assert(out[0].x==2.5f && out[0].y==2.5f && out[0].facet==3 && bits==1075894652u && out[0].unitId==66);}
{uint32_t bits;std::memcpy(&bits,&out[1].headingRadians,4);assert(out[1].x==2.5f && out[1].y==3.5f && out[1].facet==2 && bits==1072776762u && out[1].unitId==66);}
{uint32_t bits;std::memcpy(&bits,&out[2].headingRadians,4);assert(out[2].x==3.5f && out[2].y==3.5f && out[2].facet==9 && bits==0u && out[2].unitId==66);}
{uint32_t bits;std::memcpy(&bits,&out[3].headingRadians,4);assert(out[3].x==3.5f && out[3].y==4.5f && out[3].facet==0 && bits==1059117436u && out[3].unitId==66);}
{uint32_t bits;std::memcpy(&bits,&out[4].headingRadians,4);assert(out[4].x==4.5f && out[4].y==4.5f && out[4].facet==7 && bits==1084283260u && out[4].unitId==66);}
{uint32_t bits;std::memcpy(&bits,&out[5].headingRadians,4);assert(out[5].x==4.5f && out[5].y==0.5f && out[5].facet==2 && bits==1072776762u && out[5].unitId==66);}
{uint32_t bits;std::memcpy(&bits,&out[6].headingRadians,4);assert(out[6].x==5.5f && out[6].y==0.5f && out[6].facet==6 && bits==1082965580u && out[6].unitId==66);}
{uint32_t bits;std::memcpy(&bits,&out[7].headingRadians,4);assert(out[7].x==4.5f && out[7].y==1.5f && out[7].facet==1 && bits==1067506044u && out[7].unitId==66);}
{uint32_t bits;std::memcpy(&bits,&out[8].headingRadians,4);assert(out[8].x==3.5f && out[8].y==1.5f && out[8].facet==0 && bits==1059117436u && out[8].unitId==66);}
{uint32_t bits;std::memcpy(&bits,&out[9].headingRadians,4);assert(out[9].x==3.5f && out[9].y==2.5f && out[9].facet==1 && bits==1067506044u && out[9].unitId==66);}
}
{ uint32_t rng=1u; swgb::RmsResourceGroups cfg{1,2,5,0,2};
assert(swgb::generateRmsGroupedResourcePlacements(7,7,{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},data,66,cfg,rng,out,&error,{}));
assert(rng==2652862833u && out.size()==10);
{uint32_t bits;std::memcpy(&bits,&out[0].headingRadians,4);assert(out[0].x==0.5f && out[0].y==2.5f && out[0].facet==9 && bits==0u && out[0].unitId==66);}
{uint32_t bits;std::memcpy(&bits,&out[1].headingRadians,4);assert(out[1].x==1.5f && out[1].y==2.5f && out[1].facet==0 && bits==1059117436u && out[1].unitId==66);}
{uint32_t bits;std::memcpy(&bits,&out[2].headingRadians,4);assert(out[2].x==2.5f && out[2].y==2.5f && out[2].facet==3 && bits==1075894652u && out[2].unitId==66);}
{uint32_t bits;std::memcpy(&bits,&out[3].headingRadians,4);assert(out[3].x==0.5f && out[3].y==3.5f && out[3].facet==9 && bits==0u && out[3].unitId==66);}
{uint32_t bits;std::memcpy(&bits,&out[4].headingRadians,4);assert(out[4].x==1.5f && out[4].y==3.5f && out[4].facet==4 && bits==1078530011u && out[4].unitId==66);}
{uint32_t bits;std::memcpy(&bits,&out[5].headingRadians,4);assert(out[5].x==3.5f && out[5].y==2.5f && out[5].facet==5 && bits==1081165370u && out[5].unitId==66);}
{uint32_t bits;std::memcpy(&bits,&out[6].headingRadians,4);assert(out[6].x==2.5f && out[6].y==1.5f && out[6].facet==4 && bits==1078530011u && out[6].unitId==66);}
{uint32_t bits;std::memcpy(&bits,&out[7].headingRadians,4);assert(out[7].x==3.5f && out[7].y==1.5f && out[7].facet==6 && bits==1082965580u && out[7].unitId==66);}
{uint32_t bits;std::memcpy(&bits,&out[8].headingRadians,4);assert(out[8].x==4.5f && out[8].y==1.5f && out[8].facet==4 && bits==1078530011u && out[8].unitId==66);}
{uint32_t bits;std::memcpy(&bits,&out[9].headingRadians,4);assert(out[9].x==5.5f && out[9].y==1.5f && out[9].facet==1 && bits==1067506044u && out[9].unitId==66);}
}
{ uint32_t rng=12345u; swgb::RmsResourceGroups cfg{1,2,5,0,2};
assert(swgb::generateRmsGroupedResourcePlacements(7,7,{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},data,66,cfg,rng,out,&error,{}));
assert(rng==493972393u && out.size()==10);
{uint32_t bits;std::memcpy(&bits,&out[0].headingRadians,4);assert(out[0].x==0.5f && out[0].y==1.5f && out[0].facet==0 && bits==1059117436u && out[0].unitId==66);}
{uint32_t bits;std::memcpy(&bits,&out[1].headingRadians,4);assert(out[1].x==1.5f && out[1].y==1.5f && out[1].facet==2 && bits==1072776762u && out[1].unitId==66);}
{uint32_t bits;std::memcpy(&bits,&out[2].headingRadians,4);assert(out[2].x==0.5f && out[2].y==0.5f && out[2].facet==2 && bits==1072776762u && out[2].unitId==66);}
{uint32_t bits;std::memcpy(&bits,&out[3].headingRadians,4);assert(out[3].x==1.5f && out[3].y==0.5f && out[3].facet==9 && bits==0u && out[3].unitId==66);}
{uint32_t bits;std::memcpy(&bits,&out[4].headingRadians,4);assert(out[4].x==2.5f && out[4].y==0.5f && out[4].facet==9 && bits==0u && out[4].unitId==66);}
{uint32_t bits;std::memcpy(&bits,&out[5].headingRadians,4);assert(out[5].x==3.5f && out[5].y==0.5f && out[5].facet==5 && bits==1081165370u && out[5].unitId==66);}
{uint32_t bits;std::memcpy(&bits,&out[6].headingRadians,4);assert(out[6].x==4.5f && out[6].y==0.5f && out[6].facet==3 && bits==1075894652u && out[6].unitId==66);}
{uint32_t bits;std::memcpy(&bits,&out[7].headingRadians,4);assert(out[7].x==5.5f && out[7].y==0.5f && out[7].facet==3 && bits==1075894652u && out[7].unitId==66);}
{uint32_t bits;std::memcpy(&bits,&out[8].headingRadians,4);assert(out[8].x==6.5f && out[8].y==0.5f && out[8].facet==2 && bits==1072776762u && out[8].unitId==66);}
{uint32_t bits;std::memcpy(&bits,&out[9].headingRadians,4);assert(out[9].x==3.5f && out[9].y==1.5f && out[9].facet==1 && bits==1067506044u && out[9].unitId==66);}
}
{ uint32_t rng=1u; swgb::RmsResourceGroups cfg{2,2,5,0,2};
assert(swgb::generateRmsGroupedResourcePlacements(7,7,{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},data,102,cfg,rng,out,&error,{}));
assert(rng==3707600809u && out.size()==10);
{uint32_t bits;std::memcpy(&bits,&out[0].headingRadians,4);assert(out[0].x==0.5f && out[0].y==4.5f && out[0].facet==9 && bits==0u && out[0].unitId==102);}
{uint32_t bits;std::memcpy(&bits,&out[1].headingRadians,4);assert(out[1].x==1.5f && out[1].y==4.5f && out[1].facet==3 && bits==1075894652u && out[1].unitId==102);}
{uint32_t bits;std::memcpy(&bits,&out[2].headingRadians,4);assert(out[2].x==1.5f && out[2].y==3.5f && out[2].facet==4 && bits==1078530011u && out[2].unitId==102);}
{uint32_t bits;std::memcpy(&bits,&out[3].headingRadians,4);assert(out[3].x==1.5f && out[3].y==5.5f && out[3].facet==2 && bits==1072776762u && out[3].unitId==102);}
{uint32_t bits;std::memcpy(&bits,&out[4].headingRadians,4);assert(out[4].x==0.5f && out[4].y==5.5f && out[4].facet==2 && bits==1072776762u && out[4].unitId==102);}
{uint32_t bits;std::memcpy(&bits,&out[5].headingRadians,4);assert(out[5].x==4.5f && out[5].y==3.5f && out[5].facet==4 && bits==1078530011u && out[5].unitId==102);}
{uint32_t bits;std::memcpy(&bits,&out[6].headingRadians,4);assert(out[6].x==3.5f && out[6].y==3.5f && out[6].facet==7 && bits==1084283260u && out[6].unitId==102);}
{uint32_t bits;std::memcpy(&bits,&out[7].headingRadians,4);assert(out[7].x==4.5f && out[7].y==2.5f && out[7].facet==1 && bits==1067506044u && out[7].unitId==102);}
{uint32_t bits;std::memcpy(&bits,&out[8].headingRadians,4);assert(out[8].x==5.5f && out[8].y==2.5f && out[8].facet==9 && bits==0u && out[8].unitId==102);}
{uint32_t bits;std::memcpy(&bits,&out[9].headingRadians,4);assert(out[9].x==6.5f && out[9].y==2.5f && out[9].facet==5 && bits==1081165370u && out[9].unitId==102);}
}
{ uint32_t rng=12345u; swgb::RmsResourceGroups cfg{2,2,5,0,2};
assert(swgb::generateRmsGroupedResourcePlacements(7,7,{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},data,102,cfg,rng,out,&error,{}));
assert(rng==1627187356u && out.size()==10);
{uint32_t bits;std::memcpy(&bits,&out[0].headingRadians,4);assert(out[0].x==2.5f && out[0].y==2.5f && out[0].facet==3 && bits==1075894652u && out[0].unitId==102);}
{uint32_t bits;std::memcpy(&bits,&out[1].headingRadians,4);assert(out[1].x==2.5f && out[1].y==3.5f && out[1].facet==2 && bits==1072776762u && out[1].unitId==102);}
{uint32_t bits;std::memcpy(&bits,&out[2].headingRadians,4);assert(out[2].x==3.5f && out[2].y==3.5f && out[2].facet==9 && bits==0u && out[2].unitId==102);}
{uint32_t bits;std::memcpy(&bits,&out[3].headingRadians,4);assert(out[3].x==3.5f && out[3].y==4.5f && out[3].facet==0 && bits==1059117436u && out[3].unitId==102);}
{uint32_t bits;std::memcpy(&bits,&out[4].headingRadians,4);assert(out[4].x==4.5f && out[4].y==4.5f && out[4].facet==7 && bits==1084283260u && out[4].unitId==102);}
{uint32_t bits;std::memcpy(&bits,&out[5].headingRadians,4);assert(out[5].x==4.5f && out[5].y==0.5f && out[5].facet==2 && bits==1072776762u && out[5].unitId==102);}
{uint32_t bits;std::memcpy(&bits,&out[6].headingRadians,4);assert(out[6].x==5.5f && out[6].y==0.5f && out[6].facet==6 && bits==1082965580u && out[6].unitId==102);}
{uint32_t bits;std::memcpy(&bits,&out[7].headingRadians,4);assert(out[7].x==4.5f && out[7].y==1.5f && out[7].facet==1 && bits==1067506044u && out[7].unitId==102);}
{uint32_t bits;std::memcpy(&bits,&out[8].headingRadians,4);assert(out[8].x==3.5f && out[8].y==1.5f && out[8].facet==0 && bits==1059117436u && out[8].unitId==102);}
{uint32_t bits;std::memcpy(&bits,&out[9].headingRadians,4);assert(out[9].x==3.5f && out[9].y==2.5f && out[9].facet==1 && bits==1067506044u && out[9].unitId==102);}
}
{ uint32_t rng=1u; swgb::RmsResourceGroups cfg{1,2,5,0,2};
assert(swgb::generateRmsGroupedResourcePlacements(7,7,{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},data,102,cfg,rng,out,&error,{}));
assert(rng==2652862833u && out.size()==10);
{uint32_t bits;std::memcpy(&bits,&out[0].headingRadians,4);assert(out[0].x==0.5f && out[0].y==2.5f && out[0].facet==9 && bits==0u && out[0].unitId==102);}
{uint32_t bits;std::memcpy(&bits,&out[1].headingRadians,4);assert(out[1].x==1.5f && out[1].y==2.5f && out[1].facet==0 && bits==1059117436u && out[1].unitId==102);}
{uint32_t bits;std::memcpy(&bits,&out[2].headingRadians,4);assert(out[2].x==2.5f && out[2].y==2.5f && out[2].facet==3 && bits==1075894652u && out[2].unitId==102);}
{uint32_t bits;std::memcpy(&bits,&out[3].headingRadians,4);assert(out[3].x==0.5f && out[3].y==3.5f && out[3].facet==9 && bits==0u && out[3].unitId==102);}
{uint32_t bits;std::memcpy(&bits,&out[4].headingRadians,4);assert(out[4].x==1.5f && out[4].y==3.5f && out[4].facet==4 && bits==1078530011u && out[4].unitId==102);}
{uint32_t bits;std::memcpy(&bits,&out[5].headingRadians,4);assert(out[5].x==3.5f && out[5].y==2.5f && out[5].facet==5 && bits==1081165370u && out[5].unitId==102);}
{uint32_t bits;std::memcpy(&bits,&out[6].headingRadians,4);assert(out[6].x==2.5f && out[6].y==1.5f && out[6].facet==4 && bits==1078530011u && out[6].unitId==102);}
{uint32_t bits;std::memcpy(&bits,&out[7].headingRadians,4);assert(out[7].x==3.5f && out[7].y==1.5f && out[7].facet==6 && bits==1082965580u && out[7].unitId==102);}
{uint32_t bits;std::memcpy(&bits,&out[8].headingRadians,4);assert(out[8].x==4.5f && out[8].y==1.5f && out[8].facet==4 && bits==1078530011u && out[8].unitId==102);}
{uint32_t bits;std::memcpy(&bits,&out[9].headingRadians,4);assert(out[9].x==5.5f && out[9].y==1.5f && out[9].facet==1 && bits==1067506044u && out[9].unitId==102);}
}
{ uint32_t rng=12345u; swgb::RmsResourceGroups cfg{1,2,5,0,2};
assert(swgb::generateRmsGroupedResourcePlacements(7,7,{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},data,102,cfg,rng,out,&error,{}));
assert(rng==493972393u && out.size()==10);
{uint32_t bits;std::memcpy(&bits,&out[0].headingRadians,4);assert(out[0].x==0.5f && out[0].y==1.5f && out[0].facet==0 && bits==1059117436u && out[0].unitId==102);}
{uint32_t bits;std::memcpy(&bits,&out[1].headingRadians,4);assert(out[1].x==1.5f && out[1].y==1.5f && out[1].facet==2 && bits==1072776762u && out[1].unitId==102);}
{uint32_t bits;std::memcpy(&bits,&out[2].headingRadians,4);assert(out[2].x==0.5f && out[2].y==0.5f && out[2].facet==2 && bits==1072776762u && out[2].unitId==102);}
{uint32_t bits;std::memcpy(&bits,&out[3].headingRadians,4);assert(out[3].x==1.5f && out[3].y==0.5f && out[3].facet==9 && bits==0u && out[3].unitId==102);}
{uint32_t bits;std::memcpy(&bits,&out[4].headingRadians,4);assert(out[4].x==2.5f && out[4].y==0.5f && out[4].facet==9 && bits==0u && out[4].unitId==102);}
{uint32_t bits;std::memcpy(&bits,&out[5].headingRadians,4);assert(out[5].x==3.5f && out[5].y==0.5f && out[5].facet==5 && bits==1081165370u && out[5].unitId==102);}
{uint32_t bits;std::memcpy(&bits,&out[6].headingRadians,4);assert(out[6].x==4.5f && out[6].y==0.5f && out[6].facet==3 && bits==1075894652u && out[6].unitId==102);}
{uint32_t bits;std::memcpy(&bits,&out[7].headingRadians,4);assert(out[7].x==5.5f && out[7].y==0.5f && out[7].facet==3 && bits==1075894652u && out[7].unitId==102);}
{uint32_t bits;std::memcpy(&bits,&out[8].headingRadians,4);assert(out[8].x==6.5f && out[8].y==0.5f && out[8].facet==2 && bits==1072776762u && out[8].unitId==102);}
{uint32_t bits;std::memcpy(&bits,&out[9].headingRadians,4);assert(out[9].x==3.5f && out[9].y==1.5f && out[9].facet==1 && bits==1067506044u && out[9].unitId==102);}
}
{ uint32_t rng=4028364353u; swgb::RmsResourceGroups cfg{1,1,2,2,3};
assert(swgb::generateRmsGroupedResourcePlacements(3,3,{0,0,0,0,0,0,0,0,0},data,66,cfg,rng,out,&error,{}));
assert(rng==1641283084u && out.size()==4);
{uint32_t bits;std::memcpy(&bits,&out[0].headingRadians,4);assert(out[0].x==0.5f && out[0].y==0.5f && out[0].facet==2 && bits==1072776762u && out[0].unitId==66);}
{uint32_t bits;std::memcpy(&bits,&out[1].headingRadians,4);assert(out[1].x==1.5f && out[1].y==0.5f && out[1].facet==8 && bits==1085600939u && out[1].unitId==66);}
{uint32_t bits;std::memcpy(&bits,&out[2].headingRadians,4);assert(out[2].x==2.5f && out[2].y==0.5f && out[2].facet==0 && bits==1059117436u && out[2].unitId==66);}
{uint32_t bits;std::memcpy(&bits,&out[3].headingRadians,4);assert(out[3].x==0.5f && out[3].y==1.5f && out[3].facet==6 && bits==1082965580u && out[3].unitId==66);}
}
{ uint32_t rng=1u; swgb::RmsResourceGroups cfg{1,1,0,0,0};
assert(swgb::generateRmsGroupedResourcePlacements(1,1,{0},data,348,cfg,rng,out,&error,{}));
assert(rng==3357800067u && out.size()==1);
{uint32_t bits;std::memcpy(&bits,&out[0].headingRadians,4);assert(out[0].x==0.5f && out[0].y==0.5f && out[0].facet==6 && bits==0u && out[0].unitId==348);}
}
{ uint32_t rng=12345u; swgb::RmsResourceGroups cfg{1,0,5,0,2};
assert(swgb::generateRmsGroupedResourcePlacements(4,4,{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},data,102,cfg,rng,out,&error,{}));
assert(rng==1255958651u && out.size()==0);
}
{ uint32_t rng=4028364353u; swgb::RmsResourceGroups cfg{2,1,2,2,3};
assert(swgb::generateRmsGroupedResourcePlacements(3,3,{0,0,0,0,0,0,0,0,0},data,66,cfg,rng,out,&error,{}));
assert(rng==2378641492u && out.size()==4);
{uint32_t bits;std::memcpy(&bits,&out[0].headingRadians,4);assert(out[0].x==0.5f && out[0].y==0.5f && out[0].facet==2 && bits==1072776762u && out[0].unitId==66);}
{uint32_t bits;std::memcpy(&bits,&out[1].headingRadians,4);assert(out[1].x==0.5f && out[1].y==1.5f && out[1].facet==4 && bits==1078530011u && out[1].unitId==66);}
{uint32_t bits;std::memcpy(&bits,&out[2].headingRadians,4);assert(out[2].x==0.5f && out[2].y==2.5f && out[2].facet==1 && bits==1067506044u && out[2].unitId==66);}
{uint32_t bits;std::memcpy(&bits,&out[3].headingRadians,4);assert(out[3].x==1.5f && out[3].y==1.5f && out[3].facet==1 && bits==1067506044u && out[3].unitId==66);}
}
{ uint32_t rng=1u; swgb::RmsResourceGroups cfg{2,1,0,0,0};
assert(swgb::generateRmsGroupedResourcePlacements(1,1,{0},data,348,cfg,rng,out,&error,{}));
assert(rng==3357800067u && out.size()==1);
{uint32_t bits;std::memcpy(&bits,&out[0].headingRadians,4);assert(out[0].x==0.5f && out[0].y==0.5f && out[0].facet==6 && bits==0u && out[0].unitId==348);}
}
{ uint32_t rng=12345u; swgb::RmsResourceGroups cfg{2,0,5,0,2};
assert(swgb::generateRmsGroupedResourcePlacements(4,4,{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},data,102,cfg,rng,out,&error,{}));
assert(rng==1255958651u && out.size()==0);
}
{uint32_t rng=1;swgb::RmsResourceGroups cfg{2,2,5,0,2};
 assert(!swgb::generateRmsGroupedResourcePlacements(7,7,std::vector<uint8_t>(49,0),data,348,cfg,rng,out,&error,{},100));
 assert(out.empty() && rng==1 && !error.empty());
 cfg.variance=1025;
 assert(!swgb::generateRmsGroupedResourcePlacements(7,7,std::vector<uint8_t>(49,0),data,348,cfg,rng,out,&error));
 assert(out.empty() && rng==1);
 cfg.variance=0;
 assert(!swgb::generateRmsGroupedResourcePlacements(7,7,std::vector<uint8_t>(49,0),data,348,cfg,rng,out,&error,{49}));
 assert(out.empty() && rng==1);
 cfg.mode=0;
 assert(!swgb::generateRmsGroupedResourcePlacements(7,7,std::vector<uint8_t>(49,0),data,348,cfg,rng,out,&error));
 assert(out.empty() && rng==1);
}
std::cout<<"Original grouped resource fixtures passed\n";}
