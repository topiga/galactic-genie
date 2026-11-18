// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>
#include <algorithm>
#include <cmath>

namespace swgb {
// Original551c60 stores X difference and clipped X as float; Y remains in the FPU.
// Native double arithmetic preserves those stores; extended x87 sqrt is not bit-emulated.
inline double geniePointFootprintDistance(float x,float y,float targetX,float targetY,float radiusX,float radiusY) {
    const float differenceX=float(double(x)-targetX);
    const float edgeX=float(std::abs(double(differenceX))-radiusX);
    const double dx=std::max(0.,double(edgeX));
    const double dy=std::max(0.,std::abs(double(y)-targetY)-radiusY);
    return std::sqrt(dx*dx+dy*dy);
}

// Original4a5da1..4a5e36 point-path branch; caller supplies actual path qualification.
inline bool geniePointPathSpacingArrival(bool resolvedTarget,bool pathData,
                                        uint32_t flags,uint32_t mask,float spacing,double distance) {
    return !resolvedTarget && pathData && !(flags&mask) && spacing>.1f &&
           double(distance)-double(spacing)<double(1.0000000116860974e-7f);
}

struct GenieTacticalMovementAction {
    bool present=false;
    uint8_t state=3,priority=0;
    float destinationX=0,destinationY=0,range=0;
    uint16_t lastNotification=0;
    void cancel(){*this={};}
    // Original4084b0 point724 states. Path preparation/status are provider callbacks;
    // status0 continues,1 fails,2 completes. State changes report on the NEXT update.
    template<class Prepare,class PathStep,class Notify>
    int update(double footprintDistance,Prepare prepare,PathStep pathStep,Notify notify) {
        if(!present)return 0;
        if(state==1){lastNotification=506;notify(506);return 1;}
        if(state==13){lastNotification=505;notify(505);return 3;}
        if(state==3) {
            if(footprintDistance<=range)state=1;
            else state=prepare()?4:13;
        }
        // Entering4 calls virtual28 synchronously through the original state setter.
        if(state==4) {
            const int result=pathStep();
            if(result==1)state=13;
            else if(result==2)state=1;
        }
        return 0;
    }
};
} // namespace swgb
