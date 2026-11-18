// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <algorithm>
#include <array>
#include <cstdint>
#include <optional>
#include <cmath>
#include <limits>
#include "tactical_command_queue.h"
#include <vector>

namespace swgb {
// Ordered type100 land-attack groups; admission and membership reconciliation belong to the caller.
struct AiMilitaryGatherGroup {
    int32_t id;
    int state=1;
    int desiredCount=0;
    std::vector<int32_t> members;
    std::array<float,3> destination{};
    uint8_t retries=0;
    uint32_t lastMemberAdded=0;
    uint32_t flags=0;
};

// 59fa80 admits these classes before testing the master air/boat trait.
inline bool aiMilitarySoldierClass(int cls) {
    switch(cls) {
    case 32:case 33:case 34:case 35:case 36:case 44:case 47:case 49:
    case 50:case 51:case 52:case 53:case 54:case 55:case 56:case 57:return true;
    default:return false;
    }
}
// 5ae520 multiplies by the float32 constant .01 before upward rounding.
inline int aiMilitaryAttackCount(size_t count,int percentage) {
    if(percentage<=0 || !count)return 0;
    const double rounded=std::ceil(double(count)*percentage*double(.01f));
    return int(std::min(rounded,double(std::numeric_limits<int>::max())));
}
// 5ad360/59ad00 removes from the first containing group, retaining desiredCount.
inline bool removeAiMilitaryMember(std::vector<AiMilitaryGatherGroup> &groups,int32_t id) {
    for(auto group=groups.begin();group!=groups.end();++group) {
        const auto member=std::find(group->members.begin(),group->members.end(),id);
        if(member==group->members.end())continue;
        group->members.erase(member);
        if(group->members.empty())groups.erase(group);
        return true;
    }
    return false;
}

// Type100 branch of5ab090. Caller supplies the rebuilt ungrouped pool and only
// the first4506f0 result; nullopt denotes an unsupported selection callback.
// This routine does not restrict group state: original initialized+44==1 does.
// Native preplanning-only callers must keep that restriction explicit.
template<class SelectFirst>
bool refillAiMilitaryGroup(AiMilitaryGatherGroup &group,int initialized,uint32_t flags,
                          uint32_t worldTime,SelectFirst selectFirst) {
    if(int64_t(group.members.size())>=int64_t(group.desiredCount))return true;
    if(flags&8u) {group.desiredCount=int(group.members.size());return true;}
    if(initialized==1)return true;
    const auto selected=selectFirst(group.desiredCount);
    if(!selected)return false;
    const size_t count=std::min(selected->size(),size_t(group.desiredCount));
    // Original59acb0 does not deduplicate and caps total membership at40.
    // The desired count caps APPENDS, not final membership: an existing group can overshoot its request.
    for(size_t i=0;i<count && group.members.size()<40;++i) {
        group.members.push_back((*selected)[i]);
        group.lastMemberAdded=worldTime;
    }
    return true;
}

// 5ab090's destination refresh only; this does not replace the rest of phase7.
// Lookup returns optional<array<float,3>> for the original first nonnegative ID.
template<class Lookup>
void refreshAiGatherDestinations(std::vector<AiMilitaryGatherGroup> &groups,Lookup lookup) {
    for(auto &group:groups) {
        if(group.state!=0 && group.state!=1)continue;
        const auto first=std::find_if(group.members.begin(),group.members.end(),[](int32_t id){return id>=0;});
        if(first==group.members.end())continue;
        if(const auto position=lookup(*first))group.destination=*position;
    }
}

// The caller supplies resolved551c60 footprint-edge distance, not centre distance.
// shortcut: Double represents unstored x87 values in the finite controls; wider boundary precision remains caller-owned.
struct AiMilitaryGatherMember {
    double edgeDistance;
    int task=-1,command=-1;
};

template<class Lookup>
bool aiMilitaryGatherReady(const AiMilitaryGatherGroup &group,int spacing,Lookup lookup) {
    for(const auto id:group.members) {
        if(id<0)continue;
        if(const auto member=lookup(id,group.destination)) {
            const float adjusted=std::max(0.f,float(member->edgeDistance-1.0));
            if(double(adjusted)>spacing)return false;
        }
    }
    return true;
}

// 5aaa00 uses632bac's truncated64-bit result's LOW32 bits, then caps the signed value at99.
inline uint8_t aiMilitaryGatherPriority(int sn34,float adjustedDistance) {
    const double quotient=double(sn34)/adjustedDistance;
    uint32_t low=0; // x87 invalid conversion writes INT64_MIN, whose low word is zero.
    if(std::isfinite(quotient) && quotient>=-0x1p63 && quotient<0x1p63)
        low=uint32_t(int64_t(quotient));
    const int64_t priority=low<=uint32_t(std::numeric_limits<int32_t>::max())?int64_t(low):int64_t(low)-0x100000000LL;
    return uint8_t(std::min<int64_t>(priority,99));
}

// 59b160 state7/12: Stop goes directly to opcode1; moves enter tactical staging as724.
// Lookup returns optional<AiMilitaryGatherMember>; missing IDs have no command side effects.
template<class Lookup,class Stop,class Move>
bool dispatchAiMilitaryGather(AiMilitaryGatherGroup &group,int player,int state,int spacing,
        int sn34,bool force,Lookup lookup,Stop stop,Move move) {
    if(state!=7 && state!=12)return false;
    if(group.retries<255)++group.retries;
    const float threshold=state==12?1.f:float(spacing);
    for(const auto id:group.members) {
        if(id<0)continue;
        const auto member=lookup(id,group.destination);
        if(!member || (!force && member->task!=-1 && member->task!=614))continue;
        const double adjusted=member->edgeDistance-1.0;
        // The branch retains the unstored distance; priority consumes its rounded float copy.
        if(adjusted<threshold) {
            if(member->command!=-1)stop(id);
        } else {
            TacticalCommandQueue::Order order{uint32_t(id),player,group.destination[0],group.destination[1]};
            order.command=724;order.z=group.destination[2];order.range=threshold;
            order.flagC=aiMilitaryGatherPriority(sn34,float(adjusted));
            move(order);
        }
    }
    return true;
}

enum class AiMilitaryReadinessResult {Complete,BudgetExhausted,Unsupported};

struct AiMilitaryNoDeficitRegions {
    std::optional<bool> operator()(const AiMilitaryGatherGroup &) const {return std::nullopt;}
};

// Original 5a1840 type100 phase8 pass, not the complete24-phase tactical scheduler.
struct AiMilitaryReadinessPass {
    int32_t nextGroupId=-1;

    // Ready/gather implement59bb60/59b160; stop resolves the ID before emitting opcode1.
    // Remembered performs5ad4a0 even though type100 ignores its result. Callbacks must not resize/reorder groups.
    template<class Clock,class Ready,class Gather,class Stop,class Remembered,class DeficitRegions=AiMilitaryNoDeficitRegions>
    AiMilitaryReadinessResult advance(std::vector<AiMilitaryGatherGroup> &groups,
            uint32_t started,uint32_t budget,Clock clock,Ready ready,Gather gather,
            Stop stop,Remembered remembered,DeficitRegions deficitRegions={}) {
        size_t index=0;
        if(nextGroupId!=-1) {
            const auto found=std::find_if(groups.begin(),groups.end(),[&](const auto &group){return group.id==nextGroupId;});
            if(found!=groups.end())index=size_t(found-groups.begin());
            else nextGroupId=-1;
        }
        for(;index<groups.size();++index) {
            auto &group=groups[index];
            if(group.state!=0 && group.state!=1)continue;
            const bool isReady=group.retries>10 || ready(group);
            bool release=isReady;
            if(int64_t(group.desiredCount)>int64_t(group.members.size())) {
                // 5a1945: first member and then home109 lookup even when readiness fails.
                // true: different member-map regions; false: same regions or a missing object.
                // nullopt exposes a caller that cannot yet provide the original region query.
                const std::optional<bool> different=deficitRegions(group);
                if(!different) {
                    nextGroupId=group.id;
                    return AiMilitaryReadinessResult::Unsupported;
                }
                release=*different;
                if(release)group.desiredCount=int(group.members.size());
            }
            if(release) {
                for(const auto id:group.members)if(id>=0)stop(id);
                remembered();
                group.state=2;
            } else if(!isReady)gather(group);
            nextGroupId=index+1<groups.size()?groups[index+1].id:-1;
            if(uint32_t(clock()-started)>=budget)
                return nextGroupId==-1?AiMilitaryReadinessResult::Complete:AiMilitaryReadinessResult::BudgetExhausted;
        }
        nextGroupId=-1;
        return AiMilitaryReadinessResult::Complete;
    }
};
// Bounded phase7/8/9 ordering from59f194/59f789; phase bodies and the outer cadence remain caller-owned.
enum class AiMilitaryPhase { Refresh=7, Readiness=8, Planning=9 };
struct AiMilitaryPhaseCycle {
    AiMilitaryPhase phase=AiMilitaryPhase::Refresh;
    template<class Clock,class Refresh,class Readiness,class Plan>
    void advance(uint32_t budget,Clock clock,Refresh refresh,Readiness readiness,Plan plan) {
        const uint32_t started=clock();
        if(phase==AiMilitaryPhase::Refresh) {
            refresh();phase=AiMilitaryPhase::Readiness;
            if(uint32_t(clock()-started)>=budget)return;
        }
        if(phase==AiMilitaryPhase::Readiness) {
            if(!readiness(started,budget))return;
            phase=AiMilitaryPhase::Planning;
            if(uint32_t(clock()-started)>=budget)return;
        }
        plan();phase=AiMilitaryPhase::Refresh;
    }
};
} // namespace swgb
