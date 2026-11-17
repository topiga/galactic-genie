// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <array>
#include <cstdint>
#include <cstddef>
#include <unordered_map>
#include <vector>
#include "genie_target_query.h"

namespace swgb {
// Original614840 ordering; IDs replace relocatable record pointers.
class GenieVisibleTargets {
    struct Entry { int cls; uint16_t mask; std::array<size_t,16> slots{}; GenieVisibleRecord record{}; };
    std::unordered_map<uint32_t,Entry> entries_;
    std::array<std::unordered_map<int,std::vector<uint32_t>>,16> buckets_;
public:
    void clear() { entries_.clear();for(auto &viewer:buckets_)viewer.clear(); }
    void update(uint32_t id,int cls,uint16_t mask,uint8_t owner=0,uint8_t x=0,uint8_t y=0,uint8_t radius=0) {
        auto found=entries_.find(id);
        if(found!=entries_.end() && found->second.cls!=cls) { update(id,found->second.cls,0);found=entries_.end(); }
        if(found==entries_.end()) {
            if(!mask)return;
            found=entries_.emplace(id,Entry{cls,0,{}}).first;
        }
        auto &entry=found->second;
        entry.record={id,radius,owner,x,y};
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
    template<size_t N>
    std::array<std::vector<GenieQueryTarget>,5> query(size_t viewer,int x,int y,int radius,
            const int (&classes)[N],const std::array<uint8_t,17> &partitions) const {
        std::array<std::vector<GenieQueryTarget>,5> result;
        for(int cls:classes)for(uint32_t id:bucket(viewer,cls)) {
            const auto &record=entries_.at(id).record;
            if(record.owner>=partitions.size())continue;
            const auto partition=partitions[record.owner];
            if(partition>=result.size() || result[partition].size()>=256)continue;
            const int distance=genieQueryDistance(x,y,radius,record);
            if(distance>=0)result[partition].push_back({id,uint8_t(distance)});
        }
        return result;
    }
    size_t slot(size_t viewer,uint32_t id) const {
        const auto found=entries_.find(id);
        return viewer<16 && found!=entries_.end() && (found->second.mask&(1u<<viewer))
            ?found->second.slots[viewer]:size_t(-1);
    }
};
} // namespace swgb
