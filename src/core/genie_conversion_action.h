// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <optional>

namespace swgb {
// Original action104 timing kernel. Movement, orientation, notifications and
// ownership transfer are caller callbacks, not implied by a successful roll.
struct GenieConversionClock {
    float elapsed=-1, pulseRemaining=0;
    void beginPulse(float reloadTime) {
        pulseRemaining=reloadTime;
        if(elapsed<0)elapsed=0;
    }
    bool advancePulse(float dt) {
        elapsed=float(double(elapsed)+dt);
        pulseRemaining=float(double(pulseRemaining)-dt);
        return pulseRemaining<=0;
    }
};
struct GenieConversionRollInput {
    int targetClass=0, targetInterfaceKind=0;
    uint32_t targetTrait=0;
    float targetResistance77=0;
    float elapsed=0, taskMinimum=0, taskMaximum=0;
    int16_t unitChance=0;
    float sourceMinimum176=0,sourceMaximum177=0;
    float targetMinimum178=0,targetMaximum179=0;
    float buildingMinimum180=0,buildingMaximum181=0,buildingChance182=0;
};
struct GenieConversionRollResult {int roll=0,threshold=0;bool succeeds=false;};
inline std::optional<GenieConversionRollResult> genieConversionRoll(
        uint32_t draw,const GenieConversionRollInput &v) {
    if(draw>32767)return std::nullopt;
    for(float x:{v.targetResistance77,v.elapsed,v.taskMinimum,v.taskMaximum,
                 v.sourceMinimum176,v.sourceMaximum177,v.targetMinimum178,v.targetMaximum179,
                 v.buildingMinimum180,v.buildingMaximum181,v.buildingChance182})
        if(!std::isfinite(x))return std::nullopt;
    int roll=int(draw*100u/32767u);
    double resistance=((v.targetTrait&2) || v.targetInterfaceKind==2 ||
                       v.targetInterfaceKind==10 || v.targetInterfaceKind==11)?3.:0.;
    if(v.targetClass==47)resistance+=8.;
    if(v.targetResistance77!=0)resistance+=v.targetResistance77;
    if(resistance>0) {
        const double adjusted=roll*resistance;
        if(adjusted>INT32_MAX)return std::nullopt;
        roll=int(adjusted);
    }
    const bool building=v.targetClass==18 || v.targetClass==9 || v.targetClass==10;
    if(building && (double(v.buildingChance182)>INT32_MAX || double(v.buildingChance182)<INT32_MIN))
        return std::nullopt;
    int threshold=building?int(v.buildingChance182):v.unitChance;
    const double minimum=double(v.sourceMinimum176)+v.targetMinimum178+
                         (building?v.buildingMinimum180:v.taskMinimum);
    const double maximum=double(v.sourceMaximum177)+v.targetMaximum179+
                         (building?v.buildingMaximum181:v.taskMaximum);
    if(v.elapsed<minimum)threshold=-1000;
    else if(maximum<=v.elapsed)threshold=1000;
    return GenieConversionRollResult{roll,threshold,roll<=threshold};
}
inline float genieConversionCharge(float charge,float rate,float dt) {
    // Original55a789 uses extended intermediate and compares it before cap.
    const double value=double(rate)*dt+charge;
    return value>=100.?100.f:float(value);
}
} // namespace swgb
