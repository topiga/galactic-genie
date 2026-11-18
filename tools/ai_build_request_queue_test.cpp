// SPDX-License-Identifier: GPL-3.0-or-later
#include "../src/engine/ai_build_request_queue.h"
#include <cassert>
#include <vector>
#include <iostream>
using swgb::AiBuildRequestQueue;
int main() {
    auto queue=[] {AiBuildRequestQueue q;assert(q.admit(70,1));assert(q.admit(12,2));assert(q.admit(87,3));return q;};
    auto order=[](const auto&q) {std::vector<int> v;for(const auto&r:q.requests())v.push_back(r.sequence);return v;};
    auto yes=[](const auto&){return true;};
    // Literal expected order/attempts from11 executed original577df0 controls.
    {auto q=queue();assert(q.select(3,3,yes,yes,yes)==1);assert((order(q)==std::vector<int>{2,3,1}));assert(q.find(1)->attempts==1);assert(q.total()==3);}
    {auto q=queue();q.markDispatched(1);assert(q.select(3,3,yes,yes,yes)==2);assert((order(q)==std::vector<int>{1,3,2}));assert(q.find(1)->attempts==0);}
    for(int stage=0;stage<3;++stage) {auto q=queue();auto available=[&](const auto&r){return stage!=0||r.sequence!=1;};auto affordable=[&](const auto&r){return stage!=1||r.sequence!=1;};auto suitable=[&](const auto&r){return stage!=2||r.sequence!=1;};assert(q.select(3,3,available,affordable,suitable)==2);assert((order(q)==std::vector<int>{1,3,2}));assert(q.find(1)->attempts==1&&q.find(2)->attempts==1);}
    {auto q=queue();assert(!q.select(3,0,[](const auto&r){return r.sequence!=1;},yes,yes));assert(q.find(1)->attempts==1&&q.find(2)->attempts==0);}
    {auto q=queue();assert(!q.select(3,-1,yes,yes,yes));assert(q.find(1)->attempts==0);}
    {auto q=queue();for(int i=1;i<=3;++i)q.find(i)->attempts=3;assert(!q.select(3,3,yes,yes,yes));for(const auto&r:q.requests())assert(r.cooldown&&r.attempts==3);}
    {auto q=queue();for(int i=1;i<=3;++i){q.find(i)->attempts=1;q.find(i)->cooldown=true;}assert(!q.select(3,3,yes,yes,yes));for(const auto&r:q.requests())assert(r.cooldown&&r.attempts==0);assert(q.select(3,3,yes,yes,yes)==1);assert(!q.find(1)->cooldown&&q.find(1)->attempts==1);}
    // Original5782c0 sequence/prototype acknowledgement and first-match gates.
    {auto q=queue();q.markDispatched(1);assert(!q.admit(70,4));assert(!q.acknowledge(99,70,101));assert(q.pending(70)==1);assert(q.failed(1));assert(q.pending(70)==1&&!q.find(1)->inProgress);assert(q.acknowledge(1,999,101));assert(q.pending(70)==0&&q.total()==2);assert(!q.acknowledge(1,70,101));assert(q.admit(70,4));}
    {auto q=queue();assert(!q.acknowledge(-1,68,101));assert(q.acknowledge(-1,70,101));assert((order(q)==std::vector<int>{2,3}));}
    {auto q=queue();q.find(1)->done=true;assert(!q.acknowledge(1,70,101));assert(q.pending(70)==1);}
    std::cout<<"Original ordinary build request queue checks passed\n";
}
