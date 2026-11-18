// SPDX-License-Identifier: GPL-3.0-or-later
#include "../src/engine/ai_build_placement.h"
#include <cassert>
#include <iostream>
#include <utility>
#include <vector>
// Independent original588b30 scan controls retained in
// build-placement-scan-controls-2026-11-18.json. Grid and master callbacks were
// controlled in that executable, not claimed to be original grid preparation.
int main() {
    using P=std::pair<int,int>;
    auto run=[](int footprint,int draw,int rejection,int edge,int scoreBoost) {
        int draws=0;std::vector<P> checks;
        const auto result=swgb::aiScanPrefabPlacement(64,64,edge?0:2,edge?0:2,
            edge?2:4,edge?2:4,edge?0:3,edge?0:3,footprint,
            [&](int x,int y){return rejection==2 && P(x,y)==P(3,3)?255:
                scoreBoost && P(x,y)==P(4,4)?9:1;},
            [&]{++draws;return draw;},
            [&](int x,int y){return rejection==3 && P(x,y)==P(3,3);},
            [&](float x,float y){assert(x-int(x)==0.5f && y-int(y)==0.5f);
                checks.emplace_back(int(x),int(y));return rejection!=1 || checks.back()!=P(3,3);});
        if(edge){assert(result.x==0 && result.y==0 && draws==2);assert(checks==std::vector<P>({{0,0}}));}
        else if(scoreBoost){assert(result.x==4 && result.y==4 && result.score==9);}
        else if(draw){assert(result.x==4 && result.y==4 && draws==8 && checks.size()==9);}
        else if(rejection){assert(result.x==2 && result.y==2 && draws==7);
            assert(checks== (rejection==1?std::vector<P>{{2,2},{3,3}}:std::vector<P>{{2,2}}));}
        else {assert(result.x==3 && result.y==3 && result.score==footprint*footprint && draws==5);
            assert(checks==std::vector<P>({{2,2},{3,3}}));}
    };
    run(1,0,0,0,0);run(1,1,0,0,0);run(2,0,0,0,0);run(3,0,0,0,0);
    run(1,0,1,0,0);run(1,0,2,0,0);run(1,0,0,1,0);run(1,0,3,0,0);run(1,0,0,0,1);
    std::cout<<"AI prefab placement checks passed\n";
}
