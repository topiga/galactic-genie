// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
namespace swgb {
// Original575bb0/575ca0 and57b260/57b590 share this gate. Availability,
// affordability, alias resolution and deferred BuildAI requests belong to callers.
struct AiBuildAttemptLatch {
    bool attempted=false;
    void reset(){attempted=false;}
    static bool excluded(int prototype) {
        return prototype==72 || prototype==117 || prototype==155 || prototype==195 ||
               prototype==487 || prototype==-2;
    }
    bool canCheck(int prototype,bool ownerRestricted) const {
        return !(ownerRestricted && prototype!=109) && !attempted && !excluded(prototype);
    }
    bool begin(int prototype,bool ownerRestricted) {
        if((ownerRestricted && prototype!=109) || attempted)return false;
        attempted=true; // The original sets77fca0 before all remaining failure gates.
        return !excluded(prototype);
    }
};
} // namespace swgb
