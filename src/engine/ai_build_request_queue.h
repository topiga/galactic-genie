// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <algorithm>
#include <cstdint>
#include <optional>
#include <unordered_map>
#include <vector>
namespace swgb {
struct AiBuildRequest {
    int prototype=-1,sequence=-1,actor=-1;
    int attempts=0,completed=0,maximumCompleted=-1;
    bool inProgress=false,done=false,cooldown=false,deferred=false,forward=false;
};
// Ordinary kind0 building requests only:5787b0,577df0,5782c0,578540.
// This does not implement kind5/6 strategic directives, construction suitability,
// timed placement, position blacklists, world command execution or refunds.
class AiBuildRequestQueue {
public:
    int pending(int prototype) const {
        auto i=pending_.find(prototype);return i==pending_.end()?0:i->second;
    }
    int total() const{return total_;}
    const std::vector<AiBuildRequest>& requests() const{return requests_;}
    bool admit(int prototype,int sequence,bool forward=false) {
        if(pending(prototype)>0)return false;
        requests_.push_back({prototype,sequence});requests_.back().forward=forward;
        ++pending_[prototype];++total_;return true;
    }
    // A selected request moves to the tail without releasing its pending count.
    // Callback order: availability, affordability, construction suitability.
    template<class Available,class Affordable,class Suitable>
    std::optional<int> select(int sn114,int sn115,Available available,Affordable affordable,Suitable suitable) {
        if(sn115<0)return std::nullopt;
        int admitted=0;
        for(size_t index=0;index<requests_.size() && admitted<=sn115;++index) {
            auto &r=requests_[index];
            if(r.done || r.inProgress)continue;
            if(r.maximumCompleted!=-1 && r.completed>r.maximumCompleted)continue;
            if((r.cooldown && r.attempts>0) || r.deferred) {
                r.attempts=std::max(0,r.attempts-1);continue;
            }
            const bool resourceBuilding=r.prototype==562 || r.prototype==584 || r.prototype==323 || r.prototype==68;
            if(r.cooldown && r.attempts<=0) {
                r.cooldown=false;if(resourceBuilding)r.attempts=sn114-1;
            }
            ++admitted;++r.attempts;
            if(r.attempts>sn114 && !r.cooldown && !r.deferred && r.prototype!=-1) {
                r.cooldown=true;r.attempts=resourceBuilding?sn114*6:sn114;continue;
            }
            if(!available(r) || !affordable(r) || !suitable(r))continue;
            const int sequence=r.sequence;
            std::rotate(requests_.begin()+index,requests_.begin()+index+1,requests_.end());
            return sequence;
        }
        return std::nullopt;
    }
    AiBuildRequest *find(int sequence) {
        auto i=std::find_if(requests_.begin(),requests_.end(),[&](const auto&r){return r.sequence==sequence;});
        return i==requests_.end()?nullptr:&*i;
    }
    bool markDispatched(int sequence) {
        auto *r=find(sequence);if(!r)return false;r->inProgress=true;r->done=false;return true;
    }
    //578540 clears in-progress; caller records position rejection and releases
    // the worker through original notification adapters. Pending remains live.
    bool failed(int sequence) {
        auto *r=find(sequence);if(!r)return false;r->inProgress=false;return true;
    }
    //5782c0 matches sequence, or first prototype when sequence==-1. Called at
    // the building state0-to2 completion notification, not foundation creation.
    bool acknowledge(int sequence,int prototype,int actor) {
        auto i=std::find_if(requests_.begin(),requests_.end(),[&](const auto&r){
            return !r.done && (sequence==-1?r.prototype==prototype:r.sequence==sequence);});
        if(i==requests_.end())return false;
        i->actor=actor;i->inProgress=false;i->done=true;++i->completed;
        --pending_[i->prototype];--total_;requests_.erase(i);return true;
    }
private:
    std::vector<AiBuildRequest> requests_;
    std::unordered_map<int,int> pending_;
    int total_=0;
};
} // namespace swgb
