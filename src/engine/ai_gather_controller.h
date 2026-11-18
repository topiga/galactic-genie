// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "ai_gather_allocation.h"
namespace swgb {
struct AiGatherControllerInput {
    bool registered=true,contained=false,hasController=true;
    int task=-1,command=-1,priority=0;
    float carriedAmount=0;
    uint32_t hostTime=0;
    std::array<int32_t,4> assigned{},desired{};
};
// Original5a8670. Assignment callback implements5a94f0, including the final
// bucket=-1 fallback and attempted-bucket mask; its successful dispatch updates
// the record. The host clock is independent of simulation time.
template<class Record,class Assign>
bool updateAiGatherController(Record &record,const AiGatherControllerInput &in,Assign assign) {
    if(!in.registered || in.contained)return false;
    record.lastUpdate=in.hostTime;
    if(!in.hasController)return false;
    if(in.task==600 || in.task==602 || in.task==618)return true;
    std::array<int32_t,4> tried{};
    const auto clear=[&] {record.resource=-1;record.batch=-1;record.completed=-1;};
    const auto retry=[&] {
        if(record.resource<0 || record.resource>=4)return false;
        tried[size_t(record.resource)]=1;
        if(assign(record.resource,false,nullptr))return true;
        clear();return false;
    };
    if(in.carriedAmount>0) {
        if(in.task==-1 && record.resource!=-1)retry();
        return true;
    }
    if((in.command==710 && in.priority==100) || in.command==702)return true;
    if(record.resource>=0 && record.resource<4 &&
       in.assigned[size_t(record.resource)]<=in.desired[size_t(record.resource)]) {
        if(in.task!=-1 || retry())return true;
    }
    if(record.batch>record.completed) {
        if(in.task!=-1)return true;
        if(record.resource!=-1 && retry())return true;
    }
    for(int bucket:aiGatherUnmetOrder(in.assigned,in.desired)) {
        if(tried[size_t(bucket)] || in.assigned[size_t(bucket)]>=in.desired[size_t(bucket)])continue;
        if(record.resource==bucket && (in.task==609 || in.task==613))return true;
        tried[size_t(bucket)]=1;
        if(assign(bucket,false,nullptr))return true;
    }
    if(record.resource!=-1 && (in.task==609 || in.task==613))return true;
    assign(-1,true,&tried);
    return true;
}
} // namespace swgb
