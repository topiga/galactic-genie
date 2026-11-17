// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "../src/core/genie_empty_owner_start.h"
#include <vector>

static void emptyOwnerStartChecks() {
    struct Placement {int unit;float x,y;bool allowed;};
    struct Created {int unit;float x,y;};
    struct Case {uint32_t seed;int workers,width,height;float radius;bool factory;uint32_t finalState;
        std::vector<Placement> checks;std::vector<Created> created;};
    // Original5d1110 vectors: rms-empty-factory-v5-result (actual factories/collision)
    // and placement-controls-double (controlled rejection, no factory draws),18Nov2026.
    const Case cases[]={
        {1u,0,120,120,2.0f,true,11478u,
            {{109,65.0f,15.0f,true}},
            {{109,65.0f,15.0f}}},
        {1u,3,120,120,2.0f,true,1572934298u,
            {{109,65.0f,15.0f,true},{293,62.26721954345703f,13.990264892578125f,true},{293,62.100040435791016f,17.685537338256836f,true},{293,67.63936614990234f,16.183629989624023f,true}},
            {{109,65.0f,15.0f},{293,62.26721954345703f,13.990264892578125f},{293,62.100040435791016f,17.685537338256836f},{293,67.63936614990234f,16.183629989624023f}}},
        {12345u,3,120,120,2.0f,true,598984645u,
            {{109,67.0f,35.0f,true},{83,65.36573028564453f,32.92080307006836f,false},{83,64.8073959350586f,32.37308883666992f,true},{293,65.90069580078125f,37.62483596801758f,true},{293,66.63570404052734f,35.843685150146484f,false},{293,68.53022766113281f,37.08786392211914f,false},{293,64.48225402832031f,35.47691345214844f,true}},
            {{109,67.0f,35.0f},{83,64.8073959350586f,32.37308883666992f},{293,65.90069580078125f,37.62483596801758f},{293,64.48225402832031f,35.47691345214844f}}},
        {4294967295u,3,120,120,2.0f,true,2864392669u,
            {{109,96.0f,15.0f,true},{293,95.78768157958984f,13.636249542236328f,false},{293,94.27277374267578f,17.71724510192871f,true},{83,96.6465072631836f,15.660542488098145f,false},{83,97.54908752441406f,16.29252052307129f,false},{83,97.25702667236328f,17.26618194580078f,true},{83,95.95242309570312f,15.13943862915039f,false},{83,97.88204956054688f,16.891475677490234f,false},{83,93.51802062988281f,14.716238975524902f,true}},
            {{109,96.0f,15.0f},{293,94.27277374267578f,17.71724510192871f},{83,97.25702667236328f,17.26618194580078f},{83,93.51802062988281f,14.716238975524902f}}},
        {12345u,1,120,96,1.5f,false,3856336482u,
            {{109,67.5f,30.5f,false},{109,75.5f,66.5f,false},{109,79.5f,26.5f,true},{293,81.95652770996094f,24.21425437927246f,false},{293,79.79096221923828f,26.16322898864746f,false},{293,81.29576873779297f,25.811777114868164f,true}},
            {{109,79.5f,26.5f},{293,81.29576873779297f,25.811777114868164f}}},
    };
    for(const auto &c:cases) {
        uint32_t state=c.seed;size_t checked=0,created=0;bool completed=false;int countReads=0;
        const bool result=swgb::genieEmptyOwnerStart(state,c.width,c.height,c.radius,c.radius,
            [&](int unit,swgb::GenieStartPosition p) {
                check(checked<c.checks.size(),"Fallback does not add placement checks");
                const auto &wanted=c.checks.at(checked++);
                check(unit==wanted.unit && p.x==wanted.x && p.y==wanted.y,"Original fallback candidate, draw order and double precision");
                return wanted.allowed;
            },
            [&](int unit,swgb::GenieStartPosition p) {
                check(created<c.created.size(),"Fallback creates only captured objects");
                const auto &wanted=c.created.at(created++);
                check(unit==wanted.unit && p.x==wanted.x && p.y==wanted.y,"Original factory order and accepted position");
                check(unit==109 ? !completed:completed,"Completion precedes worker creation");
                if(c.factory)for(int i=0;i<(unit==109?4:7);++i)swgb::genieRandomDraw(state);
                return created;
            },
            [&](size_t handle,swgb::GenieStartPosition p) {
                check(handle==1 && !completed && p.x==c.created[0].x && p.y==c.created[0].y,"Complete the returned CC at its original candidate");
                completed=true;if(c.factory)state=swgb::genieRandomDraw(state);
            },
            [&]() {check(completed,"Read resource84 after completion");++countReads;return c.workers;});
        check(result && completed && countReads==1 && checked==c.checks.size() && created==c.created.size() && state==c.finalState,
              "Original fallback retains rejection retries, factory interleaving and final CRT");
    }
    uint32_t state=1;int checked=0,callbacks=0;
    const bool exhausted=swgb::genieEmptyOwnerStart(state,120,96,2.f,2.f,
        [&](int unit,swgb::GenieStartPosition) {check(unit==109,"Only CC checks before exhaustion");++checked;return false;},
        [&](int,swgb::GenieStartPosition) {++callbacks;return 0;},
        [&](int,swgb::GenieStartPosition) {++callbacks;},[&]() {++callbacks;return 3;});
    check(!exhausted && checked==4999 && callbacks==0 && state==3379307729u,
          "Original failure checks4999 candidates and consumes10000 draws including the final unused pair");
}
