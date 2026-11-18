// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <optional>
#include <vector>

namespace swgb {
// Original590960's 24-byte resource record, represented semantically.
struct AiResourceRecord {
    int32_t id=-1;
    uint8_t x=0,y=0,selections=0;
    int32_t baseScore=0;
    uint8_t cooldown=0,invalid=0,depotDistance=0,resourceType=0;
    int32_t depotId=-1;
    uint32_t blockedAt=UINT32_MAX;
};
struct AiResourceBucket {
    std::vector<AiResourceRecord> records;
    int32_t cachedBestId=-1,cachedBestDistance=-1;
};
struct AiResourceDepot {
    int32_t id,prototype;
    float x,y;
    bool resolved,sameZone;
};
struct AiResourceDepotMetric {int32_t id=-1;double distance=std::sqrt(double(std::numeric_limits<float>::max()));};
inline bool aiResourceDepotCompatible(int resourceClass,int prototype,bool includeCommandCenter) {
    if(prototype==109 && !includeCommandCenter)return false;
    switch(resourceClass) {
    case 1:case 4:case 5:case 7:case 25:case 27:case 60:return prototype==68 || prototype==109;
    case 23:case 24:return prototype==45;
    case 31:return prototype==562 || prototype==109;
    case 29:return prototype==323 || prototype==109;
    case 30:return prototype==584 || prototype==109;
    default:return false;
    }
}
//590eb0 prefers ANY compatible same-zone depot over every other-zone depot.
// Squared best distance is stored float32 after each strict improvement.
inline AiResourceDepotMetric aiResourceDepotMetric(int resourceClass,float x,float y,
        bool includeCommandCenter,const std::vector<AiResourceDepot> &depots) {
    AiResourceDepotMetric result;float best=std::numeric_limits<float>::max();
    for(int pass=0;pass<2;++pass) {
        if(pass==1 && result.id!=-1)break;
        for(const auto &depot:depots) {
            if(!depot.resolved || !aiResourceDepotCompatible(resourceClass,depot.prototype,includeCommandCenter) ||
               (resourceClass!=25 && depot.sameZone!=(pass==0)))continue;
            const double dx=double(depot.x)-x,dy=double(depot.y)-y;
            const double squared=dx*dx+dy*dy;
            if(squared<best) {best=float(squared);result.id=depot.id;}
        }
    }
    result.distance=std::sqrt(double(best));return result;
}
inline void refreshAiResourceMetric(AiResourceRecord &record,const AiResourceDepotMetric &metric) {
    const int distance=int(std::min(255.,metric.distance));
    record.depotDistance=uint8_t(distance);record.depotId=metric.id;record.baseScore=2*distance;
}
inline AiResourceRecord makeAiResourceRecord(int32_t id,float x,float y,uint8_t resourceType,
                                             const AiResourceDepotMetric &metric) {
    AiResourceRecord record;record.id=id;record.x=uint8_t(int(x));record.y=uint8_t(int(y));
    record.resourceType=resourceType;refreshAiResourceMetric(record,metric);return record;
}
struct AiResourceTarget {
    int prototype,unitClass,owner;
    float x,y,resourceAmount;
    uint8_t state,taskReferences,visibility;
    bool contained,farmBlocked;
    std::optional<int> huntersAtTarget; //5ae220; required only for living class60.
};
enum class AiResourceSelectionStatus { Selected, None, Unsupported };
struct AiResourceSelection {
    AiResourceSelectionStatus status=AiResourceSelectionStatus::None;
    int32_t id=-1,depotId=-1,score=0;
    float x=0,y=0;
};
//586ec0 standard land/nontransport branch. Lookup exposes remembered IDs through
// the live registry, never an enumeration of unknown world objects. SN and zone
// callbacks may report unsupported input; they must not resize the record vector.
// Other original callers own admission/removal, timers, retry paths and dispatch.
template<class Lookup,class StrategicNumber,class SameZone>
AiResourceSelection selectAiLandResource(AiResourceBucket &bucket,int resourceType,int owner,
        float workerX,float workerY,uint32_t now,const std::vector<int32_t> &excluded,
        Lookup lookup,StrategicNumber strategicNumber,SameZone sameZone) {
    AiResourceSelection result;AiResourceRecord *best=nullptr;bool refreshBest=false;
    const auto signed32=[](uint32_t v){return v<=uint32_t(INT32_MAX)?int32_t(v):int32_t(int64_t(v)-0x100000000LL);};
    for(auto &record:bucket.records) {
        if(record.invalid==1)continue;
        if(record.cooldown) {if(!record.invalid)--record.cooldown;continue;}
        if(record.blockedAt!=UINT32_MAX && uint32_t(now-record.blockedAt)<60000u)continue;
        if(std::find(excluded.begin(),excluded.end(),record.id)!=excluded.end())continue;
        const auto target=lookup(record.id);
        if(!target) {record.invalid=1;if(bucket.cachedBestId==record.id)refreshBest=true;continue;}
        const int cls=target->unitClass;
        const int limitId=resourceType==0?(cls==4 || cls==60 || cls==5?235:234):
            resourceType==1?233:resourceType==2?237:resourceType==3?238:-1;
        if(limitId<0)return {AiResourceSelectionStatus::Unsupported};
        const auto limit=strategicNumber(limitId);
        if(!limit)return {AiResourceSelectionStatus::Unsupported};
        if(*limit!=-1 && int(record.depotDistance)>*limit)continue;
        if(cls==23 || cls==24)continue;
        if(target->prototype==50 && (target->owner!=owner || target->state!=2 ||
                                   target->taskReferences>0 || target->farmBlocked))continue;
        if(resourceType==1 && target->taskReferences>1)continue;
        if((resourceType==2 || resourceType==3) && target->taskReferences>4)continue;
        if(cls==27 && target->taskReferences>3)continue;
        if(cls==60 && target->state==2) {
            const auto minimum=strategicNumber(204);
            if(!minimum || !target->huntersAtTarget)return {AiResourceSelectionStatus::Unsupported};
            if(*target->huntersAtTarget<*minimum)continue;
        }
        if(cls==60 && target->state>2 && target->taskReferences>7)continue;
        if(cls==1 && (target->contained || target->taskReferences>3 || target->visibility!=15))continue;
        if(cls==4 && target->taskReferences>3)continue;
        if(cls==25 && target->taskReferences>2)continue;
        uint32_t raw=uint32_t(record.baseScore)*10u;
        const double dx=double(workerX)-target->x,dy=double(workerY)-target->y;
        const double distance=std::sqrt(dx*dx+dy*dy);
        if(distance<=2.)raw-=40u;else if(distance>4.)raw+=40u;
        if(cls==1)raw-=100u;else if(cls==27)raw-=75u;
        else if(cls==25 || target->prototype==50)raw-=50u;
        raw+=record.selections/(cls==1 || cls==27?150u:50u);
        if((cls==1 || cls==27) && target->taskReferences>2)raw+=200u;
        if((cls==4 || cls==60 || cls==5) && target->state>2 && target->resourceAmount>25.f)raw-=25u;
        const int32_t score=signed32(raw);
        if(best && score>=result.score)continue;
        if(cls!=25) {
            const auto zone=sameZone(record.x,record.y);
            if(!zone)return {AiResourceSelectionStatus::Unsupported};
            if(!*zone)continue; // Standard land scope has no original transport availability.
        }
        best=&record;result={AiResourceSelectionStatus::Selected,record.id,record.depotId,score,target->x,target->y};
    }
    if(refreshBest) {
        bucket.cachedBestId=-1;bucket.cachedBestDistance=-1;
        for(const auto &record:bucket.records)if(record.invalid!=1 &&
            (bucket.cachedBestDistance==-1 || record.depotDistance<bucket.cachedBestDistance)) {
            bucket.cachedBestId=record.id;bucket.cachedBestDistance=record.depotDistance;
        }
    }
    if(best)++best->selections;
    return result;
}
} // namespace swgb
