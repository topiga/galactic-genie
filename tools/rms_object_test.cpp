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
    // Full sequences from original4dded0; clipped rectangles and negative fallback.
    { uint32_t state=1u;
        check(generateRmsObjectCandidatesInArea(5,6,2,3,0,state,out,&err),"original area fixture failed");
        const std::vector<uint32_t> expected={17};
        check(out==expected && state==1u,"original area ordering/RNG mismatch");
    }
    { uint32_t state=1u;
        check(generateRmsObjectCandidatesInArea(5,6,2,3,1,state,out,&err),"original area fixture failed");
        const std::vector<uint32_t> expected={11,12,13,16,17,18,21,22,23};
        check(out==expected && state==1u,"original area ordering/RNG mismatch");
    }
    { uint32_t state=1u;
        check(generateRmsObjectCandidatesInArea(9,7,4,3,2,state,out,&err),"original area fixture failed");
        const std::vector<uint32_t> expected={29,20,11,12,13,14,15,21,22,23,24,30,31,32,33,38,39,40,41,42,47,48,49,50,51};
        check(out==expected && state==3884216597u,"original area ordering/RNG mismatch");
    }
    { uint32_t state=1u;
        check(generateRmsObjectCandidatesInArea(9,7,0,0,3,state,out,&err),"original area fixture failed");
        const std::vector<uint32_t> expected={9,0,1,2,3,10,11,12,18,19,20,21,27,28,29,30};
        check(out==expected && state==3357800067u,"original area ordering/RNG mismatch");
    }
    { uint32_t state=1u;
        check(generateRmsObjectCandidatesInArea(9,7,8,6,3,state,out,&err),"original area fixture failed");
        const std::vector<uint32_t> expected={41,32,33,34,35,42,43,44,50,51,52,53,59,60,61,62};
        check(out==expected && state==3357800067u,"original area ordering/RNG mismatch");
    }
    { uint32_t state=1u;
        check(generateRmsObjectCandidatesInArea(9,7,4,3,20,state,out,&err),"original area fixture failed");
        const std::vector<uint32_t> expected={2,22,37,32,38,18,0,1,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,19,20,21,23,24,25,26,27,28,29,30,31,33,34,35,36,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62};
        check(out==expected && state==2179684817u,"original area ordering/RNG mismatch");
    }
    { uint32_t state=1u;
        check(generateRmsObjectCandidatesInArea(1,9,0,4,3,state,out,&err),"original area fixture failed");
        const std::vector<uint32_t> expected={1,2,3,4,5,6,7};
        check(out==expected && state==1u,"original area ordering/RNG mismatch");
    }
    { uint32_t state=1u;
        check(generateRmsObjectCandidatesInArea(9,1,4,0,3,state,out,&err),"original area fixture failed");
        const std::vector<uint32_t> expected={1,2,3,4,5,6,7};
        check(out==expected && state==1u,"original area ordering/RNG mismatch");
    }
    { uint32_t state=1u;
        check(generateRmsObjectCandidatesInArea(9,7,-1,3,2,state,out,&err),"original area fixture failed");
        const std::vector<uint32_t> expected={2,22,37,32,38,18,0,1,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,19,20,21,23,24,25,26,27,28,29,30,31,33,34,35,36,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62};
        check(out==expected && state==2179684817u,"original area ordering/RNG mismatch");
    }
    { uint32_t state=1u;
        check(generateRmsObjectCandidatesInArea(9,7,4,-1,2,state,out,&err),"original area fixture failed");
        const std::vector<uint32_t> expected={2,22,37,32,38,18,0,1,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,19,20,21,23,24,25,26,27,28,29,30,31,33,34,35,36,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62};
        check(out==expected && state==2179684817u,"original area ordering/RNG mismatch");
    }
    { uint32_t state=1u;
        check(generateRmsObjectCandidatesInArea(9,7,4,3,-1,state,out,&err),"original area fixture failed");
        const std::vector<uint32_t> expected={2,22,37,32,38,18,0,1,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,19,20,21,23,24,25,26,27,28,29,30,31,33,34,35,36,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62};
        check(out==expected && state==2179684817u,"original area ordering/RNG mismatch");
    }
    { uint32_t state=12345u;
        check(generateRmsObjectCandidatesInArea(5,6,2,3,0,state,out,&err),"original area fixture failed");
        const std::vector<uint32_t> expected={17};
        check(out==expected && state==12345u,"original area ordering/RNG mismatch");
    }
    { uint32_t state=12345u;
        check(generateRmsObjectCandidatesInArea(5,6,2,3,1,state,out,&err),"original area fixture failed");
        const std::vector<uint32_t> expected={11,12,13,16,17,18,21,22,23};
        check(out==expected && state==12345u,"original area ordering/RNG mismatch");
    }
    { uint32_t state=12345u;
        check(generateRmsObjectCandidatesInArea(9,7,4,3,2,state,out,&err),"original area fixture failed");
        const std::vector<uint32_t> expected={31,20,11,12,13,14,15,21,22,23,24,29,30,32,33,38,39,40,41,42,47,48,49,50,51};
        check(out==expected && state==1449997005u,"original area ordering/RNG mismatch");
    }
    { uint32_t state=12345u;
        check(generateRmsObjectCandidatesInArea(9,7,0,0,3,state,out,&err),"original area fixture failed");
        const std::vector<uint32_t> expected={9,0,1,2,3,10,11,12,18,19,20,21,27,28,29,30};
        check(out==expected && state==1255958651u,"original area ordering/RNG mismatch");
    }
    { uint32_t state=12345u;
        check(generateRmsObjectCandidatesInArea(9,7,8,6,3,state,out,&err),"original area fixture failed");
        const std::vector<uint32_t> expected={41,32,33,34,35,42,43,44,50,51,52,53,59,60,61,62};
        check(out==expected && state==1255958651u,"original area ordering/RNG mismatch");
    }
    { uint32_t state=12345u;
        check(generateRmsObjectCandidatesInArea(9,7,4,3,20,state,out,&err),"original area fixture failed");
        const std::vector<uint32_t> expected={20,31,21,33,5,28,32,19,0,1,2,3,4,6,7,8,9,10,11,12,13,14,15,16,17,18,22,23,24,25,26,27,29,30,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62};
        check(out==expected && state==922525705u,"original area ordering/RNG mismatch");
    }
    { uint32_t state=12345u;
        check(generateRmsObjectCandidatesInArea(1,9,0,4,3,state,out,&err),"original area fixture failed");
        const std::vector<uint32_t> expected={1,2,3,4,5,6,7};
        check(out==expected && state==12345u,"original area ordering/RNG mismatch");
    }
    { uint32_t state=12345u;
        check(generateRmsObjectCandidatesInArea(9,1,4,0,3,state,out,&err),"original area fixture failed");
        const std::vector<uint32_t> expected={1,2,3,4,5,6,7};
        check(out==expected && state==12345u,"original area ordering/RNG mismatch");
    }
    { uint32_t state=12345u;
        check(generateRmsObjectCandidatesInArea(9,7,-1,3,2,state,out,&err),"original area fixture failed");
        const std::vector<uint32_t> expected={20,31,21,33,5,28,32,19,0,1,2,3,4,6,7,8,9,10,11,12,13,14,15,16,17,18,22,23,24,25,26,27,29,30,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62};
        check(out==expected && state==922525705u,"original area ordering/RNG mismatch");
    }
    { uint32_t state=12345u;
        check(generateRmsObjectCandidatesInArea(9,7,4,-1,2,state,out,&err),"original area fixture failed");
        const std::vector<uint32_t> expected={20,31,21,33,5,28,32,19,0,1,2,3,4,6,7,8,9,10,11,12,13,14,15,16,17,18,22,23,24,25,26,27,29,30,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62};
        check(out==expected && state==922525705u,"original area ordering/RNG mismatch");
    }
    { uint32_t state=12345u;
        check(generateRmsObjectCandidatesInArea(9,7,4,3,-1,state,out,&err),"original area fixture failed");
        const std::vector<uint32_t> expected={20,31,21,33,5,28,32,19,0,1,2,3,4,6,7,8,9,10,11,12,13,14,15,16,17,18,22,23,24,25,26,27,29,30,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62};
        check(out==expected && state==922525705u,"original area ordering/RNG mismatch");
    }
    { uint32_t state=1967u;
        check(generateRmsObjectCandidatesInArea(5,6,2,3,0,state,out,&err),"original area fixture failed");
        const std::vector<uint32_t> expected={17};
        check(out==expected && state==1967u,"original area ordering/RNG mismatch");
    }
    { uint32_t state=1967u;
        check(generateRmsObjectCandidatesInArea(5,6,2,3,1,state,out,&err),"original area fixture failed");
        const std::vector<uint32_t> expected={11,12,13,16,17,18,21,22,23};
        check(out==expected && state==1967u,"original area ordering/RNG mismatch");
    }
    { uint32_t state=1967u;
        check(generateRmsObjectCandidatesInArea(9,7,4,3,2,state,out,&err),"original area fixture failed");
        const std::vector<uint32_t> expected={23,20,11,12,13,14,15,21,22,24,29,30,31,32,33,38,39,40,41,42,47,48,49,50,51};
        check(out==expected && state==3326033699u,"original area ordering/RNG mismatch");
    }
    { uint32_t state=1967u;
        check(generateRmsObjectCandidatesInArea(9,7,0,0,3,state,out,&err),"original area fixture failed");
        const std::vector<uint32_t> expected={0,1,2,3,9,10,11,12,18,19,20,21,27,28,29,30};
        check(out==expected && state==948628385u,"original area ordering/RNG mismatch");
    }
    { uint32_t state=1967u;
        check(generateRmsObjectCandidatesInArea(9,7,8,6,3,state,out,&err),"original area fixture failed");
        const std::vector<uint32_t> expected={32,33,34,35,41,42,43,44,50,51,52,53,59,60,61,62};
        check(out==expected && state==948628385u,"original area ordering/RNG mismatch");
    }
    { uint32_t state=1967u;
        check(generateRmsObjectCandidatesInArea(9,7,4,3,20,state,out,&err),"original area fixture failed");
        const std::vector<uint32_t> expected={14,15,20,39,9,3,25,19,0,1,2,4,5,6,7,8,10,11,12,13,16,17,18,21,22,23,24,26,27,28,29,30,31,32,33,34,35,36,37,38,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62};
        check(out==expected && state==2701782783u,"original area ordering/RNG mismatch");
    }
    { uint32_t state=1967u;
        check(generateRmsObjectCandidatesInArea(1,9,0,4,3,state,out,&err),"original area fixture failed");
        const std::vector<uint32_t> expected={1,2,3,4,5,6,7};
        check(out==expected && state==1967u,"original area ordering/RNG mismatch");
    }
    { uint32_t state=1967u;
        check(generateRmsObjectCandidatesInArea(9,1,4,0,3,state,out,&err),"original area fixture failed");
        const std::vector<uint32_t> expected={1,2,3,4,5,6,7};
        check(out==expected && state==1967u,"original area ordering/RNG mismatch");
    }
    { uint32_t state=1967u;
        check(generateRmsObjectCandidatesInArea(9,7,-1,3,2,state,out,&err),"original area fixture failed");
        const std::vector<uint32_t> expected={14,15,20,39,9,3,25,19,0,1,2,4,5,6,7,8,10,11,12,13,16,17,18,21,22,23,24,26,27,28,29,30,31,32,33,34,35,36,37,38,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62};
        check(out==expected && state==2701782783u,"original area ordering/RNG mismatch");
    }
    { uint32_t state=1967u;
        check(generateRmsObjectCandidatesInArea(9,7,4,-1,2,state,out,&err),"original area fixture failed");
        const std::vector<uint32_t> expected={14,15,20,39,9,3,25,19,0,1,2,4,5,6,7,8,10,11,12,13,16,17,18,21,22,23,24,26,27,28,29,30,31,32,33,34,35,36,37,38,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62};
        check(out==expected && state==2701782783u,"original area ordering/RNG mismatch");
    }
    { uint32_t state=1967u;
        check(generateRmsObjectCandidatesInArea(9,7,4,3,-1,state,out,&err),"original area fixture failed");
        const std::vector<uint32_t> expected={14,15,20,39,9,3,25,19,0,1,2,4,5,6,7,8,10,11,12,13,16,17,18,21,22,23,24,26,27,28,29,30,31,32,33,34,35,36,37,38,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62};
        check(out==expected && state==2701782783u,"original area ordering/RNG mismatch");
    }
    { uint32_t state=4294967295u;
        check(generateRmsObjectCandidatesInArea(5,6,2,3,0,state,out,&err),"original area fixture failed");
        const std::vector<uint32_t> expected={17};
        check(out==expected && state==4294967295u,"original area ordering/RNG mismatch");
    }
    { uint32_t state=4294967295u;
        check(generateRmsObjectCandidatesInArea(5,6,2,3,1,state,out,&err),"original area fixture failed");
        const std::vector<uint32_t> expected={11,12,13,16,17,18,21,22,23};
        check(out==expected && state==4294967295u,"original area ordering/RNG mismatch");
    }
    { uint32_t state=4294967295u;
        check(generateRmsObjectCandidatesInArea(9,7,4,3,2,state,out,&err),"original area fixture failed");
        const std::vector<uint32_t> expected={20,29,11,12,13,14,15,21,22,23,24,30,31,32,33,38,39,40,41,42,47,48,49,50,51};
        check(out==expected && state==730189939u,"original area ordering/RNG mismatch");
    }
    { uint32_t state=4294967295u;
        check(generateRmsObjectCandidatesInArea(9,7,0,0,3,state,out,&err),"original area fixture failed");
        const std::vector<uint32_t> expected={9,0,1,2,3,10,11,12,18,19,20,21,27,28,29,30};
        check(out==expected && state==1948984945u,"original area ordering/RNG mismatch");
    }
    { uint32_t state=4294967295u;
        check(generateRmsObjectCandidatesInArea(9,7,8,6,3,state,out,&err),"original area fixture failed");
        const std::vector<uint32_t> expected={41,32,33,34,35,42,43,44,50,51,52,53,59,60,61,62};
        check(out==expected && state==1948984945u,"original area ordering/RNG mismatch");
    }
    { uint32_t state=4294967295u;
        check(generateRmsObjectCandidatesInArea(9,7,4,3,20,state,out,&err),"original area fixture failed");
        const std::vector<uint32_t> expected={42,20,37,5,1,15,9,36,0,2,3,4,6,7,8,10,11,12,13,14,16,17,18,19,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,38,39,40,41,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62};
        check(out==expected && state==4202113871u,"original area ordering/RNG mismatch");
    }
    { uint32_t state=4294967295u;
        check(generateRmsObjectCandidatesInArea(1,9,0,4,3,state,out,&err),"original area fixture failed");
        const std::vector<uint32_t> expected={1,2,3,4,5,6,7};
        check(out==expected && state==4294967295u,"original area ordering/RNG mismatch");
    }
    { uint32_t state=4294967295u;
        check(generateRmsObjectCandidatesInArea(9,1,4,0,3,state,out,&err),"original area fixture failed");
        const std::vector<uint32_t> expected={1,2,3,4,5,6,7};
        check(out==expected && state==4294967295u,"original area ordering/RNG mismatch");
    }
    { uint32_t state=4294967295u;
        check(generateRmsObjectCandidatesInArea(9,7,-1,3,2,state,out,&err),"original area fixture failed");
        const std::vector<uint32_t> expected={42,20,37,5,1,15,9,36,0,2,3,4,6,7,8,10,11,12,13,14,16,17,18,19,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,38,39,40,41,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62};
        check(out==expected && state==4202113871u,"original area ordering/RNG mismatch");
    }
    { uint32_t state=4294967295u;
        check(generateRmsObjectCandidatesInArea(9,7,4,-1,2,state,out,&err),"original area fixture failed");
        const std::vector<uint32_t> expected={42,20,37,5,1,15,9,36,0,2,3,4,6,7,8,10,11,12,13,14,16,17,18,19,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,38,39,40,41,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62};
        check(out==expected && state==4202113871u,"original area ordering/RNG mismatch");
    }
    { uint32_t state=4294967295u;
        check(generateRmsObjectCandidatesInArea(9,7,4,3,-1,state,out,&err),"original area fixture failed");
        const std::vector<uint32_t> expected={42,20,37,5,1,15,9,36,0,2,3,4,6,7,8,10,11,12,13,14,16,17,18,19,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,38,39,40,41,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62};
        check(out==expected && state==4202113871u,"original area ordering/RNG mismatch");
    }
    rng=12345;
    check(!generateRmsObjectCandidatesInArea(9,7,9,3,2,rng,out,&err) &&
        out.empty() && rng==12345 && !err.empty(),"area outside center must fail transactionally");
    check(!generateRmsObjectCandidatesInArea(9,7,4,3,1025,rng,out,&err) &&
        out.empty() && rng==12345,"area radius limit must preserve RNG");
    check(!generateRmsObjectCandidatesInArea(0,7,0,3,0,rng,out,&err) &&
        out.empty() && rng==12345,"invalid area dimensions must preserve RNG");
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
    rng=1;
    check(generateRmsTimberPlacements(5,6,std::vector<uint8_t>(30,0),data,3,rng,placements,&err,{6,15}) &&
          rng==1766988168u && placements.size()==3 &&
          placements[0].x==.5f && placements[0].y==2.5f && placements[0].facet==4 &&
          placements[1].x==.5f && placements[1].y==.5f && placements[1].facet==10 &&
          placements[2].x==1.5f && placements[2].y==.5f && placements[2].facet==9,
          "occupied scenery centers reject without consuming facet RNG");
    std::vector<uint32_t> occupied(30);
    std::iota(occupied.begin(),occupied.end(),0);
    rng=1;
    check(generateRmsTimberPlacements(5,6,std::vector<uint8_t>(30,0),data,30,rng,placements,&err,occupied) &&
          placements.empty() && rng==1030492215u,"fully occupied map still consumes candidate shuffle only");
    rng=1;
    check(!generateRmsTimberPlacements(5,6,std::vector<uint8_t>(30,0),data,3,rng,placements,&err,{30}) &&
          placements.empty() && rng==1,"invalid occupancy must fail transactionally");
    u.clearanceSize[0]=1;
    const auto before=rng;
    check(!generateRmsTimberPlacements(1,1,{0},data,1,rng,placements,&err) && placements.empty() && rng==before,
          "unsupported prototype must fail transactionally");
    u.clearanceSize[0]=.5f;
    check(!generateRmsTimberPlacements(1,1,{255},data,1,rng,placements,&err) && rng==before,
          "missing terrain restriction rejected");
    std::cout << "RMS object candidate tests passed\n";
}
