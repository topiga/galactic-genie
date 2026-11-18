// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "ai_resource_selection.h"
namespace swgb {
struct AiForageTarget {int unitClass=0;bool contained=false;};
// Original5857a0 updates InfoAI20f8, read by zero-argument57df40.
// This scan does not age cooldowns, invalidate missing IDs, or request fresh sight.
template<class Lookup>
bool aiSheepAndForageTooFar(const AiResourceBucket &food,uint32_t now,Lookup lookup) {
    for(const auto &record:food.records) {
        if(record.invalid==1 || record.cooldown!=0)continue;
        if(record.blockedAt!=UINT32_MAX && uint32_t(now-record.blockedAt)<60000u)continue;
        const auto target=lookup(record.id);
        if(!target)continue;
        if((target->unitClass==27 || (target->unitClass==1 && !target->contained)) &&
           record.depotDistance<=10)return false;
    }
    return true;
}
} // namespace swgb
