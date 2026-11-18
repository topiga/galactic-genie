// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "ai_resource_selection.h"
namespace swgb {
// Actual58a660/58a8a0 notification order, including their asymmetric depot IDs.
// Registry mutation precedes these refreshes. Farm50 admission separately invokes590960.
inline std::vector<int> aiDepotRefreshBuckets(int prototype,int unitClass,bool removed) {
    if(!removed && !((unitClass>=6 && unitClass<=10) || unitClass==18))return {};
    if(prototype==109)return {0,1,3,2};
    if(prototype==584)return removed?std::vector<int>{2}:std::vector<int>{3,2};
    if(prototype==323)return removed?std::vector<int>{3}:std::vector<int>{};
    if(prototype==68 || (prototype==45 && !removed))return {0};
    if(prototype==562)return {1};
    return {};
}
//590960 duplicate observation updates only metric fields, even for invalid records.
// The best cache only improves; a farther formerly-best resource leaves a stale
// cache value until591370. Do not reset coordinates or revive invalid records.
inline void observeAiResourceMetric(AiResourceBucket &bucket,AiResourceRecord &record,
                                    const AiResourceDepotMetric &metric) {
    refreshAiResourceMetric(record,metric);
    if(bucket.cachedBestDistance==-1 || record.depotDistance<bucket.cachedBestDistance) {
        bucket.cachedBestDistance=record.depotDistance;bucket.cachedBestId=record.id;
    }
}
// Original585ac0 refreshes food only when unsigned elapsed time is strictly>15s.
inline bool aiFoodMetricRefreshDue(uint32_t now,uint32_t last) {return uint32_t(now-last)>15000u;}
// Original591370: missing objects do not invalidate their resource records here.
// Metric returns nullopt only for a missing object; callers must handle unsupported
// geometry before invoking this loop. Positions, cooldowns and use counters persist.
template<class Metric>
void refreshAiResourceBucket(AiResourceBucket &bucket,Metric metric) {
    bucket.cachedBestId=-1;bucket.cachedBestDistance=-1;
    for(auto &record:bucket.records) {
        if(record.invalid==1)continue;
        const auto value=metric(record.id);if(!value)continue;
        refreshAiResourceMetric(record,*value);
        if(bucket.cachedBestDistance==-1 || record.depotDistance<bucket.cachedBestDistance) {
            bucket.cachedBestDistance=record.depotDistance;bucket.cachedBestId=record.id;
        }
    }
}
} // namespace swgb
