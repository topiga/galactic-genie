// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "../src/core/genie_conversion_action.h"
static void conversionActionChecks() {
    const struct {uint32_t draw;int cls,interfaceKind;uint32_t trait;float resistance,elapsed;int roll,threshold;bool success;} cases[]={
        {41,0,0,0,0.f,5.f,0,25,true},
        {41,0,0,0,0.f,3.99f,0,-1000,false},
        {41,0,0,0,0.f,4.f,0,25,true},
        {41,0,0,0,0.f,10.f,0,1000,true},
        {41,0,0,0,0.f,10.01f,0,1000,true},
        {41,0,2,0,0.f,5.f,0,25,true},
        {41,0,0,2,0.f,5.f,0,25,true},
        {41,0,10,0,0.f,5.f,0,25,true},
        {41,0,11,0,0.f,5.f,0,25,true},
        {41,47,0,0,0.f,5.f,0,25,true},
        {41,0,0,0,2.f,5.f,0,25,true},
        {41,0,0,0,-2.f,5.f,0,25,true},
        {41,18,0,0,0.f,5.f,0,-1000,false},
        {45,0,0,0,0.f,5.f,0,25,true},
        {45,0,0,0,0.f,3.99f,0,-1000,false},
        {45,0,0,0,0.f,4.f,0,25,true},
        {45,0,0,0,0.f,10.f,0,1000,true},
        {45,0,0,0,0.f,10.01f,0,1000,true},
        {45,0,2,0,0.f,5.f,0,25,true},
        {45,0,0,2,0.f,5.f,0,25,true},
        {45,0,10,0,0.f,5.f,0,25,true},
        {45,0,11,0,0.f,5.f,0,25,true},
        {45,47,0,0,0.f,5.f,0,25,true},
        {45,0,0,0,2.f,5.f,0,25,true},
        {45,0,0,0,-2.f,5.f,0,25,true},
        {45,18,0,0,0.f,5.f,0,-1000,false},
        {48,0,0,0,0.f,5.f,0,25,true},
        {48,0,0,0,0.f,3.99f,0,-1000,false},
        {48,0,0,0,0.f,4.f,0,25,true},
        {48,0,0,0,0.f,10.f,0,1000,true},
        {48,0,0,0,0.f,10.01f,0,1000,true},
        {48,0,2,0,0.f,5.f,0,25,true},
        {48,0,0,2,0.f,5.f,0,25,true},
        {48,0,10,0,0.f,5.f,0,25,true},
        {48,0,11,0,0.f,5.f,0,25,true},
        {48,47,0,0,0.f,5.f,0,25,true},
        {48,0,0,0,2.f,5.f,0,25,true},
        {48,0,0,0,-2.f,5.f,0,25,true},
        {48,18,0,0,0.f,5.f,0,-1000,false},
        {71,0,0,0,0.f,5.f,0,25,true},
        {71,0,0,0,0.f,3.99f,0,-1000,false},
        {71,0,0,0,0.f,4.f,0,25,true},
        {71,0,0,0,0.f,10.f,0,1000,true},
        {71,0,0,0,0.f,10.01f,0,1000,true},
        {71,0,2,0,0.f,5.f,0,25,true},
        {71,0,0,2,0.f,5.f,0,25,true},
        {71,0,10,0,0.f,5.f,0,25,true},
        {71,0,11,0,0.f,5.f,0,25,true},
        {71,47,0,0,0.f,5.f,0,25,true},
        {71,0,0,0,2.f,5.f,0,25,true},
        {71,0,0,0,-2.f,5.f,0,25,true},
        {71,18,0,0,0.f,5.f,0,-1000,false},
        {365,0,0,0,0.f,5.f,1,25,true},
        {365,0,0,0,0.f,3.99f,1,-1000,false},
        {365,0,0,0,0.f,4.f,1,25,true},
        {365,0,0,0,0.f,10.f,1,1000,true},
        {365,0,0,0,0.f,10.01f,1,1000,true},
        {365,0,2,0,0.f,5.f,3,25,true},
        {365,0,0,2,0.f,5.f,3,25,true},
        {365,0,10,0,0.f,5.f,3,25,true},
        {365,0,11,0,0.f,5.f,3,25,true},
        {365,47,0,0,0.f,5.f,8,25,true},
        {365,0,0,0,2.f,5.f,2,25,true},
        {365,0,0,0,-2.f,5.f,1,25,true},
        {365,18,0,0,0.f,5.f,1,-1000,false},
        {7584,0,0,0,0.f,5.f,23,25,true},
        {7584,0,0,0,0.f,3.99f,23,-1000,false},
        {7584,0,0,0,0.f,4.f,23,25,true},
        {7584,0,0,0,0.f,10.f,23,1000,true},
        {7584,0,0,0,0.f,10.01f,23,1000,true},
        {7584,0,2,0,0.f,5.f,69,25,false},
        {7584,0,0,2,0.f,5.f,69,25,false},
        {7584,0,10,0,0.f,5.f,69,25,false},
        {7584,0,11,0,0.f,5.f,69,25,false},
        {7584,47,0,0,0.f,5.f,184,25,false},
        {7584,0,0,0,2.f,5.f,46,25,false},
        {7584,0,0,0,-2.f,5.f,23,25,true},
        {7584,18,0,0,0.f,5.f,23,-1000,false},
        {17440,0,0,0,0.f,5.f,53,25,false},
        {17440,0,0,0,0.f,3.99f,53,-1000,false},
        {17440,0,0,0,0.f,4.f,53,25,false},
        {17440,0,0,0,0.f,10.f,53,1000,true},
        {17440,0,0,0,0.f,10.01f,53,1000,true},
        {17440,0,2,0,0.f,5.f,159,25,false},
        {17440,0,0,2,0.f,5.f,159,25,false},
        {17440,0,10,0,0.f,5.f,159,25,false},
        {17440,0,11,0,0.f,5.f,159,25,false},
        {17440,47,0,0,0.f,5.f,424,25,false},
        {17440,0,0,0,2.f,5.f,106,25,false},
        {17440,0,0,0,-2.f,5.f,53,25,false},
        {17440,18,0,0,0.f,5.f,53,-1000,false},
        {35,0,0,0,0.f,5.f,0,25,true},
        {35,0,0,0,0.f,3.99f,0,-1000,false},
        {35,0,0,0,0.f,4.f,0,25,true},
        {35,0,0,0,0.f,10.f,0,1000,true},
        {35,0,0,0,0.f,10.01f,0,1000,true},
        {35,0,2,0,0.f,5.f,0,25,true},
        {35,0,0,2,0.f,5.f,0,25,true},
        {35,0,10,0,0.f,5.f,0,25,true},
        {35,0,11,0,0.f,5.f,0,25,true},
        {35,47,0,0,0.f,5.f,0,25,true},
        {35,0,0,0,2.f,5.f,0,25,true},
        {35,0,0,0,-2.f,5.f,0,25,true},
        {35,18,0,0,0.f,5.f,0,-1000,false},
    };
    for(const auto &c:cases) {
        swgb::GenieConversionRollInput v;
        v.targetClass=c.cls;v.targetInterfaceKind=c.interfaceKind;v.targetTrait=c.trait;
        v.targetResistance77=c.resistance;v.elapsed=c.elapsed;
        v.taskMinimum=4;v.taskMaximum=10;v.unitChance=25;
        v.buildingMinimum180=6;v.buildingMaximum181=20;v.buildingChance182=40;
        const auto result=swgb::genieConversionRoll(c.draw,v);
        check(result && result->roll==c.roll && result->threshold==c.threshold && result->succeeds==c.success,
              "Original conversion104 resistance, timing windows and RNG decision");
    }
    { swgb::GenieConversionClock clock;clock.elapsed=5.f;clock.pulseRemaining=1.f;
      const bool pulse=clock.advancePulse(0.1f);
      check(clock.elapsed==5.099999904632568f && clock.pulseRemaining==0.8999999761581421f && pulse==false,"Original conversion clock exact float stores and single pulse boundary"); }
    { swgb::GenieConversionClock clock;clock.elapsed=3.9f;clock.pulseRemaining=0.1f;
      const bool pulse=clock.advancePulse(0.1f);
      check(clock.elapsed==4.0f && clock.pulseRemaining==0.0f && pulse==true,"Original conversion clock exact float stores and single pulse boundary"); }
    { swgb::GenieConversionClock clock;clock.elapsed=0.f;clock.pulseRemaining=0.f;
      const bool pulse=clock.advancePulse(0.1f);
      check(clock.elapsed==0.10000000149011612f && clock.pulseRemaining==-0.10000000149011612f && pulse==true,"Original conversion clock exact float stores and single pulse boundary"); }
    { swgb::GenieConversionClock clock;clock.elapsed=9.9f;clock.pulseRemaining=0.1f;
      const bool pulse=clock.advancePulse(0.1f);
      check(clock.elapsed==10.0f && clock.pulseRemaining==0.0f && pulse==true,"Original conversion clock exact float stores and single pulse boundary"); }
    { swgb::GenieConversionClock clock;clock.elapsed=9.f;clock.pulseRemaining=0.1f;
      const bool pulse=clock.advancePulse(5.f);
      check(clock.elapsed==14.0f && clock.pulseRemaining==-4.900000095367432f && pulse==true,"Original conversion clock exact float stores and single pulse boundary"); }
    { swgb::GenieConversionClock clock;clock.elapsed=10.f;clock.pulseRemaining=0.f;
      const bool pulse=clock.advancePulse(0.1f);
      check(clock.elapsed==10.100000381469727f && clock.pulseRemaining==-0.10000000149011612f && pulse==true,"Original conversion clock exact float stores and single pulse boundary"); }
    { swgb::GenieConversionClock clock;clock.elapsed=10.f;clock.pulseRemaining=0.f;
      const bool pulse=clock.advancePulse(0.1f);
      check(clock.elapsed==10.100000381469727f && clock.pulseRemaining==-0.10000000149011612f && pulse==true,"Original conversion clock exact float stores and single pulse boundary"); }
    check(swgb::genieConversionCharge(0.f,1.f,0.1f)==0.10000000149011612f,"Original Jedi charge recovery and cap");
    check(swgb::genieConversionCharge(99.9f,2.f,0.1f)==100.0f,"Original Jedi charge recovery and cap");
    check(swgb::genieConversionCharge(101.f,0.f,0.1f)==100.0f,"Original Jedi charge recovery and cap");
    check(swgb::genieConversionCharge(0.f,-1.f,0.1f)==-0.10000000149011612f,"Original Jedi charge recovery and cap");
    check(swgb::genieConversionCharge(50.f,2.f,0.f)==50.0f,"Original Jedi charge recovery and cap");
    check(swgb::genieConversionCharge(50.f,1.f,3.7f)==53.70000076293945f,"Original Jedi charge recovery and cap");
}
