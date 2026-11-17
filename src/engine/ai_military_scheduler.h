// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <algorithm>
#include <array>
#include <cstdint>
#include <optional>
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
};

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

enum class AiMilitaryReadinessResult {Complete,BudgetExhausted,Unsupported};

// Original 5a1840 full-count type100 phase8 pass, not the complete24-phase tactical scheduler.
struct AiMilitaryReadinessPass {
    int32_t nextGroupId=-1;

    // Ready/gather implement59bb60/59b160; stop resolves the ID before emitting opcode1.
    // Remembered performs5ad4a0 even though type100 ignores its result. Callbacks must not resize/reorder groups.
    template<class Clock,class Ready,class Gather,class Stop,class Remembered>
    AiMilitaryReadinessResult advance(std::vector<AiMilitaryGatherGroup> &groups,
            uint32_t started,uint32_t budget,Clock clock,Ready ready,Gather gather,
            Stop stop,Remembered remembered) {
        size_t index=0;
        if(nextGroupId!=-1) {
            const auto found=std::find_if(groups.begin(),groups.end(),[&](const auto &group){return group.id==nextGroupId;});
            if(found!=groups.end())index=size_t(found-groups.begin());
            else nextGroupId=-1;
        }
        for(;index<groups.size();++index) {
            auto &group=groups[index];
            if(group.state!=0 && group.state!=1)continue;
            // shortcut: Count-deficit groups require original home-object/region callbacks before admission.
            if(int64_t(group.desiredCount)>int64_t(group.members.size())) {
                nextGroupId=group.id;
                return AiMilitaryReadinessResult::Unsupported;
            }
            if(group.retries>10 || ready(group)) {
                for(const auto id:group.members)if(id>=0)stop(id);
                remembered();
                group.state=2;
            } else gather(group);
            nextGroupId=index+1<groups.size()?groups[index+1].id:-1;
            if(uint32_t(clock()-started)>=budget)
                return nextGroupId==-1?AiMilitaryReadinessResult::Complete:AiMilitaryReadinessResult::BudgetExhausted;
        }
        nextGroupId=-1;
        return AiMilitaryReadinessResult::Complete;
    }
};
} // namespace swgb
