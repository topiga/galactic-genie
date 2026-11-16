// SPDX-License-Identifier: GPL-3.0-or-later
#include "../src/engine/local_simulation_clock.h"
#include <initializer_list>

static void localSimulationClockChecks() {
    using Clock = swgb::LocalSimulationClock;
    // Original instruction goldens: rms-normal-local-admission-oracle-2026-11-16.
    struct Golden { uint32_t elapsed; bool admitted; uint32_t delta; };
    for (const Golden g : {Golden{0,false,0}, {49,false,0}, {50,true,75},
                           {66,true,99}, {100,true,150}, {101,true,150},
                           {5000,true,150}}) {
        Clock clock;
        auto first = clock.advance(1000,1.5f);
        check(first.admitted && first.deltaMilliseconds==0,
              "Local clock admits the original initial zero-time simulation step");
        const auto step = clock.advance(1000+g.elapsed,1.5f);
        check(step.admitted==g.admitted && step.deltaMilliseconds==g.delta,
              "Local clock matches original 50ms admission and 100ms cap");
    }
    Clock clock;
    clock.advance(1000);
    check(!clock.advance(1033).admitted,"Short frame does not advance local simulation");
    auto step=clock.advance(1066);
    check(step.admitted && step.deltaMilliseconds==66,
          "Admission retains actual elapsed time, not a fixed 50ms quantum");
    clock.suspend();
    step=clock.advance(6000);
    check(!step.admitted && step.deltaMilliseconds==0,
          "Resume rebases host time without an AI or simulation step");
    check(!clock.advance(6033).admitted,"Resume timestamp becomes the new interval origin");
    step=clock.advance(6066);
    check(step.admitted && step.deltaMilliseconds==66,"Normal admission resumes after rebase");

    clock.reset(); clock.advance(1000); clock.suspend();
    check(!clock.advance(1033).admitted,"Short resume sample remains pending");
    check(!clock.advance(1050).admitted,"Rebase survives a rejected short sample");
    step=clock.advance(1100,1.5f);
    check(step.admitted && step.deltaMilliseconds==75,"Rebase clears only on accepted timestamp");

    clock.reset(); clock.advance(0xfffffff0u);
    check(!clock.advance(0x10).admitted,"Host timestamp subtraction wraps unsigned");
    step=clock.advance(0x22,1.5f);
    check(step.admitted && step.deltaMilliseconds==75,"Wrapped 50ms interval matches original");
    clock.reset(); clock.advance(0xffffffceu);
    step=clock.advance(0,1.5f);
    check(step.admitted && step.deltaMilliseconds==75,"Host clock can wrap exactly to zero");
    step=clock.advance(33,1.5f);
    check(step.admitted && step.deltaMilliseconds==0,"Preserve original zero timestamp sentinel");

    clock.reset(); clock.suspend();
    step=clock.advance(1000);
    check(step.admitted && step.deltaMilliseconds==0,"Initial sample remains valid after suspension");
    clock.reset(); clock.advance(1000);
    step=clock.advance(1067,1.5f);
    check(step.admitted && step.deltaMilliseconds==100,"Scaled milliseconds truncate, not round");
    clock.reset(); clock.advance(1000);
    step=clock.advance(1050,0.f);
    check(!step.admitted && step.deltaMilliseconds==0,"Zero scaled delta skips noninitial simulation");
    step=clock.advance(1100,1.5f);
    check(step.admitted && step.deltaMilliseconds==75,"Zero-speed sample still accepts its timestamp");

    for (float speed : {-1.f, std::numeric_limits<float>::infinity(),
                        std::numeric_limits<float>::quiet_NaN(),
                        std::numeric_limits<float>::max()}) {
        clock.reset(); clock.advance(1000);
        check(!clock.advance(5000,speed).admitted,"Reject invalid speed without clock mutation");
        step=clock.advance(1050);
        check(step.admitted && step.deltaMilliseconds==50,"Invalid speed preserves prior timestamp");
    }
    clock.reset(); clock.advance(1000,1.5f);
    unsigned admitted=0,total=0;
    for (uint32_t i=1;i<=20;++i) {
        step=clock.advance(1000+33*i,1.5f);
        admitted+=step.admitted; total+=step.deltaMilliseconds;
    }
    check(admitted==10 && total==990,"Original 33ms host sequence produces ten 66ms steps");
}
