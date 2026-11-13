// SPDX-License-Identifier: GPL-3.0-or-later
#include "engine/rms_objects.h"
#include "core/genie_dat.h"
#include <cmath>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <numeric>

static void check(bool value, const char *message) {
    if (!value) { std::cerr << message << '\n'; std::exit(1); }
}

int main() {
    using namespace swgb;
    std::vector<uint32_t> out;
    std::string err;
    // Independently derived from original 4dded0 / 4d6ab0 and CRT draws.
    const auto fixture = [&](uint32_t seed, uint32_t finalState,
                             std::vector<uint32_t> prefix) {
        auto expected = prefix;
        for (uint32_t i=0;i<30;++i)
            if (std::find(prefix.begin(),prefix.end(),i)==prefix.end()) expected.push_back(i);
        check(generateRmsObjectCandidates(5,6,seed,out,&err), "candidate generation failed");
        check(out==expected, "original move-to-front ordering mismatch");
        check(seed==finalState, "candidate RNG consumption mismatch");
    };
    fixture(1,1030492215,{6,15,10});
    fixture(2,1287834384,{12,15}); // Repeated node is moved, not duplicated.
    fixture(1967,159055541,{1,13,5}); // Draw32767 reaches the span endpoint.
    uint32_t rng=1;
    check(generateRmsObjectCandidates(3,7,rng,out,&err), "narrow map failed");
    std::vector<uint32_t> rowMajor(21);
    std::iota(rowMajor.begin(),rowMajor.end(),0);
    check(out==rowMajor && rng==1, "narrow map must not consume RNG");
    check(generateRmsObjectCandidates(1,1,rng,out,&err) && out==std::vector<uint32_t>{0},
          "singleton candidate lost");
    check(!generateRmsObjectCandidates(0,1,rng,out,&err) && out.empty() && rng==1 && !err.empty(),
          "invalid dimensions must clear output and preserve RNG");
    check(!generateRmsObjectCandidates(1,1025,rng,out,&err) && rng==1, "dimension limit");
    check(generateRmsObjectCandidates(1024,1024,rng,out,&err) && err.empty(), "maximum map failed");
    std::sort(out.begin(),out.end());
    check(out.size()==1024*1024, "maximum map size");
    for (uint32_t i=0;i<out.size();++i) check(out[i]==i,"candidate list is not a permutation");
    dat::DatFile data;
    data.civs.resize(1);
    data.civs[0].units.resize(349);
    auto &u=data.civs[0].units[348];
    u.exists=true; u.id=348; u.type=10; u.cls=31; u.hitPoints=50;
    u.standingGraphic[0]=5282;
    u.collisionSize[0]=u.collisionSize[1]=u.clearanceSize[0]=u.clearanceSize[1]=.5f;
    u.collisionSize[2]=2;
    u.placementSideTerrain[0]=u.placementSideTerrain[1]=u.placementTerrain[0]=u.placementTerrain[1]=-1;
    u.terrainRestriction=11; u.obstructionType=2; u.obstructionClass=1; u.enableAutoGather=1;
    for (auto &storage:u.resourceStorages) storage={-1,0,0};
    u.resourceStorages[0]={1,100,0};
    data.graphics.resize(5283);
    auto &g=data.graphics[5282]; g.exists=true; g.sequenceType=6; g.angleCount=12;
    data.terrainRestrictions.resize(12);
    auto &restriction=data.terrainRestrictions[11].passableBuildableDmgMultiplier;
    restriction={1,0,.05f,std::nextafter(.05f,1.f),.01f};
    std::vector<RmsTimberPlacement> placements;
    rng=1;
    check(generateRmsTimberPlacements(5,6,std::vector<uint8_t>(30,0),data,3,rng,placements,&err),"timber placement failed");
    check(placements.size()==3 && rng==1766988168u,"original three-tree count/RNG");
    check(placements[0].x==1.5f && placements[0].y==1.5f && placements[0].facet==4 &&
          placements[1].x==.5f && placements[1].y==3.5f && placements[1].facet==10 &&
          placements[2].x==.5f && placements[2].y==2.5f && placements[2].facet==9,
          "original centers/facets mismatch");
    for (uint8_t rejected:{1,2,4}) {
        rng=1;
        check(generateRmsTimberPlacements(5,6,std::vector<uint8_t>(30,rejected),data,3,rng,placements,&err)
              && placements.empty() && rng==1030492215u,"terrain rejection must not draw facets");
    }
    rng=1;
    check(generateRmsTimberPlacements(5,6,std::vector<uint8_t>(30,0),data,0,rng,placements,&err)
          && placements.empty() && rng==1030492215u,"zero request still shuffles candidates");
    rng=1;
    check(generateRmsTimberPlacements(1,1,{3},data,1,rng,placements,&err) && placements.size()==1,
          "restriction strictly above threshold accepted");
    rng=948628385u;
    check(generateRmsTimberPlacements(1,1,{0},data,1,rng,placements,&err) && placements[0].facet==11,
          "original maximum draw facet clamps to eleven");
    rng=1;
    check(generateRmsTimberPlacements(5,6,std::vector<uint8_t>(30,0),data,30,rng,placements,&err)
          && placements.size()==30,"adjacent trees must all fit");
    u.clearanceSize[0]=1;
    const auto before=rng;
    check(!generateRmsTimberPlacements(1,1,{0},data,1,rng,placements,&err) && placements.empty() && rng==before,
          "unsupported prototype must fail transactionally");
    u.clearanceSize[0]=.5f;
    check(!generateRmsTimberPlacements(1,1,{255},data,1,rng,placements,&err) && rng==before,
          "missing terrain restriction rejected");
    std::cout << "RMS object candidate tests passed\n";
}
