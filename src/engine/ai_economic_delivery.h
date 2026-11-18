// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>
namespace swgb {
// Original5add50. Called after resource credit and before clearing carried load;
// the deposit action supplies its already-rounded/truncated integer amount.
template<class Records>
void creditAiEconomicDelivery(Records &records,int32_t worker,int32_t resource,int32_t amount) {
    for(auto &record:records)if(record.id==worker) {
        if(record.resource==resource) {
            const uint32_t sum=uint32_t(record.completed)+uint32_t(amount);
            record.completed=sum<=uint32_t(INT32_MAX)?int32_t(sum):int32_t(int64_t(sum)-0x100000000LL);
        }
        return;
    }
}
} // namespace swgb
