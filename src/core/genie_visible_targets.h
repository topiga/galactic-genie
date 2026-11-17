// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <array>
#include <cstdint>
#include <cstddef>
#include <unordered_map>
#include <vector>

namespace swgb {
// Original614840 ordering; IDs replace relocatable record pointers.
class GenieVisibleTargets {
    struct Entry { int cls; uint16_t mask; std::array<size_t,16> slots{}; };
    std::unordered_map<uint32_t,Entry> entries_;
    std::array<std::unordered_map<int,std::vector<uint32_t>>,16> buckets_;
public:
    void clear() { entries_.clear();for(auto &viewer:buckets_)viewer.clear(); }
    void update(uint32_t id,int cls,uint16_t mask) {
        auto found=entries_.find(id);
        if(found!=entries_.end() && found->second.cls!=cls) { update(id,found->second.cls,0);found=entries_.end(); }
        if(found==entries_.end()) {
            if(!mask)return;
            found=entries_.emplace(id,Entry{cls,0,{}}).first;
        }
        auto &entry=found->second;
        const uint16_t changed=entry.mask^mask;
        for(size_t viewer=0;viewer<16;++viewer) {
            const uint16_t bit=uint16_t(1u<<viewer);
            if(!(changed&bit))continue;
            auto &bucket=buckets_[viewer][cls];
            if(mask&bit) { entry.slots[viewer]=bucket.size();bucket.push_back(id); }
            else {
                const size_t slot=entry.slots[viewer];
                const uint32_t moved=bucket.back();
                bucket[slot]=moved;bucket.pop_back();
                if(moved!=id)entries_.at(moved).slots[viewer]=slot;
            }
        }
        entry.mask=mask;
        if(!mask)entries_.erase(found);
    }
    const std::vector<uint32_t>& bucket(size_t viewer,int cls) const {
        static const std::vector<uint32_t> empty;
        if(viewer>=16)return empty;
        const auto found=buckets_[viewer].find(cls);
        return found==buckets_[viewer].end()?empty:found->second;
    }
    size_t slot(size_t viewer,uint32_t id) const {
        const auto found=entries_.find(id);
        return viewer<16 && found!=entries_.end() && (found->second.mask&(1u<<viewer))
            ?found->second.slots[viewer]:size_t(-1);
    }
};
} // namespace swgb
