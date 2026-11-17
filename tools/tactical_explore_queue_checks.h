// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "../src/engine/tactical_explore_queue.h"
#include <cmath>
static void tacticalExploreQueueChecks() {
    swgb::TacticalExploreQueue staging;swgb::LocalCommandQueue local;
    std::vector<uint32_t> seen;int groups=0;
    const auto apply=[&](const auto &batch){++groups;for(const auto &order:batch)seen.push_back(order.actor);};
    check(staging.enqueue({1,1,10,20})&&staging.enqueue({2,1,10,20})&&
          staging.enqueue({3,1,11,20})&&staging.enqueue({4,1,10,20}),"Stage adjacent and nonadjacent exploration orders");
    local.drain();check(seen.empty(),"Tactical staging cannot execute before owner flush");
    staging.flush(local,apply);check(staging.size()==0&&seen.empty(),"Owner flush transfers orders without applying them");
    local.drain();check(seen==std::vector<uint32_t>({1,2,3,4})&&groups==3,
          "Only adjacent matching payloads group, and actors retain their order");
    local.reset();seen.clear();groups=0;
    for(size_t i=0;i<1999;++i)local.enqueue([]{});
    local.drain();
    staging.enqueue({1,1,10,20});staging.enqueue({2,1,10,20});staging.enqueue({3,1,11,20});
    staging.flush(local,apply);local.drain();
    check(seen==std::vector<uint32_t>({3,1,2})&&groups==2,
          "Distinct tactical groups preserve physical local-ring wrap ordering");
    local.reset();seen.clear();groups=0;
    for(uint32_t i=0;i<200;++i)check(staging.enqueue({i+1,1,10,20}),"Original tactical capacity accepts record200");
    check(!staging.enqueue({201,1,10,20})&&staging.size()==200,"Original tactical staging drops record201 without overwriting");
    staging.flush(local,apply);local.drain();
    check(seen.size()==200&&seen.front()==1&&seen.back()==200&&groups==1,"Full equal batch uses one local-ring entry");
    staging.enqueue({9,1,1,2});staging.reset();staging.flush(local,apply);local.drain();
    check(seen.size()==200,"New session discards staged orders");
    seen.clear();groups=0;
    staging.enqueue({1,1,-0.f,2});staging.enqueue({2,1,0.f,2});
    staging.flush(local,[&](const auto &batch){
        ++groups;check(batch.size()==2&&std::signbit(batch[0].x)&&std::signbit(batch[1].x),
                      "Grouped float-equal destinations share the first packet header");
    });local.drain();check(groups==1,"Original grouping considers signed zero equal");
    std::puts("PASS: original tactical staging capacity, adjacent groups, owner flush and local-ring ordering");
}
