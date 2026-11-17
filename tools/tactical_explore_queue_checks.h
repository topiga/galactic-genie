// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "../src/engine/tactical_command_queue.h"
#include <cmath>
static void tacticalExploreQueueChecks() {
    swgb::TacticalCommandQueue staging;swgb::LocalCommandQueue local;
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
        ++groups;check(batch.size()==2&&!std::signbit(batch[0].x)&&!std::signbit(batch[1].x),
                      "Grouped float-equal destinations share the last packet header");
    });local.drain();check(groups==1,"Original grouping considers signed zero equal");
    local.reset();groups=0;
    staging.enqueue({1,1,1,2});staging.enqueue({2,1,std::nanf(""),2});staging.enqueue({3,1,3,2});
    staging.flush(local,[&](const auto &batch){
        ++groups;check(batch.size()==3 && batch.front().x==3 && batch.back().x==3,
                      "Adjacent unordered comparisons bridge a group whose final header replaces all actor coordinates");
    });local.drain();check(groups==1,"Batch comparisons follow adjacent records, not the first record");
    for(int field=0;field<10;++field) {
        local.reset();staging.reset();groups=0;
        swgb::TacticalCommandQueue::Order first{1,1,10,20},second{2,1,10,20};
        switch(field) {
        case 0:second.command=710;break;case 1:second.target=9;break;
        case 2:second.targetPlayer=58;break;case 3:second.z=2;break;
        case 4:second.range=2;break;case 5:second.flagA=0;break;
        case 6:second.flagB=1;break;case 7:second.flagC=0;break;
        case 8:second.player=2;break;case 9:second.y=21;break;
        }
        staging.enqueue(first);staging.enqueue(second);
        staging.flush(local,apply);local.drain();
        check(groups==2,"Every original tactical header discriminator splits adjacent groups");
    }
    for(uint16_t command:{uint16_t(700),uint16_t(729),uint16_t(710)}) {
        local.reset();groups=0;
        swgb::TacticalCommandQueue::Order a{1,1,10,20},b{2,1,10,20};
        a.command=b.command=command;a.range=1;b.range=9;
        staging.enqueue(a);staging.enqueue(b);
        staging.flush(local,[&](const auto &batch){
            ++groups;if(command!=710)check(batch.front().range==0,"Attack ranges are zeroed before grouping");
        });local.drain();check(groups==(command==710?2:1),"Only commands700 and729 normalize range");
    }
    std::puts("PASS: original tactical staging capacity, adjacent groups, owner flush and local-ring ordering");
}
