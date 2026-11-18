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
struct AiGroupSearchState {
    // Original Vec3 order x,z,y: group+194 centroid and +244 retained search origin.
    std::array<float,3> centroid{}, origin{};
    uint32_t lastBusyTime=0, started=0;
};

// Original5ae7f0 after actor lookup. The caller resolves controller task/target,
// virtual1c0 action state and movement-group482710(group,0) independently.
inline bool aiGroupSearchIdle(int actionState,int task,int target,
                              bool movementGroupPresent,bool movementGroupReady) {
    if(actionState!=0 && actionState!=1 && actionState!=2 && actionState!=3 &&
       actionState!=13 && actionState!=14)return false;
    if(task!=-1 && !(task==615 && target==-1 && actionState==2))return false;
    return !movementGroupPresent || movementGroupReady;
}
struct AiGroupSearchMember {
    std::array<float,3> position{};
    // Original59a460: resolved, has controller, HP>0 and still owned by this AI.
    bool eligible=true, idle=true;
};
// Recovered centroid/flag0/last-busy portion of59a460. Member removal and other
// flag/accounting updates remain the caller's responsibility. Empty valid sets
// are unsupported here (original divides by zero before later cleanup).
inline bool refreshAiGroupSearch(AiGroupSearchState &search,uint32_t &flags,
                                const std::vector<AiGroupSearchMember> &members,uint32_t now) {
    std::array<float,3> sum{};
    size_t count=0;
    bool idle=true;
    for(const auto &member:members) {
        if(!member.eligible)continue;
        for(size_t axis=0;axis<3;++axis) {
            if(!std::isfinite(member.position[axis]))return false;
            sum[axis]=static_cast<float>(sum[axis]+member.position[axis]);
        }
        ++count;idle=idle && member.idle;
    }
    if(!count)return false;
    for(size_t axis=0;axis<3;++axis)
        search.centroid[axis]=static_cast<float>(static_cast<double>(sum[axis])/count);
    if(idle)flags|=1u;
    else {flags&=~1u;search.lastBusyTime=now;}
    return true;
}

enum class AiGroupSearchStart { Waiting, Started };
// Call only after original-compatible group admission and an exhausted target
// search, not after a suspended/unsupported search. 5a3c40 clamps signed-negative
// elapsed time to zero; this differs from the unsigned state18 timeout below.
// Dispatch observes state18 and the old initialization flag, matching original.
template<class Dispatch>
AiGroupSearchStart beginAiGroupSearch(AiGroupSearchState &search,uint32_t &flags,
                                      int &phase,bool &initialized,uint32_t now,Dispatch dispatch) {
    const uint32_t elapsed=now-search.lastBusyTime;
    if((elapsed&0x80000000u) || elapsed<5000u) {
        phase=1;
        return AiGroupSearchStart::Waiting;
    }
    if(flags&32u)search.origin=search.centroid;
    phase=18;search.started=now;
    dispatch();
    initialized=true;
    // Original clears target/prototype and copies centroid into attack destination
    // after dispatch; those fields remain with the caller, not this search state.
    return AiGroupSearchStart::Started;
}

// Original59b436: two ordered CRT rand draws and58ec60 rectangular map clamp.
// RNG callback must supply original nonnegative CRT values (0..32767). It is
// supplied by the caller so this helper cannot change player/world RNG ownership.
template<class Random>
std::optional<std::array<int32_t,2>> aiGroupSearchPoint(
    const AiGroupSearchState &search,int32_t width,int32_t height,Random random) {
    if(width<=0 || height<=0)return std::nullopt;
    const double x=std::trunc(double(search.origin[0])),y=std::trunc(double(search.origin[2]));
    if(!std::isfinite(x) || !std::isfinite(y) ||
       x<double(std::numeric_limits<int32_t>::min()) || x>double(std::numeric_limits<int32_t>::max()) ||
       y<double(std::numeric_limits<int32_t>::min()) || y>double(std::numeric_limits<int32_t>::max()))return std::nullopt;
    const uint32_t rx=random(),ry=random();
    if(rx>32767u || ry>32767u)return std::nullopt;
    // Low32 addition is intentional: original LEA wraps before signed clamp.
    const auto coordinate=[](int32_t origin,uint32_t draw,int32_t size) {
        const uint32_t bits=uint32_t(origin)+draw%40u-20u;
        const int64_t signedValue=bits<=0x7fffffffu?int64_t(bits):int64_t(bits)-0x100000000LL;
        return int32_t(std::clamp<int64_t>(signedValue,0,int64_t(size)-1));
    };
    return std::array<int32_t,2>{coordinate(int32_t(x),rx,width),coordinate(int32_t(y),ry,height)};
}

// Original5a1d70. Stop sees state1 and the previous flag5; clear that flag only
// after dispatch. Return true only when state18 actually releases for replanning.
template<class Stop>
bool advanceAiGroupSearch(const AiGroupSearchState &search,uint32_t &flags,
                           int &phase,uint32_t now,Stop stop) {
    if(phase!=18)return false;
    const uint32_t elapsed=now-search.started;
    if(!((flags&1u) && elapsed>5000u) && elapsed<=25000u)return false;
    phase=1;stop();flags&=~32u;
    return true;
}
} // namespace swgb
