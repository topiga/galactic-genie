// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "../src/engine/ai_known_categories.h"
static void aiKnownCategoryChecks() {
    using namespace swgb;
    check(aiKnownCategory(70)==16 && aiKnownCategory(109)==8 && aiKnownCategory(12)==5 &&
          aiKnownCategory(999)==-1,"Original building category lookup vectors");
    check(aiKnownCategory(70,58)==34 && aiKnownCategory(70,8)==35 &&
          aiKnownCategory(70,50)==33 && aiKnownCategory(70,51)==33,
          "Observed class overrides precede prototype lookup");
    AiKnownCategoryMemory memory;
    check(memory.count(2,70)==0 && memory.count(2,999)==-1 && memory.count(0,70)==-1,
          "Known count distinguishes empty category from unsupported category/player");
    for(int id=0;id<10;++id)memory.observe(1,id,2,70,3);
    memory.observe(1,100,1,70,3);
    check(memory.count(2,70)==10 && memory.count(1,70)==0,
          "New foreign records increment once while own admission does not");
    memory.observe(1,0,2,70,3);
    check(memory.count(2,70)==10,"Repeated observation does not increment category count");
    memory.observe(1,0,2,109,18);
    check(memory.count(2,70)==9 && memory.count(2,109)==1,
          "Original existing-record type change transfers one category credit");
    memory.observe(1,1,3,70,3);
    check(memory.count(2,70)==9 && memory.count(3,70)==0,
          "Same-category ownership change preserves original counter quirk");
    memory.observe(1,1,3,109,18);
    check(memory.count(2,70)==9 && memory.count(3,70)==-1 && memory.count(3,109)==1,
          "Type change uses incoming owner for both counter mutations");
    memory.ownedRemoval(0,-1);
    check(memory.snapshot(0)->owner==2 && memory.count(2,109)==1,
          "Missing object on original owned-removal path leaves record unchanged");
    memory.ownedRemoval(0,3);
    check(memory.snapshot(0)->owner==3 && memory.count(2,109)==1 && memory.count(3,109)==1,
          "Resolved owned removal changes remembered owner without erasing or decrementing");
    memory.observe(1,2,2,999,3);
    check(memory.count(2,70)==8,"Changing to unknown category decrements only known old category");
    AiKnownCategoryMemory refresh;
    refresh.observe(1,10,2,70,3,100);
    refresh.observe(1,11,2,70,3,99);
    refresh.observe(1,12,2,70,3,100);
    refresh.observe(1,13,1,70,3,100);
    const auto lookup=[](int id)->std::optional<AiKnownCategorySnapshot> {
        if(id==10 || id==13)return AiKnownCategorySnapshot{70,3,2};
        if(id==11)return AiKnownCategorySnapshot{109,18,3};
        return std::nullopt;
    };
    refresh.rebuild(1,99,lookup);
    check(refresh.count(2,70)==1 && refresh.count(3,109)==1,
          "Original refresh removes missing known IDs and reads current known-object type/owner");
    refresh.rebuild(1,100,lookup);
    check(refresh.count(2,70)==1 && refresh.count(3,109)==0,
          "Observation cutoff is inclusive and remembered self records stay excluded");
    refresh.lastRefreshTime=0;
    check(!refresh.refreshIfDue(1,300000,lookup) && refresh.refreshIfDue(1,300001,lookup),
          "Known-category refresh uses strict unsigned five-minute interval");
    refresh.lastRefreshTime=100;
    check(refresh.refreshIfDue(1,10,lookup),"Future constructor timer uses original unsigned elapsed arithmetic");
    refresh.lastRefreshTime.reset();
    check(!refresh.refreshIfDue(1,1500000,lookup),"Unknown constructor timer is not silently invented");
    std::puts("PASS: original incremental known-category admission and update vectors");
}
