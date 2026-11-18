// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>

namespace swgb {
// Actual actor virtual30/34 (54b170/54b1d0). These deliberately use the
// current master copy ID separately from its physical prototype ID.
inline bool aiConversionSourceVirtual30(int16_t prototype, int16_t copyId) {
    switch(copyId) {
    case 52:case 180:case 183:case 204:case 232:case 239:case 647:case 652:return true;
    }
    switch(prototype) {
    case 800:case 814:case 824:case 1075:case 1076:return true;
    default:return false;
    }
}
inline bool aiConversionSourceVirtual34(int16_t prototype, int16_t copyId) {
    switch(copyId) {
    case 89:case 115:case 125:case 134:case 136:case 140:case 643:case 645:return true;
    }
    switch(prototype) {
    case 725:case 728:case 731:case 747:case 753:case 761:case 770:case 778:
    case 784:case 792:case 806:case 1077:case 1080:case 1082:case 1084:case 1085:return true;
    default:return false;
    }
}
// Actual controller5b48e0 predicate. Source virtual30/34 remain explicit
// providers; this is not the action's movement, resistance or success roll.
struct AiConversionEligibility {
    bool sourceVirtual30=false, sourceVirtual34=true;
    int16_t sourceClass=50;
    float charge=100;
    bool ignoreCharge=false, targetResolved=true;
    int targetOwner=0;
    int16_t targetClass=0, targetPrototype=-1;
    uint8_t targetType=70, targetImmunity=0;
    float enabledResource67=0, jediResource27=0, advancedResource87=0;
};
inline bool aiConversionEligible(const AiConversionEligibility &v) {
    if(v.sourceVirtual30 || v.sourceClass==51 ||
       (!(v.charge>=100.f) && !v.ignoreCharge) || !v.targetResolved ||
       v.targetOwner==0 || !(v.enabledResource67>0.f))return false;
    switch(v.targetClass) {
    case 1:case 2:case 3:case 4:case 60:case 5:case 44:case 46:
    case 51:case 6:case 8:case 26:case 62:return false;
    }
    switch(v.targetPrototype) {
    case 4:case 977:case 109:case 276:case 82:case 104:case 24:
    case 199:case 50:return false;
    }
    if((v.targetType==70 || v.targetType==80) && v.targetImmunity)return false;
    if((v.targetClass==50 || v.targetClass==64) &&
       (!v.sourceVirtual34 || !(v.jediResource27>0.f)))return false;
    switch(v.targetClass) {
    case 32:case 33:case 34:case 35:case 36:case 59:case 43:case 45:
    case 48:case 63:case 53:case 57:case 18:case 9:case 10:case 11:
    case 12:case 13:case 14:case 15:case 16:case 17:case 37:case 38:
    case 39:case 40:return v.sourceVirtual34 && v.advancedResource87>0.f;
    default:return true;
    }
}
} // namespace swgb
