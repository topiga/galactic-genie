// SPDX-License-Identifier: GPL-3.0-or-later
#include "engine/ai_gather_quota.h"
#include <iostream>
using namespace swgb;
int main() {
    struct Golden {AiGatherQuotaInput input;int explorers,gatherers;};
    const Golden rows[]={
        {{1,0,0,0,34,4,-1,-1,60,3,1,30,false},1,0},
        {{2,0,0,0,34,4,-1,-1,60,3,1,30,false},1,1},
        {{3,0,0,0,34,4,-1,-1,60,3,1,30,false},2,1},
        {{5,0,0,0,34,4,-1,-1,60,3,1,30,false},2,3},
        {{12,0,0,0,34,4,-1,-1,60,3,1,30,false},4,8},
        {{40,0,0,0,34,4,-1,-1,60,3,1,30,false},4,36},
        {{12,2,0,0,34,4,-1,-1,60,3,1,30,false},2,10},
        {{12,0,30,0,34,4,-1,-1,60,3,1,30,false},2,10},
        {{12,0,60,3,34,4,-1,-1,60,3,1,30,false},2,10},
        {{12,0,60,4,34,4,-1,-1,60,3,1,30,false},1,11},
        {{12,0,0,0,34,4,-1,-1,60,3,1,30,true},0,12},
        {{12,8,0,0,34,4,-1,-1,60,3,1,30,false},-4,16},
        {{1,0,0,0,34,4,0,0,60,3,1,30,false},1,0},
        {{2,0,0,0,34,4,0,0,60,3,1,30,false},0,0},
        {{2,0,0,0,34,4,-1,0,60,3,1,30,false},2,0},
        {{12,0,0,0,34,4,1,4,60,3,1,30,false},1,4},
        {{3,0,0,0,0,4,-1,-1,60,3,1,30,false},0,3},
        {{12,0,0,0,100,4,-1,-1,60,3,1,30,false},4,8},
    };
    for(const auto &g:rows) {
        const auto result=aiGatherQuota(g.input);
        if(!result || result->explorers!=g.explorers || result->gatherers!=g.gatherers) {
            std::cerr<<"Original gather quota mismatch\n";return 1;
        }
    }
    std::cout<<"Original gather quota: 18 goldens PASS\n";
}
