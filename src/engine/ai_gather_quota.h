// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cmath>
#include <cstdint>
#include <optional>
namespace swgb {
struct AiGatherQuotaInput {
    int32_t civilians=0,militaryExplorers=0,exploredPercent=0,knownEnemyObjects=0;
    int32_t explorerPercent=0,totalExplorers=0,explorerCap=-1,gathererCap=-1;
    int32_t explorationRequired=0,enemyObjectsRequired=0,minimumExplorers=0,halfExplorationRequired=0;
    bool forceGatherers=false;
};
struct AiGatherQuota {int32_t explorers=0,gatherers=0;};
// Original5a0400. KnownEnemyObjects is587480's count for the first enemy ID in
// tactical1400, not food/resource availability. Caller supplies visibility and
// registry callbacks; the original phase2 skips this routine without civilians.
inline std::optional<AiGatherQuota> aiGatherQuota(const AiGatherQuotaInput &in) {
    if(in.civilians<=0)return std::nullopt;
    const auto signed32=[](uint32_t v) {return v<=uint32_t(INT32_MAX)?int32_t(v):int32_t(int64_t(v)-0x100000000LL);};
    const auto add=[&](int32_t a,int32_t b){return signed32(uint32_t(a)+uint32_t(b));};
    const auto sub=[&](int32_t a,int32_t b){return signed32(uint32_t(a)-uint32_t(b));};
    AiGatherQuota out;
    if(in.civilians==1)return AiGatherQuota{1,0};
    if(in.civilians==2) {
        out.gatherers=in.gathererCap!=-1 && in.gathererCap<1?in.gathererCap:1;
        out.explorers=sub(in.civilians,out.gatherers);
        if(in.explorerCap!=-1 && out.explorers>in.explorerCap)out.explorers=in.explorerCap;
        return out;
    }
    const double raw=std::ceil(double(in.civilians)*in.explorerPercent*double(.01f));
    out.explorers=signed32(uint32_t(int64_t(raw)));
    if(add(out.explorers,in.militaryExplorers)>in.totalExplorers)
        out.explorers=sub(in.totalExplorers,in.militaryExplorers);
    if(in.explorerCap!=-1 && out.explorers>in.explorerCap)out.explorers=in.explorerCap;
    if(in.exploredPercent>=in.explorationRequired && in.knownEnemyObjects>in.enemyObjectsRequired) {
        if(out.explorers>in.minimumExplorers)out.explorers=in.minimumExplorers;
    } else if(in.exploredPercent>=in.halfExplorationRequired)
        out.explorers=out.explorers/2>0?out.explorers/2:0;
    if(in.forceGatherers)out.explorers=0;
    out.gatherers=sub(in.civilians,out.explorers);
    if(in.gathererCap!=-1 && out.gatherers>in.gathererCap)out.gatherers=in.gathererCap;
    return out;
}
} // namespace swgb
