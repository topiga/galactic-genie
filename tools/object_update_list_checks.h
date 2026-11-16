// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "../src/core/genie_update_list.h"
#include <array>
#include <cstdio>
#include <iterator>
#include <utility>

static void objectUpdateListChecks() {
    // Actual original list controls, not proof of gameplay membership classification:
    // object-update-list-controls-v1-oracle-2026-11-16.json, functions4aee40/4aef50/4aeff0/4af380.
    // Event first=0 update,1 destruction callback.
    using Event=std::pair<int,size_t>;
    struct Case {
        const char *name;
        std::vector<Event> events;
        std::vector<size_t> primary,secondary;
    };
    const Case cases[]={
        {"baseline",{{0,1},{0,2},{0,3}},{1,2,3},{}},
        {"append_reallocate",{{0,1},{0,2},{0,3},{0,4}},{1,2,3,4},{}},
        {"destroy_self",{{0,1},{1,1},{0,3},{0,2}},{3,2},{}},
        {"remove_future",{{0,1},{0,3}},{1,3},{}},
        {"remove_past",{{0,1},{0,2}},{3,2},{}},
        {"remove_self_false",{{0,1},{0,2}},{3,2},{}},
        {"append_destroy_self",{{0,1},{1,1},{0,4},{0,2},{0,3}},{4,2,3},{}},
        {"migrate_self_sleep",{{0,1},{0,2}},{3,2},{1}},
    };
    for(size_t test=0;test<std::size(cases);++test) {
        swgb::GenieUpdateList primary,secondary;
        std::array<size_t,5> slots;slots.fill(swgb::GenieUpdateList::npos);
        std::vector<Event> events;
        const auto append=[&](size_t id){slots[id]=primary.append(id);};
        const auto remove=[&](size_t id){
            const size_t slot=slots[id],moved=primary.remove(slot);
            slots[id]=swgb::GenieUpdateList::npos;
            if(moved!=swgb::GenieUpdateList::npos)slots[moved]=slot;
        };
        append(1);append(2);append(3);
        primary.traverse([&](size_t id){
            events.emplace_back(0,id);
            if(test==1 && id==1)append(4);
            if(test==2 && id==1)return true;
            if(test==3 && id==1)remove(2);
            if(test==4 && id==2)remove(1);
            if(test==5 && id==1)remove(1);
            if(test==6 && id==1){append(4);return true;}
            if(test==7 && id==1){remove(1);secondary.append(1);}
            return false;
        },[&](size_t id){events.emplace_back(1,id);remove(id);});
        std::vector<size_t> actualPrimary,actualSecondary;
        for(size_t i=0;i<primary.size();++i) {
            actualPrimary.push_back(primary[i]);
            check(slots[primary[i]]==i,"Swap removal repairs every surviving payload slot");
        }
        for(size_t i=0;i<secondary.size();++i)actualSecondary.push_back(secondary[i]);
        check(events==cases[test].events && actualPrimary==cases[test].primary &&
              actualSecondary==cases[test].secondary,cases[test].name);
    }
    swgb::GenieUpdateList list;
    check(list.append(0)==0,"Zero is a valid backing-storage payload");
    size_t visited=0;
    list.traverse([&](size_t id){
        check(id==visited,"Live traversal survives vector reallocations with ordered payloads");
        if(id==0)for(size_t added=1;added<=4096;++added)list.append(added);
        ++visited;return false;
    },[](size_t){check(false,"No finalization requested in append control");});
    check(visited==4097 && list.size()==4097,"All callback-appended payloads visit in the same pass");
    check(list.remove(list.size()-1)==swgb::GenieUpdateList::npos,"Tail removal reports no moved payload");
    check(list.remove(0)==4095 && list[0]==4095,"Swap removal reports the exact moved payload");
    list.clear();
    check(list.empty() && list.append(0)==0,"Session clear resets slots while preserving zero payload support");
    std::puts("PASS: eight original update-list traces, swap slots, live append/reallocation and zero payload");
}
