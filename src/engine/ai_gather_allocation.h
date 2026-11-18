// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <utility>
namespace swgb {
struct AiGatherAllocation {
    std::array<int32_t,4> desired{}; // food,carbon,metal,nova
    std::array<int,4> order{0,1,2,3};
};
//5a8920 ranks bucket shortages by integer percentage, with stable descending ties.
// The caller still excludes already-tried/satisfied buckets and retries assignment.
inline std::array<int,4> aiGatherUnmetOrder(const std::array<int32_t,4> &assigned,
                                          const std::array<int32_t,4> &desired) {
    std::array<int32_t,4> score{};std::array<int,4> order{0,1,2,3};
    for(size_t i=0;i<4;++i) {
        if(desired[i]<=0) {score[i]=-100;continue;}
        const uint32_t raw=(uint32_t(desired[i])-uint32_t(assigned[i]))*100u;
        const int32_t numerator=raw<=uint32_t(INT32_MAX)?int32_t(raw):int32_t(int64_t(raw)-0x100000000LL);
        score[i]=numerator/desired[i];
    }
    for(int pass=0;pass<3;++pass)for(size_t i=0;i<3;++i)if(score[i]<score[i+1]) {
        std::swap(score[i],score[i+1]);std::swap(order[i],order[i+1]);
    }
    return order;
}
// Original5a801b..5a822d. Inputs are resolved registered workers with valid
// resource buckets, controller target quota+117c, and the two SN vectors.
// Caller supplies registry eligibility and scheduling; this is not an allocator
// for every world worker or a replacement for original assignment callbacks.
inline AiGatherAllocation allocateAiGatherers(int32_t registeredValid,int32_t quota,
        const std::array<int32_t,4> &percentages,const std::array<int32_t,4> &adjustments={}) {
    const auto signed32=[](uint32_t value)->int32_t {
        return value<=uint32_t(INT32_MAX)?int32_t(value):int32_t(int64_t(value)-0x100000000LL);
    };
    const auto rounded=[&](int32_t count,double percentage) {
        // CRT startup PC53, with explicit float32 .01 constant. Do not replace by/100:
        // e.g5*10*.01f+.5 remains below1 and truncates0 under the original precision.
        double value=double(count)*percentage;
        value*=double(.01f);value+=.5;
        if(!std::isfinite(value) || value< -0x1p63 || value>=0x1p63)return int32_t(0);
        return signed32(uint32_t(int64_t(value))); //632bac writes signed64; caller keeps low32.
    };
    AiGatherAllocation result;
    std::array<int32_t,4> effective{};
    uint32_t total=0;
    for(size_t i=0;i<4;++i) {
        effective[i]=signed32(uint32_t(percentages[i])+uint32_t(adjustments[i]));
        result.desired[i]=rounded(registeredValid,effective[i]);total+=uint32_t(result.desired[i]);
    }
    const int32_t deficit=signed32(uint32_t(quota)-total);
    if(deficit>0)for(size_t i=0;i<4;++i)
        result.desired[i]=signed32(uint32_t(result.desired[i])+uint32_t(rounded(deficit,float(effective[i]))));
    // Exact original selection swaps; equal values do not swap directly, but
    // a later larger bucket can reverse earlier equal buckets indirectly.
    for(size_t i=0;i<4;++i)for(size_t j=i;j<4;++j)
        if(result.desired[size_t(result.order[i])]<result.desired[size_t(result.order[j])])
            std::swap(result.order[i],result.order[j]);
    return result;
}
} // namespace swgb
