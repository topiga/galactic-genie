// SPDX-License-Identifier: GPL-3.0-or-later
#include "engine/ai_group_targets.h"
#include <cstdlib>
#include <iostream>
#include <limits>
using namespace swgb;
static void check(bool value, const char *message) {
    if (!value) { std::cerr << "FAIL: " << message << '\n'; std::exit(1); }
}
int main() {
    // Original tactical-target-group-input-controls-2026-11-18: all members
    // resolve, share region and trait bit2; float stores after each axis sum.
    std::vector<AiGroupMemberInput> members{{1,true,7,0,0,{2,4,6},60,20},
                                          {2,true,7,0,50,{4,8,10},100,5}};
    auto group = aggregateAiTargetGroup(members);
    check(group && group->centroid == std::array<float,3>{3,6,8} && group->region == 7 &&
          group->landMode && !group->allClass50 && group->firstClass50 == 2 && group->minRange == 5,
          "Original centroid, trait and first-converter aggregation");
    members[1].region = 8;check(!aggregateAiTargetGroup(members), "Mixed regions reject aggregate");
    members[1].region = 7;members[1].trait = 2;
    check(!aggregateAiTargetGroup(members), "Mixed trait bit2 rejects aggregate");
    members[1].trait = 0;members[1].resolved = false;
    check(!aggregateAiTargetGroup(members), "Missing member rejects aggregate");
    check(!aggregateAiTargetGroup({}), "Empty group never reaches original centroid divide");

    check(!aiGroupNormalPrioritySupported(104,3) && !aiGroupNormalPrioritySupported(109,8) &&
          aiGroupNormalPrioritySupported(109,3), "Special original priority inputs cannot silently default to Normal");

    // Exact17 original592a02 priority controls. The remembered-missing case
    // maps to markerOne=false, as the original lookup explicitly does.
    struct PriorityCase {int prototype,cls,mode,flag114,flag109,marker,sn106,sn107,sn108;AiTargetPriority expected;};
    const auto normal=AiTargetPriority::Normal,force=AiTargetPriority::Force100,almost=AiTargetPriority::Force99Point9;
    const PriorityCase priorityCases[]={
        {109,18,0,0,0,0,0,0,0,normal},{109,46,0,0,0,0,0,0,0,normal},
        {109,46,1,0,0,0,0,0,0,force},{109,46,2,0,0,0,0,0,0,normal},
        {109,8,0,0,0,0,0,0,0,normal},{109,8,0,0,0,0,0,1,0,force},
        {109,8,0,0,0,0,0,2,0,normal},{104,18,0,0,0,0,1,0,0,force},
        {276,18,0,0,0,0,0,0,1,force},{276,18,0,0,1,0,0,0,0,force},
        {276,18,0,1,0,0,0,0,0,normal},{109,51,0,1,0,1,0,0,0,force},
        {109,51,0,1,0,0,0,0,0,almost},{109,51,0,1,0,0,0,0,0,almost},
        {104,18,0,1,0,0,1,0,0,almost},{109,18,0,1,1,0,0,0,0,normal},
        {104,46,1,1,0,0,0,0,0,force}};
    for(const auto &c:priorityCases) {
        AiGroupTargetPriorityInputs inputs{c.mode==1,c.flag114==1,c.flag109==1,c.marker==1,
                                           c.sn106,c.sn107,c.sn108};
        check(resolveAiGroupTargetPriority(c.prototype,c.cls,inputs)==c.expected,
              "Original priority override and precedence golden");
    }
    AiGroupTargetPriorityInputs unknown;
    check(resolveAiGroupTargetPriority(109,18,unknown)==normal &&
          !resolveAiGroupTargetPriority(109,46,unknown) && !resolveAiGroupTargetPriority(104,18,unknown),
          "Unknown relevant priority inputs defer; ordinary units need no defaults");
    unknown.terminateMode=true;
    check(resolveAiGroupTargetPriority(104,46,unknown)==force,
          "Original first priority wins without demanding later unknown fields");
    unknown={};unknown.sn107=0;
    check(resolveAiGroupTargetPriority(109,8,unknown)==normal,
          "Class8 requires only its strategic-number input");

    // Saved 27 original candidate-filter controls, standard land subset.
    AiGroupTargetFilter target;target.recordId=77;target.recordOwner=2;target.hp=100;target.visibility=15;
    const auto accepted=[&] {return filterAiGroupTarget(*group,target)==AiGroupTargetAdmission::Accepted;};
    check(accepted(), "Visible ordinary target is admitted");
    target.hp=-1;check(accepted(), "Original prefilter rejects zero HP, not negative HP");
    target.hp=0;check(!accepted(), "Zero HP rejected");target.hp=100;
    target.visibility=0;check(!accepted(), "Hidden mobile rejected");
    target.unitType=80;target.excludedByVirtual300=true;check(accepted(), "Building skips visibility and virtual300");
    target.unitType=70;target.excludedByVirtual300=false;target.visibility=128;
    check(!accepted(), "Fog128 mobile requires fog mode");target.fogMode=1;check(accepted(), "Fog-enabled mobile accepted");
    target.trait=2;check(!accepted(), "Land group excludes trait2 target");target.trait=0;
    target.unitType=45;check(!accepted(), "Land group excludes type45");target.unitType=70;
    target.transportOption=true;target.region=7;check(!accepted(), "Transport request excludes own region");
    target.region=8;check(accepted(), "Transport request includes other region");target.transportOption=false;
    target.unitClass=58;target.worldMode=3;check(!accepted(), "Original world mode excludes class58");
    check(aiGroupThreatInRange(true,true,2,1,4,4) && !aiGroupThreatInRange(true,true,2,.5f,4,4) &&
          !aiGroupThreatInRange(false,true,2,100,1,4), "Threat membership preserves enemy and inclusive distance/HP gates");

    // Actual 592430 metric-builder goldens; virtual damage/reload modeled in original controls.
    std::vector<AiGroupCombatMember> combat{{true,60,{10,2,false},{{6,2,false},{4,2,false}}},
                                           {true,100,{20,4,false},{{6,2,false},{4,2,false}}}};
    auto metric=buildAiGroupTargetMetric(combat,false,100,9.300538063049316f,true,109,3);
    check(metric && metric->killTime==10 && metric->vulnerability==32 && metric->preferredClass &&
          !metric->wallLike && metric->sameRegion, "Original damage, reload and threat aggregate golden");
    const std::vector<AiGroupCombatMember> fractional{
        {true,60,{.25f,.3f,false},{{.7f,.4f,false},{4.1f,3.3f,false}}},
        {true,100,{3.f,2.7f,false},{{.7f,.4f,false},{4.1f,3.3f,false}}}};
    const auto fractionalMetric=buildAiGroupTargetMetric(fractional,false,100,1,true,109,3);
    check(fractionalMetric && fractionalMetric->killTime==51.42857360839844f &&
          fractionalMetric->vulnerability==53.468353271484375f,
          "Original fractional DPS keeps division unrounded until accumulation store");
    check(!buildAiGroupTargetMetric(combat,false,100,1,true,109,8),
          "Legacy callers still defer unresolved special target priorities");
    const auto wallMetric=buildAiGroupTargetMetric(combat,false,100,1,true,109,8,normal);
    check(wallMetric && wallMetric->wallLike && wallMetric->priority==normal,
          "Explicit resolved priority enables class8 without changing original wall flag");
    const auto forcedMetric=buildAiGroupTargetMetric(combat,false,100,1,true,104,18,almost);
    check(forcedMetric && forcedMetric->priority==almost,
          "Original priority is carried into the shared ranker");

    // Five original592430 zero-reload controls: unordered sums select FLT_MAX.
    for (int test=0;test<5;++test) {
        auto zero=combat;
        if(test<2) for(auto &member:zero) {member.attack.reload=0;if(test==1)member.attack.damage=0;}
        else for(auto &member:zero) for(size_t i=0;i<member.threats.size();++i) {
            if(test!=4 || i==0) {member.threats[i].reload=0;if(test>=3)member.threats[i].damage=0;}
        }
        const auto result=buildAiGroupTargetMetric(zero,false,100,1,true,109,3);
        const float maximum=std::numeric_limits<float>::max();
        check(result && result->killTime==(test==0?0.f:test==1?maximum:10.f) &&
              result->vulnerability==(test<2?32.f:test==2?0.f:maximum),
              "Original zero reload preserves infinity DPS and unordered-sum sentinel");
    }
    metric=buildAiGroupTargetMetric(combat,true,100,1,true,70,18);
    check(metric && metric->vulnerability==std::numeric_limits<float>::max() && !metric->preferredClass,
          "No threats uses FLT_MAX and original low-priority prototype filter");
    combat[0].threats.clear();metric=buildAiGroupTargetMetric(combat,false,100,1,true,109,3);
    check(metric && metric->vulnerability==std::numeric_limits<float>::max(), "One unthreatened member terminates vulnerability sum");
    for(auto &member:combat) member.resolved=false;
    metric=buildAiGroupTargetMetric(combat,false,100,1,true,109,3);
    check(metric && metric->vulnerability==0 && metric->killTime==std::numeric_limits<float>::max(),
          "Nonempty threat list and all unresolved members preserve zero vulnerability");
    combat[0].resolved=true;combat[0].attack={0,1,true};
    check(!buildAiGroupTargetMetric(combat,false,100,1,true,109,3), "Conversion fallback is explicitly unsupported");

    // Original state2 controls: invalid-target reset issues Stop; unseen mobile reset does not.
    AiGroupRetainedTarget retained;retained.hp=100;retained.visibility=15;
    check(retainAiGroupTarget(retained)==AiGroupTargetRetention::Retain,"Retained visible target does not redispatch");
    retained.visibility=0;check(retainAiGroupTarget(retained)==AiGroupTargetRetention::Reset,"Hidden mobile reset without Stop");
    retained.unitType=80;check(retainAiGroupTarget(retained)==AiGroupTargetRetention::Retain,"Hidden building retained");
    retained.memberResolved=false;check(retainAiGroupTarget(retained)==AiGroupTargetRetention::Reset,"Missing first member resets even building");
    retained.hp=.5f;check(retainAiGroupTarget(retained)==AiGroupTargetRetention::ResetAndStop,"Low HP dispatches Stop before first-member lookup");
    retained.hp=1;retained.memberResolved=true;check(retainAiGroupTarget(retained)==AiGroupTargetRetention::Retain,"Exact one HP retained");
    retained.allied=true;check(retainAiGroupTarget(retained)==AiGroupTargetRetention::ResetAndStop,"Allied retained target resets and stops");

    // Original ranked-path controls: first rank attempted despite an expired host budget.
    AiTargetRanking ranking{{100,90,80},{2,0,1}};
    AiGroupTargetContinuation continuation;
    std::vector<size_t> attempted;
    auto reject=[&](size_t i){attempted.push_back(i);return AiGroupTargetAdmission::Rejected;};
    auto selected=selectAiGroupTarget(ranking,continuation,100,5,[]{return 105u;},reject);
    check(selected.status==AiGroupTargetSelectionStatus::Budget && attempted==std::vector<size_t>{2} &&
          continuation.active && continuation.cursor==1,"Original budget retains next ranked position");
    selected=selectAiGroupTarget(ranking,continuation,100,5,[]{return 105u;},
        [](size_t){return AiGroupTargetAdmission::Accepted;});
    check(selected.status==AiGroupTargetSelectionStatus::Selected && selected.index==0 && !continuation.active,
          "Continuation accepts first attempted record and clears cursor");
    selected=selectAiGroupTarget(ranking,continuation,0,5,[]{return 0u;},
        [](size_t){return AiGroupTargetAdmission::Unsupported;});
    check(selected.status==AiGroupTargetSelectionStatus::Unsupported && continuation.active && continuation.cursor==0,
          "Unrecovered blocker fallback is not silently skipped");
    selected=selectAiGroupTarget(ranking,continuation,0,5,[]{return 0u;},reject);
    check(selected.status==AiGroupTargetSelectionStatus::Exhausted && !continuation.active,"Exhaustion clears continuation");
    std::cout << "Original group target helpers passed\n";
}
