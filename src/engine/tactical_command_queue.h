// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "local_command_queue.h"
#include <cstdint>
#include <vector>

namespace swgb {
// Original 44dda0/44de50 tactical staging and adjacent-payload grouping.
class TacticalCommandQueue {
public:
    struct Order {
        uint32_t actor;int player;float x,y;
        uint16_t command=705;
        int32_t target=-1;
        uint8_t targetPlayer=255;
        float z=-1,range=1;
        uint8_t flagA=1,flagB=0,flagC=1;
    };
    static constexpr size_t capacity=200;
    bool enqueue(Order order) {
        if(orders_.size()==capacity)return false;
        if(order.command==700 || order.command==729)order.range=0;
        orders_.push_back(order);return true;
    }
    size_t size() const {return orders_.size();}
    void reset() {orders_.clear();}
    template<class Apply> void flush(LocalCommandQueue &local,Apply apply) {
        size_t first=0;
        while(first<orders_.size()) {
            size_t end=first+1;
            const auto equal=[](float a,float b){return !(a<b || a>b);}; // x87 C3, including unordered
            const auto matches=[&](const Order &a,const Order &b) {
                return a.player==b.player && a.command==b.command && a.target==b.target &&
                       a.targetPlayer==b.targetPlayer && a.flagA==b.flagA &&
                       a.flagB==b.flagB && a.flagC==b.flagC && equal(a.x,b.x) &&
                       equal(a.y,b.y) && equal(a.z,b.z) && equal(a.range,b.range);
            };
            while(end<orders_.size() && matches(orders_[end],orders_[end-1]))++end;
            std::vector<Order> group(orders_.begin()+first,orders_.begin()+end);
            // A grouped packet carries one shared header followed by actor IDs.
            for(auto &order:group){const auto actor=order.actor;order=orders_[end-1];order.actor=actor;}
            local.enqueue([group=std::move(group),apply]{apply(group);});
            first=end;
        }
        orders_.clear();
    }
private:
    std::vector<Order> orders_;
};
}
