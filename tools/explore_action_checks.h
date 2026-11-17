// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "../src/core/genie_explore_action.h"
static void exploreActionChecks() {
    for(uint8_t child=0;child<=6;++child) {
        swgb::GenieExploreAction action;
        auto begin=action.begin(10,20,1);
        check(action.state==11&&begin.rebuildChild&&!begin.requestDestination,
              "Known exploration destination creates its movement child");
        auto result=action.update(child);
        const bool request=child==1||child==3||child==5;
        check(result&&result->controllerStatus==0&&action.state==(request?3:11)&&
              result->requestDestination==request&&result->clearPrimary==(child==4)&&
              result->rebuildChild==(child==4),
              "Original exploration child outcomes retain the parent and dispatch the correct callback");
    }
    swgb::GenieExploreAction action;
    auto result=action.begin(-1,20,1);
    check(action.state==3&&result.requestDestination&&!result.rebuildChild,
          "Original unknown X requests an exploration destination");
    action.begin(10,20,1);
    result=action.retarget(false,30,40,2);
    check(action.destinationX==10&&action.destinationY==20&&action.state==11&&
          !result.rebuildChild&&!result.requestDestination,
          "Nonforced existing exploration action is reused unchanged");
    result=action.retarget(true,30,40,2);
    check(action.destinationX==30&&action.destinationY==40&&action.destinationZ==2&&
          action.state==11&&result.rebuildChild,
          "Forced exploration retarget updates the same parent and rebuilds its child");
    for(int axis=0;axis<2;++axis) {
        result=action.retarget(true,axis?30:-1,axis?-1:40,3);
        check(action.state==3&&result.requestDestination&&!result.rebuildChild&&
              action.destinationX==30&&action.destinationY==40&&action.destinationZ==2,
              "Negative retarget coordinate requests a destination without overwriting stored coordinates");
    }
    action.state=1;auto complete=action.update(0);
    check(complete&&complete->controllerStatus==1&&!complete->rebuildChild,
          "Only parent completion returns controller status one");
    action.state=13;
    check(!action.update(0),"Unported actor failure callback remains explicit");
    std::puts("PASS: original exploration parent/child transitions and forced retarget semantics");
}
