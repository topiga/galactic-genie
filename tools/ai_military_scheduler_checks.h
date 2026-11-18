// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "../src/engine/ai_military_scheduler.h"

static void aiMilitarySchedulerChecks() {
    using namespace swgb;
    {
        //59f194 retains readiness;59f789 advances phase before checking host-budget expiry.
        AiMilitaryPhaseCycle cycle;uint32_t now=0;int refreshed=0,read=0,planned=0;bool complete=false;
        const auto tick=[&] {cycle.advance(2,[&]{return now;},[&]{++refreshed;now+=2;},
            [&](uint32_t,uint32_t){++read;if(complete)now+=2;return complete;},[&]{++planned;});};
        tick();check(cycle.phase==AiMilitaryPhase::Readiness && refreshed==1 && read==0 && planned==0,
            "Expired refresh budget retains readiness without running the next phase");
        tick();check(cycle.phase==AiMilitaryPhase::Readiness && refreshed==1 && read==1 && planned==0,
            "Readiness continuation never refreshes destinations or plans early");
        complete=true;tick();check(cycle.phase==AiMilitaryPhase::Planning && refreshed==1 && planned==0,
            "Completing readiness at the deadline retains planning for the next callback");
        tick();check(cycle.phase==AiMilitaryPhase::Refresh && refreshed==1 && planned==1,
            "Pending planning runs before the next destination refresh");
    }

    const auto fresh=[] {
        return std::vector<AiMilitaryGatherGroup>{{10,1,1,{1}},{20,1,1,{2}},{30,1,1,{3}}};
    };
    auto groups=fresh();
    groups[0].members={-1,1,2};groups[1].destination={9,8,7};groups[2].state=2;
    std::vector<int> lookups;
    refreshAiGatherDestinations(groups,[&](int32_t id)->std::optional<std::array<float,3>> {
        lookups.push_back(id);
        if(id==2)return std::nullopt;
        return std::array<float,3>{1,2,3};
    });
    check(lookups==std::vector<int>{1,2} && groups[0].destination==std::array<float,3>{1,2,3} &&
          groups[1].destination==std::array<float,3>{9,8,7},
          "Original phase7 refresh uses the first nonnegative member; missing lookup preserves destination");

    // Original tactical-readiness-pass-controls-2026-11-18.json, modeled readiness/clock boundaries.
    for(int test=0;test<8;++test) {
        groups=fresh();AiMilitaryReadinessPass pass;
        if(test==2)pass.nextGroupId=20;
        if(test==3)pass.nextGroupId=99;
        if(test==4)groups[0].state=2;
        if(test==5)groups[0].retries=11;
        if(test==6)groups[0].retries=10;
        std::vector<int> readyIds,stopped,gathered;
        int remembered=0,clocks=0;
        const auto result=pass.advance(groups,0,5,[&] {return uint32_t(test==1?5:++clocks);},
            [&](const auto &group) {readyIds.push_back(group.id);return group.id!=10 || (test!=5 && test!=6);},
            [&](auto &group) {gathered.push_back(group.id);},
            [&](int32_t id) {if(test!=7 || id!=1)stopped.push_back(id);},
            [&] {++remembered;});
        if(test==1) {
            check(result==AiMilitaryReadinessResult::BudgetExhausted && pass.nextGroupId==20 &&
                  stopped==std::vector<int>{1} && groups[1].state==1,
                  "Original exact host budget retains the next ordered group ID");
        } else {
            check(result==AiMilitaryReadinessResult::Complete && pass.nextGroupId==-1,
                  "Original completed readiness pass clears its continuation");
            check(groups[0].state==((test==2 || test==6)?1:2) && groups[1].state==2 && groups[2].state==2,
                  "Original readiness continuation and retry branches preserve group states");
        }
        if(test==2 || test==4 || test==5)
            check(std::find(readyIds.begin(),readyIds.end(),10)==readyIds.end(),
                  "Resume, completed group and retry override skip unnecessary readiness callbacks");
        if(test==6)check(gathered==std::vector<int>{10},"Retry ten still gathers without advancing to state2");
        if(test==7)check(stopped==std::vector<int>({2,3}) && remembered==3,
                        "Missing members do not block type100 readiness transition or remembered-count callback");
    }
    // Full original59b160 cases; both explicit x87 PC53/PC64 controls have these outcomes.
    struct GatherCase {double distance;int task,command,spacing,state;bool force,missing;uint8_t retry;int kind,priority;};
    const GatherCase cases[]={
        {4.99,-1,700,4,7,true,false,0,1,0},{5.,-1,700,4,7,true,false,0,2,50},
        {5.01,-1,700,4,7,true,false,0,2,49},{.5,-1,-1,4,7,true,false,0,0,0},
        {0.,-1,700,4,7,true,false,0,1,0},{8.5,600,700,4,7,false,false,0,0,0},
        {8.5,-1,700,4,7,false,false,0,2,26},{8.5,614,700,4,7,false,false,0,2,26},
        {8.5,600,700,4,7,true,false,0,2,26},{8.5,-1,700,4,7,true,true,0,0,0},
        {8.5,-1,700,4,7,true,false,255,2,26},{8.5,-1,700,4,7,true,false,254,2,26},
        {2.,-1,700,4,12,true,false,0,2,99},{999.5,-1,700,4,7,true,false,0,2,0},
        {1.,-1,700,0,7,true,false,0,2,0},{0.,-1,700,-1,7,true,false,0,2,56}};
    for(const auto &c:cases) {
        AiMilitaryGatherGroup group{10,1,1,{33},{0,0,0},c.retry};
        int kind=0,priority=-1;
        const auto lookup=[&](int32_t,const auto &)->std::optional<AiMilitaryGatherMember> {
            if(c.missing)return std::nullopt;
            return AiMilitaryGatherMember{c.distance,c.task,c.command};
        };
        check(dispatchAiMilitaryGather(group,2,c.state,c.spacing,200,c.force,lookup,
            [&](int32_t id){check(id==33,"Original gathering stop keeps member ID");kind=1;},
            [&](const auto &order){
                check(order.actor==33 && order.player==2 && order.command==724 && order.target==-1 &&
                      order.targetPlayer==255 && order.flagA==1 && order.flagB==0 && order.z==0 &&
                      order.range==(c.state==12?1.f:float(c.spacing)),"Original gathering724 envelope preserves clearance and target sentinels");
                kind=2;priority=order.flagC;
            }),"Supported original gather dispatch state");
        check(kind==c.kind && (kind!=2 || priority==c.priority) && group.retries==std::min(255,int(c.retry)+1),
              "Original gathering gates, boundary, priority and saturated retry match saved controls");
    }
    check(aiMilitaryGatherPriority(std::numeric_limits<int32_t>::max(),.5f)==254,
          "Original priority truncation uses the signed low32 bits before the upper cap");
    AiMilitaryGatherGroup ordered{40,1,3,{3,1,2},{7,8,9},0};
    std::vector<int> order;
    const auto resolved=[](int32_t id,const auto &)->std::optional<AiMilitaryGatherMember> {
        return AiMilitaryGatherMember{id==1?5.:2.,-1,700};
    };
    dispatchAiMilitaryGather(ordered,2,7,4,200,false,resolved,
        [&](int32_t id){order.push_back(id);},[&](const auto &move){order.push_back(int(move.actor));});
    check(order==std::vector<int>({3,1,2}),"Gather callbacks retain original member order across Stop and staged724");
    check(aiMilitaryGatherReady(ordered,4,resolved),"Readiness accepts adjusted-distance equality");
    check(!aiMilitaryGatherReady(ordered,3,resolved),"Readiness rejects a resolved member beyond spacing");
    check(aiMilitaryGatherReady(ordered,4,[](int32_t,const auto &)->std::optional<AiMilitaryGatherMember>{return std::nullopt;}),
          "Missing members do not prevent original gathering readiness");
    const auto retry=ordered.retries;
    check(!dispatchAiMilitaryGather(ordered,2,2,4,200,false,resolved,[](int32_t){},[](const auto &){} ) && ordered.retries==retry,
          "Unsupported gather states do not change the group");
    groups=fresh();groups[0].desiredCount=2;
    AiMilitaryReadinessPass pass;int calls=0;
    const auto unsupported=pass.advance(groups,0,5,[&]{++calls;return 0u;},
        [&](const auto &){++calls;return true;},[&](auto &){++calls;},[&](int32_t){++calls;},[&]{++calls;});
    check(unsupported==AiMilitaryReadinessResult::Unsupported && calls==1 && pass.nextGroupId==10 && groups[0].state==1,
          "Unrecovered count-deficit branches are explicit and do not issue approximate commands");
    // Saved original tactical-readiness-deficit-controls-2026-11-18.json:14 executed branches.
    struct DeficitCase {bool ready,missingFirst,missingHome;int retry,own,home,memberMode;};
    const DeficitCase deficits[]={
        {true,false,false,0,1,1,0},{false,false,false,0,1,1,0},
        {true,false,false,0,1,2,0},{false,false,false,0,1,2,0},
        {true,true,false,0,1,1,0},{false,true,false,0,1,1,0},
        {true,false,true,0,1,1,0},{false,false,true,0,1,1,0},
        {false,false,false,11,1,1,0},{false,false,false,11,1,2,0},
        {true,false,false,0,1,2,1},{true,false,false,0,1,1,2},
        {true,false,false,0,257,1,0},{true,false,false,0,1,2,0}};
    const std::vector<std::vector<int>> deficitEvents={
        {1,2,3,4,5,8},{1,2,3,4,5,9,8},{1,2,3,4,5,6,7,8},{1,2,3,4,5,6,7,8},
        {1,2,3,8},{1,2,3,9,8},{1,2,3,8},{1,2,3,9,8},
        {2,3,4,5,8},{2,3,4,5,6,7,8},{1,2,3,4,5,6,7,8},{1,2,3,8},
        {1,2,3,4,5,8},{1,2,3,4,5,6,7,8}};
    for(size_t i=0;i<std::size(deficits);++i) {
        const auto &c=deficits[i];
        groups={{10,1,c.memberMode==1?3:c.memberMode==2?1:2,
                 c.memberMode==1?std::vector<int32_t>{-1,1}:c.memberMode==2?std::vector<int32_t>{}:std::vector<int32_t>{1},
                 {},uint8_t(c.retry)}};
        pass.nextGroupId=-1;std::vector<int> events;
        const auto result=pass.advance(groups,0,5,[&]{events.push_back(8);return 1u;},
            [&](const auto &){events.push_back(1);return c.ready;},
            [&](auto &){events.push_back(9);},
            [&](int32_t id){check(id==1,"Deficit release stops only resolved IDs");events.push_back(6);},
            [&]{events.push_back(7);},
            [&](const auto &group)->std::optional<bool> {
                events.push_back(2); // first413850 lookup
                const auto first=std::find_if(group.members.begin(),group.members.end(),[](int32_t id){return id!=-1;});
                const bool member=first!=group.members.end() && !c.missingFirst;
                events.push_back(3); // home413870 executes even when first lookup failed
                if(!member || c.missingHome)return false;
                events.push_back(4);events.push_back(5);
                return uint8_t(c.own)!=uint8_t(c.home);
            });
        const bool release=i==2 || i==3 || i==9 || i==10 || i==13;
        check(result==AiMilitaryReadinessResult::Complete && pass.nextGroupId==-1 && events==deficitEvents[i],
              "Original deficit readiness/lookup/region/stop/gather/clock order matches saved execution");
        check(groups[0].state==(release?2:1) && groups[0].desiredCount==int(groups[0].members.size())+(release?0:1),
              "Only different regions shrink original desired count; ready same-region deficits continue waiting");
    }
    // Actual5ab090/59acb0 controls, with4506f0 and first-result accessors modeled.
    struct RefillCase {int desired,initialized,state,initialCount;uint32_t flags;std::vector<int32_t> selected;bool nullGroup;};
    const RefillCase refills[]={
        {2,0,1,1,0,{2,3},false},{3,0,1,1,0,{2,3,4,5},false},
        {2,0,1,1,0,{2,3,4},false},{3,0,1,1,0,{2,2,1},false},
        {40,0,1,39,0,{40,41,42},false},{1,0,1,1,0,{2,3},false},
        {3,0,1,1,8,{2,3},false},{3,0,1,1,1,{2,3},false},
        {3,1,1,1,0,{2,3},false},{3,2,1,1,0,{2,3},false},
        {3,0,2,1,0,{2,3},false},{3,0,1,1,0,{},false},
        {3,0,1,1,0,{2,3},true},{3,0,0,1,0,{2,3},false}};
    for(size_t i=0;i<std::size(refills);++i) {
        const auto &c=refills[i];AiMilitaryGatherGroup group{10,1,0,{}};
        group.state=c.state;group.desiredCount=c.desired;
        for(int id=1;id<=c.initialCount;++id)group.members.push_back(id);
        int selections=0;
        check(refillAiMilitaryGroup(group,c.initialized,c.flags,1234,[&](int requested)->std::optional<std::vector<int32_t>> {
            ++selections;check(requested==c.desired,"Original refill asks for desired total, not missing count");
            return c.nullGroup?std::vector<int32_t>{}:c.selected;
        }),"Recovered type100 refill supports recorded selector results");
        const std::vector<std::vector<int32_t>> expected={{1,2,3},{1,2,3,4},{1,2,3},{1,2,2,1},
            {},{1},{1},{1,2,3},{1},{1,2,3},{1,2,3},{1},{1},{1,2,3}};
        if(i==4)check(group.members.size()==40 && group.members.front()==1 && group.members.back()==40,
                      "Original member append stops at capacity40");
        else check(group.members==expected[i],"Original first-result refill caps and lack of append dedup match controls");
        check(group.desiredCount==(i==6?1:c.desired) && group.state==c.state &&
              group.lastMemberAdded==(group.members.size()>size_t(c.initialCount)?1234u:0u),
              "Refill changes desired count only forflag8 and stamps only successful appends");
        check(selections==((i==5 || i==6 || i==8)?0:1),"Full, flag8 and initialized-one gates skip pool selection");
    }
    AiMilitaryGatherGroup unsupportedRefill{10,1,2,{1}};
    check(!refillAiMilitaryGroup(unsupportedRefill,0,0,1234,[](int)->std::optional<std::vector<int32_t>> {return std::nullopt;}) &&
          unsupportedRefill.members==std::vector<int32_t>{1},"Unsupported refill selection cannot invent members");
    groups=fresh();pass.nextGroupId=-1;
    const auto wrapped=pass.advance(groups,0xfffffffeu,3,[]{return 1u;},
        [](const auto &){return true;},[](auto &){},[](int32_t){},[]{});
    check(wrapped==AiMilitaryReadinessResult::BudgetExhausted && pass.nextGroupId==20,
          "Original unsigned host-clock subtraction survives wraparound");
}
