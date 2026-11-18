// SPDX-License-Identifier: GPL-3.0-or-later
#include "engine/ai_resource_selection.h"
#include <cstdlib>
#include <iostream>
using namespace swgb;
static void check(bool value,const char *message) {if(!value){std::cerr<<message<<'\n';std::exit(1);}}
int main() {
    struct DepotGolden {int cls;bool include;std::vector<AiResourceDepot> depots;int selected;float metric;};
    const DepotGolden depots[]={
        {1,true,{{1,68,13,14,true,true},{2,109,20,10,true,true}},1,5.0f},
        {4,true,{{1,68,13,14,true,true},{2,109,20,10,true,true}},1,5.0f},
        {7,true,{{1,68,13,14,true,true},{2,109,20,10,true,true}},1,5.0f},
        {25,true,{{1,68,13,14,true,true},{2,109,20,10,true,true}},1,5.0f},
        {27,true,{{1,68,13,14,true,true},{2,109,20,10,true,true}},1,5.0f},
        {60,true,{{1,68,13,14,true,true},{2,109,20,10,true,true}},1,5.0f},
        {31,true,{{1,562,13,14,true,true},{2,109,20,10,true,true}},1,5.0f},
        {29,true,{{1,323,13,14,true,true},{2,109,20,10,true,true}},1,5.0f},
        {30,true,{{1,584,13,14,true,true},{2,109,20,10,true,true}},1,5.0f},
        {23,true,{{1,45,13,14,true,true},{2,109,20,10,true,true}},1,5.0f},
        {24,true,{{1,45,13,14,true,true},{2,109,20,10,true,true}},1,5.0f},
        {3,true,{{1,68,13,14,true,true},{2,109,20,10,true,true}},-1,1.8446742974197924e+19f},
        {31,true,{{1,562,12,10,true,false},{2,109,20,10,true,true}},2,10.0f},
        {31,true,{{1,562,12,10,true,false},{2,109,20,10,true,false}},1,2.0f},
        {31,false,{{1,109,11,10,true,true},{2,562,14,10,true,true}},2,4.0f},
        {27,true,{{1,68,13,14,true,true},{2,109,14,13,true,true}},1,5.0f},
        {27,true,{{1,68,13,14,false,true}},-1,1.8446742974197924e+19f},
        {25,true,{{1,68,12,10,true,false},{2,109,20,10,true,true}},1,2.0f},
    };
    for(const auto &g:depots) {
        const auto result=aiResourceDepotMetric(g.cls,10,10,g.include,g.depots);
        check(result.id==g.selected && float(result.distance)==g.metric,"Original depot selection golden mismatch");
    }
    struct Entry {int cls,x,y,metric,refs,cooldown,invalid,stamp,zone,selections,proto,state,amount;bool missing;};
    struct Golden {int bucket,limit,visibility,cachedBest;std::vector<Entry> entries;std::vector<int32_t> excluded;
                   int selected,score,postCachedBest;std::vector<int> postCooldown,postInvalid,postSelections;};
    const Golden rows[]={
        {0,-1,15,-1,{{27,12,10,3,0,0,0,-1,1,0,999,2,100,false}},{},1,-55,-1,{0},{0},{1}},
        {0,-1,15,-1,{{27,12,10,3,0,0,0,-1,1,0,999,2,100,false},{27,13,10,2,0,0,0,-1,1,0,999,2,100,false}},{},1,-55,-1,{0,0},{0,0},{1,0}},
        {0,-1,15,-1,{{27,12,10,3,0,0,0,-1,1,0,999,2,100,false},{27,14,10,3,0,0,0,-1,1,0,999,2,100,false}},{},1,-55,-1,{0,0},{0,0},{1,0}},
        {0,-1,15,-1,{{27,12,10,3,0,2,0,-1,1,0,999,2,100,false},{27,12,10,3,0,0,0,-1,1,0,999,2,100,false}},{},2,-55,-1,{1,0},{0,0},{0,1}},
        {0,-1,15,-1,{{27,12,10,3,0,0,1,-1,1,0,999,2,100,false},{27,12,10,3,0,0,0,-1,1,0,999,2,100,false}},{},2,-55,-1,{0,0},{1,0},{0,1}},
        {0,-1,15,-1,{{27,12,10,3,0,0,0,-1,1,0,999,2,100,true},{27,12,10,3,0,0,0,-1,1,0,999,2,100,false}},{},2,-55,-1,{0,0},{1,0},{0,1}},
        {0,-1,15,-1,{{27,12,10,3,0,0,0,50000,1,0,999,2,100,false},{27,12,10,3,0,0,0,-1,1,0,999,2,100,false}},{},2,-55,-1,{0,0},{0,0},{0,1}},
        {0,-1,15,-1,{{27,12,10,3,0,0,0,40000,1,0,999,2,100,false},{27,12,10,3,0,0,0,-1,1,0,999,2,100,false}},{},1,-55,-1,{0,0},{0,0},{1,0}},
        {0,5,15,-1,{{27,12,10,10,0,0,0,-1,1,0,999,2,100,false},{27,12,10,3,0,0,0,-1,1,0,999,2,100,false}},{},2,-55,-1,{0,0},{0,0},{0,1}},
        {0,-1,15,-1,{{27,12,10,3,0,0,0,-1,2,0,999,2,100,false},{27,12,10,3,0,0,0,-1,1,0,999,2,100,false}},{},-1,-1,-1,{0,0},{0,0},{0,0}},
        {0,-1,15,-1,{{27,12,10,3,4,0,0,-1,1,0,999,2,100,false},{27,12,10,3,0,0,0,-1,1,0,999,2,100,false}},{},2,-55,-1,{0,0},{0,0},{0,1}},
        {0,-1,0,-1,{{1,12,10,3,0,0,0,-1,1,0,999,2,100,false}},{},-1,-1,-1,{0},{0},{0}},
        {0,-1,0,-1,{{27,12,10,3,0,0,0,-1,1,0,999,2,100,false}},{},1,-55,-1,{0},{0},{1}},
        {0,-1,15,-1,{{1,12,10,3,0,0,0,-1,1,0,999,2,100,false}},{},1,-80,-1,{0},{0},{1}},
        {0,-1,15,-1,{{4,12,10,3,0,0,0,-1,1,0,999,2,100,false}},{},1,20,-1,{0},{0},{1}},
        {0,-1,15,-1,{{5,12,10,3,0,0,0,-1,1,0,999,2,100,false}},{},1,20,-1,{0},{0},{1}},
        {0,-1,15,-1,{{7,12,10,3,0,0,0,-1,1,0,999,2,100,false}},{},1,20,-1,{0},{0},{1}},
        {0,-1,15,-1,{{25,12,10,3,0,0,0,-1,1,0,999,2,100,false}},{},1,-30,-1,{0},{0},{1}},
        {0,-1,15,-1,{{27,12,10,3,0,0,0,-1,1,0,999,2,100,false}},{},1,-55,-1,{0},{0},{1}},
        {3,-1,15,-1,{{29,12,10,3,0,0,0,-1,1,0,999,2,100,false}},{},1,20,-1,{0},{0},{1}},
        {2,-1,15,-1,{{30,12,10,3,0,0,0,-1,1,0,999,2,100,false}},{},1,20,-1,{0},{0},{1}},
        {1,-1,15,-1,{{31,12,10,3,0,0,0,-1,1,0,999,2,100,false}},{},1,20,-1,{0},{0},{1}},
        {0,-1,15,-1,{{60,12,10,3,0,0,0,-1,1,0,999,2,100,false}},{},1,20,-1,{0},{0},{1}},
        {0,-1,15,-1,{{23,12,10,3,0,0,0,-1,1,0,999,2,100,false}},{},-1,-1,-1,{0},{0},{0}},
        {0,-1,15,-1,{{24,12,10,3,0,0,0,-1,1,0,999,2,100,false}},{},-1,-1,-1,{0},{0},{0}},
        {0,-1,15,1,{{27,12,10,3,0,0,0,-1,1,0,999,2,100,true},{27,12,10,3,0,0,0,-1,1,0,999,2,100,false}},{},2,-55,2,{0,0},{1,0},{0,1}},
        {0,-1,15,-1,{{27,12,10,3,0,0,0,-1,1,0,999,2,100,false},{27,13,10,3,0,0,0,-1,1,0,999,2,100,false}},{1},2,-15,-1,{0,0},{0,0},{0,1}},
        {0,-1,15,-1,{{27,12,10,3,0,0,0,-1,1,150,999,2,100,false}},{},1,-54,-1,{0},{0},{151}},
        {0,-1,15,-1,{{27,12,10,3,0,0,0,-1,1,255,999,2,100,false}},{},1,-54,-1,{0},{0},{0}},
        {0,-1,15,-1,{{27,12,10,3,3,0,0,-1,1,0,999,2,100,false}},{},1,145,-1,{0},{0},{1}},
        {0,-1,15,-1,{{4,12,10,3,0,0,0,-1,1,0,999,3,26,false}},{},1,-5,-1,{0},{0},{1}},
        {0,-1,15,-1,{{4,12,10,3,0,0,0,-1,1,0,999,3,25,false}},{},1,20,-1,{0},{0},{1}},
        {0,-1,15,-1,{{27,12,10,3,0,0,0,-1,1,0,50,2,100,false}},{},1,-55,-1,{0},{0},{1}},
        {0,-1,15,-1,{{27,12,10,3,1,0,0,-1,1,0,50,2,100,false}},{},-1,-1,-1,{0},{0},{0}},
        {0,-1,15,-1,{{27,15,10,3,0,0,0,-1,1,0,999,2,100,false},{27,13,10,3,0,0,0,-1,1,0,999,2,100,false}},{},2,-15,-1,{0,0},{0,0},{0,1}},
    };
    for(const auto &g:rows) {
        AiResourceBucket bucket;bucket.cachedBestId=g.cachedBest;
        for(size_t i=0;i<g.entries.size();++i) {
            const auto &e=g.entries[i];auto record=makeAiResourceRecord(int(i+1),float(e.x),float(e.y),uint8_t(g.bucket),{109,double(e.metric)});
            record.cooldown=uint8_t(e.cooldown);record.invalid=uint8_t(e.invalid);
            record.blockedAt=uint32_t(e.stamp);record.selections=uint8_t(e.selections);bucket.records.push_back(record);
        }
        const auto result=selectAiLandResource(bucket,g.bucket,1,10,10,100000,g.excluded,
            [&](int32_t id)->std::optional<AiResourceTarget> {
                const auto &e=g.entries.at(size_t(id-1));if(e.missing)return std::nullopt;
                return AiResourceTarget{e.proto,e.cls,1,float(e.x),float(e.y),float(e.amount),uint8_t(e.state),
                    uint8_t(e.refs),uint8_t(g.visibility),false,false,20};
            },[&](int)->std::optional<int>{return g.limit;},
            [&](int x,int y)->std::optional<bool>{for(const auto &e:g.entries)if(e.x==x && e.y==y)return e.zone==1;return true;});
        check(result.status!=(AiResourceSelectionStatus::Unsupported) && result.id==g.selected,
              "Original resource target selection golden mismatch");
        if(result.id!=-1)check(result.score==g.score,"Original resource target score mismatch");
        check(bucket.cachedBestId==g.postCachedBest,"Missing resource repairs cached minimum with original record order");
        for(size_t i=0;i<bucket.records.size();++i)check(bucket.records[i].cooldown==g.postCooldown[i] &&
            bucket.records[i].invalid==g.postInvalid[i] && bucket.records[i].selections==g.postSelections[i],
            "Original cooldown, invalidation and selection-count lifecycle mismatch");
    }
    std::cout<<"Original resource selection: 18 depot and35 selector goldens PASS\n";
}
