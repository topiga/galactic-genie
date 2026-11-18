// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>
#include <optional>
#include <vector>

namespace swgb {
struct AiBuildPosition {
    int32_t prototype=-1;
    uint8_t status=0,x=0,y=0;
};

// Original InfoAI+1cec/+1cf0: ordered planned positions, not live buildings.
// 58ef40/58f100 retain int32 prototypes; master-ID callers sign-extend int16.
class AiBuildPositions {
public:
    AiBuildPositions():slots_(200) {}
    const std::vector<AiBuildPosition> &slots() const {return slots_;}
    void upsert(int32_t prototype,int x,int y,int status) {
        // Native state handoffs may reuse a moved-from registry. Restore the
        // constructor state; this is host ownership safety, not an AI event.
        if(slots_.empty())slots_.resize(200);
        const auto bx=uint8_t(x),by=uint8_t(y),bs=uint8_t(status);
        size_t empty=slots_.size();
        for(size_t i=0;i<slots_.size();++i) {
            auto &slot=slots_[i];
            if(slot.prototype==prototype && slot.x==bx && slot.y==by) {
                slot.status=bs;return;
            }
            if(empty==slots_.size() && slot.prototype==-1)empty=i;
        }
        if(empty==slots_.size()) {
            // Original initializes new empty slots with the incoming status.
            slots_.resize(slots_.size()*2,AiBuildPosition{-1,bs,0,0});
        }
        slots_[empty]={prototype,bs,bx,by};
    }
    bool updateExisting(int32_t prototype,int x,int y,int status) {
        for(auto &slot:slots_)if(slot.prototype==prototype &&
            slot.x==uint8_t(x) && slot.y==uint8_t(y)) {
            slot.status=uint8_t(status);
            if(slot.status==3) {slot.prototype=-1;slot.x=slot.y=0;}
            return true;
        }
        return false;
    }
    // 58934a/58947e: normal fresh request consumes the first status0 preset.
    std::optional<AiBuildPosition> consumePreset(int32_t prototype) {
        for(auto &slot:slots_)if(slot.prototype==prototype && slot.status==0) {
            slot.status=1;return slot;
        }
        return std::nullopt;
    }
    bool plannedNearby(int32_t prototype,int x,int y,int radius) const {
        const int bx=uint8_t(x),by=uint8_t(y);
        for(const auto &slot:slots_)if(slot.prototype==prototype && slot.status==1 &&
            int64_t(slot.x)-radius<=bx && int64_t(slot.x)+radius>=bx &&
            int64_t(slot.y)-radius<=by && int64_t(slot.y)+radius>=by)return true;
        return false;
    }
    bool rejected(int32_t prototype,int x,int y) const {
        for(const auto &slot:slots_)if(slot.prototype==prototype && slot.status==2 &&
            slot.x==uint8_t(x) && slot.y==uint8_t(y))return true;
        return false;
    }
private:
    std::vector<AiBuildPosition> slots_;
};
} // namespace swgb
