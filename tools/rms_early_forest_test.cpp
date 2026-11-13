// SPDX-License-Identifier: GPL-3.0-or-later
#include "engine/rms_early_forest.h"
#include "core/genie_dat.h"
#include <cassert>
#include <cmath>
#include <cstring>
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
    u.terrainRestriction=4; u.obstructionType=2; u.obstructionClass=1; u.enableAutoGather=1; u.createDoppelgangerOnDeath=2;
    for (auto &storage:u.resourceStorages) storage={-1,0,0};
    u.resourceStorages[0]={1,100,0};
    data.graphics.resize(5293);
    auto &g=data.graphics[5292]; g.exists=true; g.sequenceType=6; g.angleCount=12;
    data.terrainRestrictions.resize(5);
    auto &restriction=data.terrainRestrictions[4].passableBuildableDmgMultiplier;
    restriction.assign(55,1);
    std::string error;
    std::vector<RmsSceneryPlacement> out;
    for (uint32_t seed:{1u,12345u}) for (bool changed:{false,true}) {
        uint32_t rng=seed;
        std::vector<RmsSceneryPlacement> early;
        assert(generateRmsEarlyForest(3,3,{4},data,rng,early,&error));
        assert(early.size()==1 && early[0].x==1.5f && early[0].y==1.5f);
        std::vector<uint8_t> terrain(9,10);
        if (changed) terrain[4]=9;
        assert(finishRmsEarlyForest(3,3,terrain,data,early,rng,out,&error));
        assert(out.size()==(changed?9u:10u) && out[0].alive==changed);
        assert(out[0].facet==early[0].facet);
        assert(rng==(seed==1 ? (changed?1766988168u:3750785579u) : (changed?1905301472u:3681310499u)));
        for (size_t i=1;i<out.size();++i) assert(out[i].alive);
    }
    {
        uint32_t rng=1;
        std::vector<RmsSceneryPlacement> early;
        assert(generateRmsEarlyForest(5,5,{6,8,16,18},data,rng,early,&error));
        std::vector<uint8_t> terrain(25,10); terrain[6]=9;
        assert(finishRmsEarlyForest(5,5,terrain,data,early,rng,out,&error));
        const int facets[]={0,6,2,9,7,5,4,10,9,8,2,10,8,6,3,0,1,4,1,1,11,5,1,0,0,4,6,6};
        assert(out.size()==28 && rng==1226606285u);
        for (size_t i=0;i<out.size();++i) {
            assert(out[i].facet==facets[i]);
            assert(out[i].alive==(i==0 || i>=4));
        }
        assert(out[0].x==1.5f && out[0].y==1.5f);
        assert(out[4].x==.5f && out[4].y==.5f);
    }
    uint32_t rng=1;
    assert(!generateRmsEarlyForest(3,3,{4,4},data,rng,out,&error));
    assert(out.empty() && rng==1);
    assert(!generateRmsEarlyForest(3,3,{9},data,rng,out,&error));
    assert(out.empty() && rng==1);
    std::vector<RmsSceneryPlacement> early;
    assert(generateRmsEarlyForest(3,3,{4},data,rng,early,&error));
    const uint32_t before=rng;
    auto invalid=early; invalid[0].alive=false;
    assert(!finishRmsEarlyForest(3,3,std::vector<uint8_t>(9,10),data,invalid,rng,out,&error));
    assert(out.empty() && rng==before);
    struct ForestSpec { int terrain,unit,alias,graphic,angles,carbon; };
    for (const auto spec: {ForestSpec{13,351,11,5295,12,100}, {17,414,11,5286,12,100},
         {18,348,10,5282,12,100}, {19,350,11,5298,12,100}, {20,349,10,5289,11,100},
         {21,413,10,5301,12,125}}) {
        auto d=data;
        d.civs[0].units.resize(415);
        d.civs[0].units[spec.unit]=data.civs[0].units[411];
        auto &prototype=d.civs[0].units[spec.unit];
        prototype.id=spec.unit; prototype.standingGraphic[0]=spec.graphic;
        prototype.terrainRestriction=11; prototype.resourceStorages[0].amount=spec.carbon;
        d.graphics.resize(5302); d.graphics[spec.graphic]=data.graphics[5292];
        d.graphics[spec.graphic].angleCount=spec.angles;
        d.terrainRestrictions.resize(12);
        d.terrainRestrictions[11].passableBuildableDmgMultiplier.assign(55,1);
        d.terrainBlock.terrains[spec.terrain]=data.terrainBlock.terrains[10];
        auto &t=d.terrainBlock.terrains[spec.terrain];
        t.terrainToDraw=spec.alias; t.terrainUnitId[0]=spec.unit;
        uint32_t state=1; std::vector<RmsSceneryPlacement> first;
        assert(generateRmsEarlyForest(5,5,{6,8,16,18},d,state,first,&error,spec.terrain));
        std::vector<uint8_t> tiles(25,spec.terrain); tiles[6]=9;
        assert(finishRmsEarlyForest(5,5,tiles,d,first,state,out,&error,spec.terrain));
        const int twelve[]={0,6,2,9,7,5,4,10,9,8,2,10,8,6,3,0,1,4,1,1,11,5,1,0,0,4,6,6};
        const int eleven[]={0,6,2,8,6,5,3,9,9,8,1,9,7,5,3,0,1,4,1,1,10,4,1,0,0,4,5,6};
        assert(state==1226606285u && out.size()==28);
        for (size_t i=0;i<out.size();++i) {
            assert(out[i].unitId==spec.unit && out[i].alive==(i==0 || i>=4));
            assert(out[i].facet==(spec.angles==11?eleven[i]:twelve[i]));
        }
        assert(!generateRmsEarlyForest(5,5,{6},d,state,out,&error,49));
        assert(state==1226606285u && out.empty());
    }
    data.terrainBlock.terrains[9].numTerrainUnitsUsed=1;
    data.terrainBlock.terrains[9].terrainUnitId[0]=411;
    assert(!finishRmsEarlyForest(3,3,std::vector<uint8_t>(9,9),data,early,rng,out,&error));
    assert(out.empty() && rng==before);
    std::cout << "RMS early forest lifecycle fixtures passed\n";
}
