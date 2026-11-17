// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>
#include <optional>

namespace swgb {

struct GenieExploreTransition {
    uint8_t controllerStatus = 0;
    bool clearPrimary = false;
    bool requestDestination = false;
    bool rebuildChild = false;
};

// Original action4 owns an action1 child; child completion does not retire this parent.
struct GenieExploreAction {
    uint8_t state = 3;
    float destinationX = -1.f, destinationY = -1.f, destinationZ = -1.f;

    GenieExploreTransition begin(float x, float y, float z) {
        destinationX=x;destinationY=y;destinationZ=z;
        state=x==-1.f?3:11; // 403da0 tests only the original X sentinel.
        return {0,false,state==3,state==11};
    }

    // 445980 keeps an existing action4 unchanged unless replacement is forced.
    GenieExploreTransition retarget(bool force, float x, float y, float z) {
        if (!force) return {};
        if (x>=0.f && y>=0.f) {
            destinationX=x;destinationY=y;destinationZ=z;
            state=11;
            return {0,false,false,true};
        }
        state=3;
        return {0,false,true,false};
    }

    // Raw original child-controller status, not GenieMovementStatus or path state.
    std::optional<GenieExploreTransition> update(uint8_t childStatus) {
        // shortcut: state13 needs actor failure notification; integrate when that callback is ported.
        if (state==13) return std::nullopt;
        if (state==1) return GenieExploreTransition{1,false,false,false};
        if (state!=11) return GenieExploreTransition{};
        if (childStatus==1 || childStatus==3 || childStatus==5) {
            state=3;
            return GenieExploreTransition{0,false,true,false};
        }
        if (childStatus==4) return GenieExploreTransition{0,true,false,true};
        return GenieExploreTransition{};
    }
};

} // namespace swgb
