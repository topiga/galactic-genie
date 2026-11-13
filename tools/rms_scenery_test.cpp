// SPDX-License-Identifier: GPL-3.0-or-later
#include "engine/rms_scenery.h"
#include "core/genie_dat.h"
#include <cassert>
#include <cmath>
#include <iostream>

int main() {
    using namespace swgb;
    dat::DatFile data;
    data.terrainBlock.terrains.resize(55);
    for (auto &terrain:data.terrainBlock.terrains) terrain.enabled=1;
    auto &forest=data.terrainBlock.terrains[10];
    forest.numTerrainUnitsUsed=1; forest.terrainUnitId[0]=411;
    forest.terrainUnitDensity[0]=1000; forest.terrainUnitCentering[0]=1;
    auto &dirt=data.terrainBlock.terrains[6];
    dirt.numTerrainUnitsUsed=1; dirt.terrainUnitId[0]=-1;
    data.civs.resize(1); data.civs[0].units.resize(412);
    auto &u=data.civs[0].units[411];
    u.exists=true; u.id=411; u.type=10; u.cls=31; u.hitPoints=50;
    u.standingGraphic[0]=5292;
    u.collisionSize[0]=u.collisionSize[1]=u.clearanceSize[0]=u.clearanceSize[1]=.5f;
    u.collisionSize[2]=2;
    u.placementSideTerrain[0]=u.placementSideTerrain[1]=u.placementTerrain[0]=u.placementTerrain[1]=-1;
    u.terrainRestriction=4; u.obstructionType=2; u.obstructionClass=1; u.enableAutoGather=1;
    for (auto &storage:u.resourceStorages) storage={-1,0,0};
    u.resourceStorages[0]={1,100,0};
    data.graphics.resize(5293);
    auto &g=data.graphics[5292]; g.exists=true; g.sequenceType=6; g.angleCount=12;
    data.terrainRestrictions.resize(5);
    auto &restriction=data.terrainRestrictions[4].passableBuildableDmgMultiplier;
    restriction.assign(55,1);
    std::string error;
    std::vector<RmsSceneryPlacement> out;
    {
        uint32_t rng=1u;
        const std::vector<uint8_t> terrain={10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10};
        assert(generateRmsScenery(5,6,terrain,data,rng,out,&error));
        assert(error.empty() && rng==1303876527u && out.size()==30);
        assert(out[0].unitId==411 && out[0].x==0.5f && out[0].y==0.5f && out[0].facet==0);
        assert(out[1].unitId==411 && out[1].x==1.5f && out[1].y==0.5f && out[1].facet==6);
        assert(out[2].unitId==411 && out[2].x==2.5f && out[2].y==0.5f && out[2].facet==2);
        assert(out[3].unitId==411 && out[3].x==3.5f && out[3].y==0.5f && out[3].facet==9);
        assert(out[4].unitId==411 && out[4].x==4.5f && out[4].y==0.5f && out[4].facet==7);
        assert(out[5].unitId==411 && out[5].x==0.5f && out[5].y==1.5f && out[5].facet==5);
        assert(out[6].unitId==411 && out[6].x==1.5f && out[6].y==1.5f && out[6].facet==4);
        assert(out[7].unitId==411 && out[7].x==2.5f && out[7].y==1.5f && out[7].facet==10);
        assert(out[8].unitId==411 && out[8].x==3.5f && out[8].y==1.5f && out[8].facet==9);
        assert(out[9].unitId==411 && out[9].x==4.5f && out[9].y==1.5f && out[9].facet==8);
        assert(out[10].unitId==411 && out[10].x==0.5f && out[10].y==2.5f && out[10].facet==2);
        assert(out[11].unitId==411 && out[11].x==1.5f && out[11].y==2.5f && out[11].facet==10);
        assert(out[12].unitId==411 && out[12].x==2.5f && out[12].y==2.5f && out[12].facet==8);
        assert(out[13].unitId==411 && out[13].x==3.5f && out[13].y==2.5f && out[13].facet==6);
        assert(out[14].unitId==411 && out[14].x==4.5f && out[14].y==2.5f && out[14].facet==3);
        assert(out[15].unitId==411 && out[15].x==0.5f && out[15].y==3.5f && out[15].facet==0);
        assert(out[16].unitId==411 && out[16].x==1.5f && out[16].y==3.5f && out[16].facet==1);
        assert(out[17].unitId==411 && out[17].x==2.5f && out[17].y==3.5f && out[17].facet==4);
        assert(out[18].unitId==411 && out[18].x==3.5f && out[18].y==3.5f && out[18].facet==1);
        assert(out[19].unitId==411 && out[19].x==4.5f && out[19].y==3.5f && out[19].facet==1);
        assert(out[20].unitId==411 && out[20].x==0.5f && out[20].y==4.5f && out[20].facet==11);
        assert(out[21].unitId==411 && out[21].x==1.5f && out[21].y==4.5f && out[21].facet==5);
        assert(out[22].unitId==411 && out[22].x==2.5f && out[22].y==4.5f && out[22].facet==1);
        assert(out[23].unitId==411 && out[23].x==3.5f && out[23].y==4.5f && out[23].facet==0);
        assert(out[24].unitId==411 && out[24].x==4.5f && out[24].y==4.5f && out[24].facet==0);
        assert(out[25].unitId==411 && out[25].x==0.5f && out[25].y==5.5f && out[25].facet==4);
        assert(out[26].unitId==411 && out[26].x==1.5f && out[26].y==5.5f && out[26].facet==6);
        assert(out[27].unitId==411 && out[27].x==2.5f && out[27].y==5.5f && out[27].facet==6);
        assert(out[28].unitId==411 && out[28].x==3.5f && out[28].y==5.5f && out[28].facet==7);
        assert(out[29].unitId==411 && out[29].x==4.5f && out[29].y==5.5f && out[29].facet==7);
    }
    {
        uint32_t rng=1u;
        const std::vector<uint8_t> terrain={0,6,9,10,41,44,0,6,9,10,41,44,0,6,9,10,41,44,0,6,9,10,41,44,0,6,9,10,41,44};
        assert(generateRmsScenery(5,6,terrain,data,rng,out,&error));
        assert(error.empty() && rng==3403800452u && out.size()==5);
        assert(out[0].unitId==411 && out[0].x==3.5f && out[0].y==0.5f && out[0].facet==0);
        assert(out[1].unitId==411 && out[1].x==4.5f && out[1].y==1.5f && out[1].facet==6);
        assert(out[2].unitId==411 && out[2].x==0.5f && out[2].y==3.5f && out[2].facet==2);
        assert(out[3].unitId==411 && out[3].x==1.5f && out[3].y==4.5f && out[3].facet==9);
        assert(out[4].unitId==411 && out[4].x==2.5f && out[4].y==5.5f && out[4].facet==7);
    }
    {
        uint32_t rng=1u;
        const std::vector<uint8_t> terrain={6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6};
        assert(generateRmsScenery(5,6,terrain,data,rng,out,&error));
        assert(error.empty() && rng==1u && out.size()==0);
    }
    {
        uint32_t rng=12345u;
        const std::vector<uint8_t> terrain={10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10};
        assert(generateRmsScenery(5,6,terrain,data,rng,out,&error));
        assert(error.empty() && rng==3960859687u && out.size()==30);
        assert(out[0].unitId==411 && out[0].x==0.5f && out[0].y==0.5f && out[0].facet==2);
        assert(out[1].unitId==411 && out[1].x==1.5f && out[1].y==0.5f && out[1].facet==7);
        assert(out[2].unitId==411 && out[2].x==2.5f && out[2].y==0.5f && out[2].facet==9);
        assert(out[3].unitId==411 && out[3].x==3.5f && out[3].y==0.5f && out[3].facet==8);
        assert(out[4].unitId==411 && out[4].x==4.5f && out[4].y==0.5f && out[4].facet==2);
        assert(out[5].unitId==411 && out[5].x==0.5f && out[5].y==1.5f && out[5].facet==8);
        assert(out[6].unitId==411 && out[6].x==1.5f && out[6].y==1.5f && out[6].facet==10);
        assert(out[7].unitId==411 && out[7].x==2.5f && out[7].y==1.5f && out[7].facet==1);
        assert(out[8].unitId==411 && out[8].x==3.5f && out[8].y==1.5f && out[8].facet==10);
        assert(out[9].unitId==411 && out[9].x==4.5f && out[9].y==1.5f && out[9].facet==8);
        assert(out[10].unitId==411 && out[10].x==0.5f && out[10].y==2.5f && out[10].facet==5);
        assert(out[11].unitId==411 && out[11].x==1.5f && out[11].y==2.5f && out[11].facet==5);
        assert(out[12].unitId==411 && out[12].x==2.5f && out[12].y==2.5f && out[12].facet==7);
        assert(out[13].unitId==411 && out[13].x==3.5f && out[13].y==2.5f && out[13].facet==7);
        assert(out[14].unitId==411 && out[14].x==4.5f && out[14].y==2.5f && out[14].facet==3);
        assert(out[15].unitId==411 && out[15].x==0.5f && out[15].y==3.5f && out[15].facet==5);
        assert(out[16].unitId==411 && out[16].x==1.5f && out[16].y==3.5f && out[16].facet==8);
        assert(out[17].unitId==411 && out[17].x==2.5f && out[17].y==3.5f && out[17].facet==3);
        assert(out[18].unitId==411 && out[18].x==3.5f && out[18].y==3.5f && out[18].facet==9);
        assert(out[19].unitId==411 && out[19].x==4.5f && out[19].y==3.5f && out[19].facet==1);
        assert(out[20].unitId==411 && out[20].x==0.5f && out[20].y==4.5f && out[20].facet==3);
        assert(out[21].unitId==411 && out[21].x==1.5f && out[21].y==4.5f && out[21].facet==3);
        assert(out[22].unitId==411 && out[22].x==2.5f && out[22].y==4.5f && out[22].facet==4);
        assert(out[23].unitId==411 && out[23].x==3.5f && out[23].y==4.5f && out[23].facet==5);
        assert(out[24].unitId==411 && out[24].x==4.5f && out[24].y==4.5f && out[24].facet==0);
        assert(out[25].unitId==411 && out[25].x==0.5f && out[25].y==5.5f && out[25].facet==3);
        assert(out[26].unitId==411 && out[26].x==1.5f && out[26].y==5.5f && out[26].facet==7);
        assert(out[27].unitId==411 && out[27].x==2.5f && out[27].y==5.5f && out[27].facet==1);
        assert(out[28].unitId==411 && out[28].x==3.5f && out[28].y==5.5f && out[28].facet==0);
        assert(out[29].unitId==411 && out[29].x==4.5f && out[29].y==5.5f && out[29].facet==10);
    }
    {
        uint32_t rng=12345u;
        const std::vector<uint8_t> terrain={0,6,9,10,41,44,0,6,9,10,41,44,0,6,9,10,41,44,0,6,9,10,41,44,0,6,9,10,41,44};
        assert(generateRmsScenery(5,6,terrain,data,rng,out,&error));
        assert(error.empty() && rng==2529458780u && out.size()==5);
        assert(out[0].unitId==411 && out[0].x==3.5f && out[0].y==0.5f && out[0].facet==2);
        assert(out[1].unitId==411 && out[1].x==4.5f && out[1].y==1.5f && out[1].facet==7);
        assert(out[2].unitId==411 && out[2].x==0.5f && out[2].y==3.5f && out[2].facet==9);
        assert(out[3].unitId==411 && out[3].x==1.5f && out[3].y==4.5f && out[3].facet==8);
        assert(out[4].unitId==411 && out[4].x==2.5f && out[4].y==5.5f && out[4].facet==2);
    }
    {
        uint32_t rng=12345u;
        const std::vector<uint8_t> terrain={6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6};
        assert(generateRmsScenery(5,6,terrain,data,rng,out,&error));
        assert(error.empty() && rng==12345u && out.size()==0);
    }
    {
        uint32_t rng=4294967295u;
        const std::vector<uint8_t> terrain={10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10};
        assert(generateRmsScenery(5,6,terrain,data,rng,out,&error));
        assert(error.empty() && rng==2926932541u && out.size()==30);
        assert(out[0].unitId==411 && out[0].x==0.5f && out[0].y==0.5f && out[0].facet==0);
        assert(out[1].unitId==411 && out[1].x==1.5f && out[1].y==0.5f && out[1].facet==10);
        assert(out[2].unitId==411 && out[2].x==2.5f && out[2].y==0.5f && out[2].facet==1);
        assert(out[3].unitId==411 && out[3].x==3.5f && out[3].y==0.5f && out[3].facet==4);
        assert(out[4].unitId==411 && out[4].x==4.5f && out[4].y==0.5f && out[4].facet==11);
        assert(out[5].unitId==411 && out[5].x==0.5f && out[5].y==1.5f && out[5].facet==2);
        assert(out[6].unitId==411 && out[6].x==1.5f && out[6].y==1.5f && out[6].facet==1);
        assert(out[7].unitId==411 && out[7].x==2.5f && out[7].y==1.5f && out[7].facet==0);
        assert(out[8].unitId==411 && out[8].x==3.5f && out[8].y==1.5f && out[8].facet==9);
        assert(out[9].unitId==411 && out[9].x==4.5f && out[9].y==1.5f && out[9].facet==1);
        assert(out[10].unitId==411 && out[10].x==0.5f && out[10].y==2.5f && out[10].facet==2);
        assert(out[11].unitId==411 && out[11].x==1.5f && out[11].y==2.5f && out[11].facet==10);
        assert(out[12].unitId==411 && out[12].x==2.5f && out[12].y==2.5f && out[12].facet==4);
        assert(out[13].unitId==411 && out[13].x==3.5f && out[13].y==2.5f && out[13].facet==6);
        assert(out[14].unitId==411 && out[14].x==4.5f && out[14].y==2.5f && out[14].facet==11);
        assert(out[15].unitId==411 && out[15].x==0.5f && out[15].y==3.5f && out[15].facet==11);
        assert(out[16].unitId==411 && out[16].x==1.5f && out[16].y==3.5f && out[16].facet==5);
        assert(out[17].unitId==411 && out[17].x==2.5f && out[17].y==3.5f && out[17].facet==2);
        assert(out[18].unitId==411 && out[18].x==3.5f && out[18].y==3.5f && out[18].facet==0);
        assert(out[19].unitId==411 && out[19].x==4.5f && out[19].y==3.5f && out[19].facet==8);
        assert(out[20].unitId==411 && out[20].x==0.5f && out[20].y==4.5f && out[20].facet==3);
        assert(out[21].unitId==411 && out[21].x==1.5f && out[21].y==4.5f && out[21].facet==6);
        assert(out[22].unitId==411 && out[22].x==2.5f && out[22].y==4.5f && out[22].facet==2);
        assert(out[23].unitId==411 && out[23].x==3.5f && out[23].y==4.5f && out[23].facet==6);
        assert(out[24].unitId==411 && out[24].x==4.5f && out[24].y==4.5f && out[24].facet==1);
        assert(out[25].unitId==411 && out[25].x==0.5f && out[25].y==5.5f && out[25].facet==8);
        assert(out[26].unitId==411 && out[26].x==1.5f && out[26].y==5.5f && out[26].facet==2);
        assert(out[27].unitId==411 && out[27].x==2.5f && out[27].y==5.5f && out[27].facet==6);
        assert(out[28].unitId==411 && out[28].x==3.5f && out[28].y==5.5f && out[28].facet==11);
        assert(out[29].unitId==411 && out[29].x==4.5f && out[29].y==5.5f && out[29].facet==4);
    }
    {
        uint32_t rng=4294967295u;
        const std::vector<uint8_t> terrain={0,6,9,10,41,44,0,6,9,10,41,44,0,6,9,10,41,44,0,6,9,10,41,44,0,6,9,10,41,44};
        assert(generateRmsScenery(5,6,terrain,data,rng,out,&error));
        assert(error.empty() && rng==2051848554u && out.size()==5);
        assert(out[0].unitId==411 && out[0].x==3.5f && out[0].y==0.5f && out[0].facet==0);
        assert(out[1].unitId==411 && out[1].x==4.5f && out[1].y==1.5f && out[1].facet==10);
        assert(out[2].unitId==411 && out[2].x==0.5f && out[2].y==3.5f && out[2].facet==1);
        assert(out[3].unitId==411 && out[3].x==1.5f && out[3].y==4.5f && out[3].facet==4);
        assert(out[4].unitId==411 && out[4].x==2.5f && out[4].y==5.5f && out[4].facet==11);
    }
    {
        uint32_t rng=4294967295u;
        const std::vector<uint8_t> terrain={6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6};
        assert(generateRmsScenery(5,6,terrain,data,rng,out,&error));
        assert(error.empty() && rng==4294967295u && out.size()==0);
    }
    uint32_t rng=948628385u;
    assert(generateRmsScenery(1,1,{10},data,rng,out,&error));
    assert(out.size()==1 && out[0].facet==11);
    rng=1;
    restriction[10]=.05f;
    assert(generateRmsScenery(1,1,{10},data,rng,out,&error) && out.empty() && rng==1);
    restriction[10]=std::nextafter(.05f,1.f);
    assert(generateRmsScenery(1,1,{10},data,rng,out,&error) && out.size()==1 && rng==2745024u);
    restriction[10]=1;
    auto fail=[&](const std::vector<uint8_t> &terrain) {
        rng=1; out={{411,.5f,.5f,0}};
        assert(!generateRmsScenery(static_cast<uint32_t>(terrain.size()),1,terrain,data,rng,out,&error));
        assert(rng==1 && out.empty() && !error.empty());
    };
    forest.terrainUnitCentering[0]=0; fail({10}); forest.terrainUnitCentering[0]=1;
    forest.terrainUnitDensity[0]=999; fail({10}); forest.terrainUnitDensity[0]=1000;
    forest.terrainUnitId[0]=410; data.civs[0].units[410].exists=true; fail({10}); forest.terrainUnitId[0]=411;
    forest.numTerrainUnitsUsed=31; fail({10}); forest.numTerrainUnitsUsed=1;
    dirt.terrainUnitId[0]=-2; fail({10,6}); dirt.terrainUnitId[0]=-1;
    dirt.terrainUnitId[0]=32767; fail({6}); dirt.terrainUnitId[0]=-1;
    u.clearanceSize[0]=1; fail({10}); u.clearanceSize[0]=.5f;
    g.sequenceType=0; fail({10}); g.sequenceType=6;
    restriction[10]=NAN; fail({10}); restriction[10]=1;
    forest.terrainToDraw=0; fail({10}); forest.terrainToDraw=-1;
    auto &shore=data.terrainBlock.terrains[37];
    auto &shoreTarget=data.terrainBlock.terrains[35];
    shore.terrainToDraw=35; rng=1;
    const std::vector<uint8_t> shoreTiles={37,35,37};
    assert(generateRmsScenery(3,1,shoreTiles,data,rng,out,&error) && rng==1 && out.empty());
    assert((shoreTiles==std::vector<uint8_t>{37,35,37}));
    shore.terrainToDraw=34; fail({37}); shore.terrainToDraw=35;
    shoreTarget.enabled=0; fail({37}); shoreTarget.enabled=1;
    shoreTarget.terrainToDraw=0; fail({37}); shoreTarget.terrainToDraw=-1;
    shoreTarget.numTerrainUnitsUsed=1; shoreTarget.terrainUnitId[0]=411;
    rng=1; assert(generateRmsScenery(1,1,{37},data,rng,out,&error) && rng==1 && out.empty());
    shoreTarget.numTerrainUnitsUsed=0;
    fail({55}); fail({255});
    rng=1;
    assert(!generateRmsScenery(0,1,{},data,rng,out,&error) && rng==1 && out.empty());
    assert(!generateRmsScenery(2,2,{10},data,rng,out,&error) && rng==1 && out.empty());
    u.exists=false; rng=1;
    assert(generateRmsScenery(1,1,{10},data,rng,out,&error) && rng==1 && out.empty());
    u.exists=true;
    // A valid ID with nonpositive density is inactive before centering is read.
    dirt.terrainUnitId[0]=0; dirt.terrainUnitDensity[0]=-1; dirt.terrainUnitCentering[0]=99;
    assert(generateRmsScenery(1,1,{6},data,rng,out,&error) && rng==1 && out.empty());
    // Additional source-alias trees retain their own slots, including11-facet TIMBERC.
    const auto prototype=data.civs[0].units[411];
    const auto graphic=data.graphics[5292];
    data.civs[0].units.resize(415);
    data.graphics.resize(5302);
    data.terrainRestrictions.resize(12);
    data.terrainRestrictions[11].passableBuildableDmgMultiplier.assign(55,1);
    const int specs[][6]={{13,11,351,5295,11,12},{17,11,414,5286,11,12},
        {18,10,348,5282,11,12},{19,11,350,5298,11,12},{20,10,349,5289,11,11},{21,10,413,5301,11,12}};
    for (const auto &spec:specs) {
        auto &t=data.terrainBlock.terrains[spec[0]];
        t.terrainToDraw=spec[1]; t.numTerrainUnitsUsed=1; t.terrainUnitId[0]=spec[2];
        t.terrainUnitDensity[0]=1000; t.terrainUnitCentering[0]=1;
        auto &unit=data.civs[0].units[spec[2]]; unit=prototype;
        if (spec[2]==413) unit.resourceStorages[0].amount=125;
        unit.id=spec[2]; unit.standingGraphic[0]=spec[3]; unit.terrainRestriction=spec[4];
        data.graphics[spec[3]]=graphic; data.graphics[spec[3]].angleCount=spec[5];
    }
    {
        rng=1u;
        const std::vector<uint8_t> tiles={9,10,10,9,10,10,9,10,10,9,10,10,9,10,10,9,10,10,9,10,10,9,10,10,9,10,10,9,10,10};
        assert(generateRmsScenery(5,6,tiles,data,rng,out,&error));
        assert(rng==2503793637u && out.size()==20);
        const uint8_t facets[]={0,6,2,9,7,5,4,10,9,8,2,10,8,6,3,0,1,4,1,1};
        for (size_t i=0;i<20;++i) {
            const size_t tile=(i/2)*3+(i%2)+1;
            assert(out[i].unitId==411 && out[i].x==float(tile%5)+.5f &&
                out[i].y==float(tile/5)+.5f && out[i].facet==facets[i]);
        }
    }
    {
        rng=12345u;
        const std::vector<uint8_t> tiles={9,10,10,9,10,10,9,10,10,9,10,10,9,10,10,9,10,10,9,10,10,9,10,10,9,10,10,9,10,10};
        assert(generateRmsScenery(5,6,tiles,data,rng,out,&error));
        assert(rng==2406323101u && out.size()==20);
        const uint8_t facets[]={2,7,9,8,2,8,10,1,10,8,5,5,7,7,3,5,8,3,9,1};
        for (size_t i=0;i<20;++i) {
            const size_t tile=(i/2)*3+(i%2)+1;
            assert(out[i].unitId==411 && out[i].x==float(tile%5)+.5f &&
                out[i].y==float(tile/5)+.5f && out[i].facet==facets[i]);
        }
    }
    {
        rng=4294967295u;
        const std::vector<uint8_t> tiles={9,10,10,9,10,10,9,10,10,9,10,10,9,10,10,9,10,10,9,10,10,9,10,10,9,10,10,9,10,10};
        assert(generateRmsScenery(5,6,tiles,data,rng,out,&error));
        assert(rng==3715335363u && out.size()==20);
        const uint8_t facets[]={0,10,1,4,11,2,1,0,9,1,2,10,4,6,11,11,5,2,0,8};
        for (size_t i=0;i<20;++i) {
            const size_t tile=(i/2)*3+(i%2)+1;
            assert(out[i].unitId==411 && out[i].x==float(tile%5)+.5f &&
                out[i].y==float(tile/5)+.5f && out[i].facet==facets[i]);
        }
    }
    {
        rng=948628385u;
        const std::vector<uint8_t> tiles={9,10,10,9,10,10,9,10,10,9,10,10,9,10,10,9,10,10,9,10,10,9,10,10,9,10,10,9,10,10};
        assert(generateRmsScenery(5,6,tiles,data,rng,out,&error));
        assert(rng==567292293u && out.size()==20);
        const uint8_t facets[]={11,6,5,0,1,3,5,10,3,6,11,3,9,3,1,9,11,7,10,3};
        for (size_t i=0;i<20;++i) {
            const size_t tile=(i/2)*3+(i%2)+1;
            assert(out[i].unitId==411 && out[i].x==float(tile%5)+.5f &&
                out[i].y==float(tile/5)+.5f && out[i].facet==facets[i]);
        }
    }
    {
        rng=1u;
        const std::vector<uint8_t> tiles={9,13,13,9,13,13,9,13,13,9,13,13,9,13,13,9,13,13,9,13,13,9,13,13,9,13,13,9,13,13};
        assert(generateRmsScenery(5,6,tiles,data,rng,out,&error));
        assert(rng==2503793637u && out.size()==20);
        const uint8_t facets[]={0,6,2,9,7,5,4,10,9,8,2,10,8,6,3,0,1,4,1,1};
        for (size_t i=0;i<20;++i) {
            const size_t tile=(i/2)*3+(i%2)+1;
            assert(out[i].unitId==351 && out[i].x==float(tile%5)+.5f &&
                out[i].y==float(tile/5)+.5f && out[i].facet==facets[i]);
        }
    }
    {
        rng=12345u;
        const std::vector<uint8_t> tiles={9,13,13,9,13,13,9,13,13,9,13,13,9,13,13,9,13,13,9,13,13,9,13,13,9,13,13,9,13,13};
        assert(generateRmsScenery(5,6,tiles,data,rng,out,&error));
        assert(rng==2406323101u && out.size()==20);
        const uint8_t facets[]={2,7,9,8,2,8,10,1,10,8,5,5,7,7,3,5,8,3,9,1};
        for (size_t i=0;i<20;++i) {
            const size_t tile=(i/2)*3+(i%2)+1;
            assert(out[i].unitId==351 && out[i].x==float(tile%5)+.5f &&
                out[i].y==float(tile/5)+.5f && out[i].facet==facets[i]);
        }
    }
    {
        rng=4294967295u;
        const std::vector<uint8_t> tiles={9,13,13,9,13,13,9,13,13,9,13,13,9,13,13,9,13,13,9,13,13,9,13,13,9,13,13,9,13,13};
        assert(generateRmsScenery(5,6,tiles,data,rng,out,&error));
        assert(rng==3715335363u && out.size()==20);
        const uint8_t facets[]={0,10,1,4,11,2,1,0,9,1,2,10,4,6,11,11,5,2,0,8};
        for (size_t i=0;i<20;++i) {
            const size_t tile=(i/2)*3+(i%2)+1;
            assert(out[i].unitId==351 && out[i].x==float(tile%5)+.5f &&
                out[i].y==float(tile/5)+.5f && out[i].facet==facets[i]);
        }
    }
    {
        rng=948628385u;
        const std::vector<uint8_t> tiles={9,13,13,9,13,13,9,13,13,9,13,13,9,13,13,9,13,13,9,13,13,9,13,13,9,13,13,9,13,13};
        assert(generateRmsScenery(5,6,tiles,data,rng,out,&error));
        assert(rng==567292293u && out.size()==20);
        const uint8_t facets[]={11,6,5,0,1,3,5,10,3,6,11,3,9,3,1,9,11,7,10,3};
        for (size_t i=0;i<20;++i) {
            const size_t tile=(i/2)*3+(i%2)+1;
            assert(out[i].unitId==351 && out[i].x==float(tile%5)+.5f &&
                out[i].y==float(tile/5)+.5f && out[i].facet==facets[i]);
        }
    }
    {
        rng=1u;
        const std::vector<uint8_t> tiles={9,17,17,9,17,17,9,17,17,9,17,17,9,17,17,9,17,17,9,17,17,9,17,17,9,17,17,9,17,17};
        assert(generateRmsScenery(5,6,tiles,data,rng,out,&error));
        assert(rng==2503793637u && out.size()==20);
        const uint8_t facets[]={0,6,2,9,7,5,4,10,9,8,2,10,8,6,3,0,1,4,1,1};
        for (size_t i=0;i<20;++i) {
            const size_t tile=(i/2)*3+(i%2)+1;
            assert(out[i].unitId==414 && out[i].x==float(tile%5)+.5f &&
                out[i].y==float(tile/5)+.5f && out[i].facet==facets[i]);
        }
    }
    {
        rng=12345u;
        const std::vector<uint8_t> tiles={9,17,17,9,17,17,9,17,17,9,17,17,9,17,17,9,17,17,9,17,17,9,17,17,9,17,17,9,17,17};
        assert(generateRmsScenery(5,6,tiles,data,rng,out,&error));
        assert(rng==2406323101u && out.size()==20);
        const uint8_t facets[]={2,7,9,8,2,8,10,1,10,8,5,5,7,7,3,5,8,3,9,1};
        for (size_t i=0;i<20;++i) {
            const size_t tile=(i/2)*3+(i%2)+1;
            assert(out[i].unitId==414 && out[i].x==float(tile%5)+.5f &&
                out[i].y==float(tile/5)+.5f && out[i].facet==facets[i]);
        }
    }
    {
        rng=4294967295u;
        const std::vector<uint8_t> tiles={9,17,17,9,17,17,9,17,17,9,17,17,9,17,17,9,17,17,9,17,17,9,17,17,9,17,17,9,17,17};
        assert(generateRmsScenery(5,6,tiles,data,rng,out,&error));
        assert(rng==3715335363u && out.size()==20);
        const uint8_t facets[]={0,10,1,4,11,2,1,0,9,1,2,10,4,6,11,11,5,2,0,8};
        for (size_t i=0;i<20;++i) {
            const size_t tile=(i/2)*3+(i%2)+1;
            assert(out[i].unitId==414 && out[i].x==float(tile%5)+.5f &&
                out[i].y==float(tile/5)+.5f && out[i].facet==facets[i]);
        }
    }
    {
        rng=948628385u;
        const std::vector<uint8_t> tiles={9,17,17,9,17,17,9,17,17,9,17,17,9,17,17,9,17,17,9,17,17,9,17,17,9,17,17,9,17,17};
        assert(generateRmsScenery(5,6,tiles,data,rng,out,&error));
        assert(rng==567292293u && out.size()==20);
        const uint8_t facets[]={11,6,5,0,1,3,5,10,3,6,11,3,9,3,1,9,11,7,10,3};
        for (size_t i=0;i<20;++i) {
            const size_t tile=(i/2)*3+(i%2)+1;
            assert(out[i].unitId==414 && out[i].x==float(tile%5)+.5f &&
                out[i].y==float(tile/5)+.5f && out[i].facet==facets[i]);
        }
    }
    {
        rng=1u;
        const std::vector<uint8_t> tiles={9,18,18,9,18,18,9,18,18,9,18,18,9,18,18,9,18,18,9,18,18,9,18,18,9,18,18,9,18,18};
        assert(generateRmsScenery(5,6,tiles,data,rng,out,&error));
        assert(rng==2503793637u && out.size()==20);
        const uint8_t facets[]={0,6,2,9,7,5,4,10,9,8,2,10,8,6,3,0,1,4,1,1};
        for (size_t i=0;i<20;++i) {
            const size_t tile=(i/2)*3+(i%2)+1;
            assert(out[i].unitId==348 && out[i].x==float(tile%5)+.5f &&
                out[i].y==float(tile/5)+.5f && out[i].facet==facets[i]);
        }
    }
    {
        rng=12345u;
        const std::vector<uint8_t> tiles={9,18,18,9,18,18,9,18,18,9,18,18,9,18,18,9,18,18,9,18,18,9,18,18,9,18,18,9,18,18};
        assert(generateRmsScenery(5,6,tiles,data,rng,out,&error));
        assert(rng==2406323101u && out.size()==20);
        const uint8_t facets[]={2,7,9,8,2,8,10,1,10,8,5,5,7,7,3,5,8,3,9,1};
        for (size_t i=0;i<20;++i) {
            const size_t tile=(i/2)*3+(i%2)+1;
            assert(out[i].unitId==348 && out[i].x==float(tile%5)+.5f &&
                out[i].y==float(tile/5)+.5f && out[i].facet==facets[i]);
        }
    }
    {
        rng=4294967295u;
        const std::vector<uint8_t> tiles={9,18,18,9,18,18,9,18,18,9,18,18,9,18,18,9,18,18,9,18,18,9,18,18,9,18,18,9,18,18};
        assert(generateRmsScenery(5,6,tiles,data,rng,out,&error));
        assert(rng==3715335363u && out.size()==20);
        const uint8_t facets[]={0,10,1,4,11,2,1,0,9,1,2,10,4,6,11,11,5,2,0,8};
        for (size_t i=0;i<20;++i) {
            const size_t tile=(i/2)*3+(i%2)+1;
            assert(out[i].unitId==348 && out[i].x==float(tile%5)+.5f &&
                out[i].y==float(tile/5)+.5f && out[i].facet==facets[i]);
        }
    }
    {
        rng=948628385u;
        const std::vector<uint8_t> tiles={9,18,18,9,18,18,9,18,18,9,18,18,9,18,18,9,18,18,9,18,18,9,18,18,9,18,18,9,18,18};
        assert(generateRmsScenery(5,6,tiles,data,rng,out,&error));
        assert(rng==567292293u && out.size()==20);
        const uint8_t facets[]={11,6,5,0,1,3,5,10,3,6,11,3,9,3,1,9,11,7,10,3};
        for (size_t i=0;i<20;++i) {
            const size_t tile=(i/2)*3+(i%2)+1;
            assert(out[i].unitId==348 && out[i].x==float(tile%5)+.5f &&
                out[i].y==float(tile/5)+.5f && out[i].facet==facets[i]);
        }
    }
    {
        rng=1u;
        const std::vector<uint8_t> tiles={9,19,19,9,19,19,9,19,19,9,19,19,9,19,19,9,19,19,9,19,19,9,19,19,9,19,19,9,19,19};
        assert(generateRmsScenery(5,6,tiles,data,rng,out,&error));
        assert(rng==2503793637u && out.size()==20);
        const uint8_t facets[]={0,6,2,9,7,5,4,10,9,8,2,10,8,6,3,0,1,4,1,1};
        for (size_t i=0;i<20;++i) {
            const size_t tile=(i/2)*3+(i%2)+1;
            assert(out[i].unitId==350 && out[i].x==float(tile%5)+.5f &&
                out[i].y==float(tile/5)+.5f && out[i].facet==facets[i]);
        }
    }
    {
        rng=12345u;
        const std::vector<uint8_t> tiles={9,19,19,9,19,19,9,19,19,9,19,19,9,19,19,9,19,19,9,19,19,9,19,19,9,19,19,9,19,19};
        assert(generateRmsScenery(5,6,tiles,data,rng,out,&error));
        assert(rng==2406323101u && out.size()==20);
        const uint8_t facets[]={2,7,9,8,2,8,10,1,10,8,5,5,7,7,3,5,8,3,9,1};
        for (size_t i=0;i<20;++i) {
            const size_t tile=(i/2)*3+(i%2)+1;
            assert(out[i].unitId==350 && out[i].x==float(tile%5)+.5f &&
                out[i].y==float(tile/5)+.5f && out[i].facet==facets[i]);
        }
    }
    {
        rng=4294967295u;
        const std::vector<uint8_t> tiles={9,19,19,9,19,19,9,19,19,9,19,19,9,19,19,9,19,19,9,19,19,9,19,19,9,19,19,9,19,19};
        assert(generateRmsScenery(5,6,tiles,data,rng,out,&error));
        assert(rng==3715335363u && out.size()==20);
        const uint8_t facets[]={0,10,1,4,11,2,1,0,9,1,2,10,4,6,11,11,5,2,0,8};
        for (size_t i=0;i<20;++i) {
            const size_t tile=(i/2)*3+(i%2)+1;
            assert(out[i].unitId==350 && out[i].x==float(tile%5)+.5f &&
                out[i].y==float(tile/5)+.5f && out[i].facet==facets[i]);
        }
    }
    {
        rng=948628385u;
        const std::vector<uint8_t> tiles={9,19,19,9,19,19,9,19,19,9,19,19,9,19,19,9,19,19,9,19,19,9,19,19,9,19,19,9,19,19};
        assert(generateRmsScenery(5,6,tiles,data,rng,out,&error));
        assert(rng==567292293u && out.size()==20);
        const uint8_t facets[]={11,6,5,0,1,3,5,10,3,6,11,3,9,3,1,9,11,7,10,3};
        for (size_t i=0;i<20;++i) {
            const size_t tile=(i/2)*3+(i%2)+1;
            assert(out[i].unitId==350 && out[i].x==float(tile%5)+.5f &&
                out[i].y==float(tile/5)+.5f && out[i].facet==facets[i]);
        }
    }
    {
        rng=1u;
        const std::vector<uint8_t> tiles={9,20,20,9,20,20,9,20,20,9,20,20,9,20,20,9,20,20,9,20,20,9,20,20,9,20,20,9,20,20};
        assert(generateRmsScenery(5,6,tiles,data,rng,out,&error));
        assert(rng==2503793637u && out.size()==20);
        const uint8_t facets[]={0,6,2,8,6,5,3,9,9,8,1,9,7,5,3,0,1,4,1,1};
        for (size_t i=0;i<20;++i) {
            const size_t tile=(i/2)*3+(i%2)+1;
            assert(out[i].unitId==349 && out[i].x==float(tile%5)+.5f &&
                out[i].y==float(tile/5)+.5f && out[i].facet==facets[i]);
        }
    }
    {
        rng=12345u;
        const std::vector<uint8_t> tiles={9,20,20,9,20,20,9,20,20,9,20,20,9,20,20,9,20,20,9,20,20,9,20,20,9,20,20,9,20,20};
        assert(generateRmsScenery(5,6,tiles,data,rng,out,&error));
        assert(rng==2406323101u && out.size()==20);
        const uint8_t facets[]={2,6,8,7,1,7,9,1,9,7,5,4,6,7,3,4,7,3,8,1};
        for (size_t i=0;i<20;++i) {
            const size_t tile=(i/2)*3+(i%2)+1;
            assert(out[i].unitId==349 && out[i].x==float(tile%5)+.5f &&
                out[i].y==float(tile/5)+.5f && out[i].facet==facets[i]);
        }
    }
    {
        rng=4294967295u;
        const std::vector<uint8_t> tiles={9,20,20,9,20,20,9,20,20,9,20,20,9,20,20,9,20,20,9,20,20,9,20,20,9,20,20,9,20,20};
        assert(generateRmsScenery(5,6,tiles,data,rng,out,&error));
        assert(rng==3715335363u && out.size()==20);
        const uint8_t facets[]={0,9,1,3,10,2,1,0,8,1,2,9,3,5,10,10,5,2,0,8};
        for (size_t i=0;i<20;++i) {
            const size_t tile=(i/2)*3+(i%2)+1;
            assert(out[i].unitId==349 && out[i].x==float(tile%5)+.5f &&
                out[i].y==float(tile/5)+.5f && out[i].facet==facets[i]);
        }
    }
    {
        rng=948628385u;
        const std::vector<uint8_t> tiles={9,20,20,9,20,20,9,20,20,9,20,20,9,20,20,9,20,20,9,20,20,9,20,20,9,20,20,9,20,20};
        assert(generateRmsScenery(5,6,tiles,data,rng,out,&error));
        assert(rng==567292293u && out.size()==20);
        const uint8_t facets[]={10,6,4,0,0,2,5,9,3,5,10,2,8,2,1,9,10,6,9,2};
        for (size_t i=0;i<20;++i) {
            const size_t tile=(i/2)*3+(i%2)+1;
            assert(out[i].unitId==349 && out[i].x==float(tile%5)+.5f &&
                out[i].y==float(tile/5)+.5f && out[i].facet==facets[i]);
        }
    }
    data.terrainBlock.terrains[5].terrainToDraw=10; rng=1;
    assert(generateRmsScenery(1,1,{5},data,rng,out,&error) && rng==1 && out.empty());
    data.graphics[5289].angleCount=12; fail({20}); data.graphics[5289].angleCount=11;
    data.terrainBlock.terrains[13].terrainToDraw=10; fail({13});
    data.terrainBlock.terrains[13].terrainToDraw=11;
    {
        rng=1u;
        const std::vector<uint8_t> tiles={9,21,21,9,21,21,9,21,21,9,21,21,9,21,21,9,21,21,9,21,21,9,21,21,9,21,21,9,21,21};
        assert(generateRmsScenery(5,6,tiles,data,rng,out,&error));
        assert(rng==2503793637u && out.size()==20);
        const uint8_t facets[]={0,6,2,9,7,5,4,10,9,8,2,10,8,6,3,0,1,4,1,1};
        for (size_t i=0;i<20;++i) {
            const size_t tile=(i/2)*3+(i%2)+1;
            assert(out[i].unitId==413 && out[i].x==float(tile%5)+.5f &&
                out[i].y==float(tile/5)+.5f && out[i].facet==facets[i]);
        }
    }
    {
        rng=12345u;
        const std::vector<uint8_t> tiles={9,21,21,9,21,21,9,21,21,9,21,21,9,21,21,9,21,21,9,21,21,9,21,21,9,21,21,9,21,21};
        assert(generateRmsScenery(5,6,tiles,data,rng,out,&error));
        assert(rng==2406323101u && out.size()==20);
        const uint8_t facets[]={2,7,9,8,2,8,10,1,10,8,5,5,7,7,3,5,8,3,9,1};
        for (size_t i=0;i<20;++i) {
            const size_t tile=(i/2)*3+(i%2)+1;
            assert(out[i].unitId==413 && out[i].x==float(tile%5)+.5f &&
                out[i].y==float(tile/5)+.5f && out[i].facet==facets[i]);
        }
    }
    {
        rng=4294967295u;
        const std::vector<uint8_t> tiles={9,21,21,9,21,21,9,21,21,9,21,21,9,21,21,9,21,21,9,21,21,9,21,21,9,21,21,9,21,21};
        assert(generateRmsScenery(5,6,tiles,data,rng,out,&error));
        assert(rng==3715335363u && out.size()==20);
        const uint8_t facets[]={0,10,1,4,11,2,1,0,9,1,2,10,4,6,11,11,5,2,0,8};
        for (size_t i=0;i<20;++i) {
            const size_t tile=(i/2)*3+(i%2)+1;
            assert(out[i].unitId==413 && out[i].x==float(tile%5)+.5f &&
                out[i].y==float(tile/5)+.5f && out[i].facet==facets[i]);
        }
    }
    {
        rng=948628385u;
        const std::vector<uint8_t> tiles={9,21,21,9,21,21,9,21,21,9,21,21,9,21,21,9,21,21,9,21,21,9,21,21,9,21,21,9,21,21};
        assert(generateRmsScenery(5,6,tiles,data,rng,out,&error));
        assert(rng==567292293u && out.size()==20);
        const uint8_t facets[]={11,6,5,0,1,3,5,10,3,6,11,3,9,3,1,9,11,7,10,3};
        for (size_t i=0;i<20;++i) {
            const size_t tile=(i/2)*3+(i%2)+1;
            assert(out[i].unitId==413 && out[i].x==float(tile%5)+.5f &&
                out[i].y==float(tile/5)+.5f && out[i].facet==facets[i]);
        }
    }
    data.civs[0].units[413].resourceStorages[0].amount=100; fail({21});
    data.civs[0].units[413].resourceStorages[0].amount=125;
    std::cout << "Original scenery traversal/facet fixtures and validation checks passed\n";
}
