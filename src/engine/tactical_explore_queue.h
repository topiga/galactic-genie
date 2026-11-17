// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "local_command_queue.h"
#include <cstdint>
#include <vector>

namespace swgb {
// Original 44dda0/44de50 staging, restricted to the fixed command705 payload.
class TacticalExploreQueue {
public:
    struct Order {uint32_t actor;int player;float x,y;};
    static constexpr size_t capacity=200;
    bool enqueue(Order order) {
        if(orders_.size()==capacity)return false;
        orders_.push_back(order);return true;
    }
    size_t size() const {return orders_.size();}
    void reset() {orders_.clear();}
    template<class Apply> void flush(LocalCommandQueue &local,Apply apply) {
        size_t first=0;
        while(first<orders_.size()) {
            size_t end=first+1;
            const auto equal=[](float a,float b){return !(a<b || a>b);}; // x87 C3, including unordered
            while(end<orders_.size() && orders_[end].player==orders_[first].player &&
                  equal(orders_[end].x,orders_[first].x) && equal(orders_[end].y,orders_[first].y))++end;
            std::vector<Order> group(orders_.begin()+first,orders_.begin()+end);
            // A grouped packet carries one shared header followed by actor IDs.
            for(auto &order:group){order.x=orders_[first].x;order.y=orders_[first].y;}
            local.enqueue([group=std::move(group),apply]{apply(group);});
            first=end;
        }
        orders_.clear();
    }
private:
    std::vector<Order> orders_;
};
}
