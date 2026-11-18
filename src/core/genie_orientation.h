// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace swgb {
// Actor virtual374: 4a11b0/4a1100. Angles and coordinates are in original
// world space. The caller adapts rendering and supplies the current graphic.
struct GenieOrientation {
    float angle=0, lastTurnSeconds=0;
};
struct GenieOrientationResult {bool ready=false, facetChanged=false;};
inline GenieOrientationResult genieOrientToward(GenieOrientation &state,
        float sourceX,float sourceY,float targetX,float targetY,
        int graphicAngleCount,float rotationPeriod,uint32_t worldMilliseconds) {
    constexpr float tau=6.2831854820251465f, pi=3.1415927410125732f;
    constexpr float inverseTau=.15915493667125702f;
    const int count=std::max(8,graphicAngleCount);
    const auto difference=[&]() {
        if(graphicAngleCount<=0)return 0.;
        double angle=std::atan2(double(targetY)-sourceY,double(targetX)-sourceX);
        if(angle<0)angle+=tau;
        angle-=state.angle;
        if(angle>pi)angle-=tau;
        if(angle< -pi)angle+=tau;
        return angle*(double(count)*inverseTau);
    };
    const auto aligned=[](double difference) {return difference>=-.5 && difference<=.5;};
    const double delta=difference();
    if(aligned(delta))return {true,false};
    constexpr double turn=6.2831853;
    if(rotationPeriod==0) {
        double angle=state.angle+delta*(turn/count);
        state.angle=float(angle);
        if(angle<0)state.angle=float(angle+tau);
        else if(angle>tau)state.angle=float(angle-tau);
        return {true,true};
    }
    const double now=double(worldMilliseconds)*.0010000000474974513f;
    if(now-state.lastTurnSeconds<rotationPeriod)return {false,false};
    state.angle=float(state.angle+(delta<0?-1.:1.)*(turn/count));
    if(state.angle<0)state.angle=float(double(state.angle)+tau);
    else if(state.angle>tau)state.angle=float(double(state.angle)-tau);
    state.lastTurnSeconds=float(now);
    return {aligned(difference()),true};
}
} // namespace swgb
