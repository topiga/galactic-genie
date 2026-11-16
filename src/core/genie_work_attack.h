// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <algorithm>
#include <cmath>

namespace swgb {

struct GenieWorkAttackConfig {
    int frameCount = 0;
    float frameDuration = 0.f;
    float reloadTime = 0.f;
    unsigned flags = 3;
    float animationSpeed = 0.f;
    float holdTime = 0.f;
};

struct GenieWorkAttackState {
    int actionState = 7;
    int frame = 0;
    int previousFrame = 0;
    float frameRemainder = 0.f;
    float reloadRemaining = 0.f;
    bool pendingHit = true;
    bool frameAdvanced = false;
    bool cycleCompleted = false;
};

struct GenieWorkAttackResult {
    bool supported;
    bool hit;
};

inline bool supportsGenieWorkAttack(const GenieWorkAttackConfig& c) {
    return c.frameCount > 0 && c.frameCount <= 32767 && c.flags == 3 &&
           std::isfinite(c.frameDuration) && c.frameDuration > 0.f &&
           std::isfinite(c.reloadTime) && c.reloadTime >= 0.f &&
           c.animationSpeed == 0.f && c.holdTime == 0.f;
}

// This is the retained oracle's starting boundary, NOT command initialization.
// Ordinary stationary melee only: target stays valid, in range and facing;
// no projectile, master1239 exception, target death, or graphic replacement.
inline bool initializeGenieWorkAttackAtState7(const GenieWorkAttackConfig& c,
                                             GenieWorkAttackState& state) {
    if (!supportsGenieWorkAttack(c)) return false;
    state = {};
    state.reloadRemaining = c.reloadTime;
    return true;
}

// Original 444da0 -> 423ef0 -> 401970, with 401620/444b00 restart order.
// Evidence: tree-felling-release-transition-findings-2026-11-16.json and
// tree-work-attack-dt-matrix-pc64-oracle-2026-11-16.json (150 phases).
// Stored floats and double intermediates match these explicit x87 PC64 controls;
// default Unicorn PC24 gives different remainder bits for other timesteps.
// Unsupported input returns without changing state. The caller owns damage.
inline GenieWorkAttackResult stepGenieWorkAttack(const GenieWorkAttackConfig& c,
                                                 GenieWorkAttackState& state,
                                                 float dt) {
    if (!supportsGenieWorkAttack(c) || !std::isfinite(dt) || dt < 0.f ||
        (state.actionState != 6 && state.actionState != 7) ||
        state.frame < 0 || state.frame >= c.frameCount ||
        state.previousFrame < 0 || state.previousFrame >= c.frameCount ||
        !std::isfinite(state.frameRemainder) || state.frameRemainder < 0.f ||
        !std::isfinite(state.reloadRemaining) || state.reloadRemaining < 0.f)
        return {false, false};

    const float duration = std::max(c.frameDuration, 0.001f);
    const double accumulated = double(state.frameRemainder) + double(dt);
    const double quotient = accumulated / double(duration);
    // The original narrows the quotient to signed16, then advances by its
    // low byte. Reject signed overflow instead of inventing overflow behavior.
    if (quotient >= 32768.0) return {false, false};
    const int steps = int(quotient);
    const int advance = steps & 255;
    if (state.frame + advance > 32767) return {false, false};

    auto next = state;
    next.reloadRemaining = std::max(0.f, float(double(next.reloadRemaining) - dt));
    next.frameAdvanced = false;
    next.cycleCompleted = false;
    next.frameRemainder = float(accumulated);
    if (accumulated >= double(duration)) {
        next.frameAdvanced = true;
        next.previousFrame = next.frame;
        next.frameRemainder = float(accumulated - double(steps) * double(duration));
        next.frame += advance;
        next.cycleCompleted = next.frame >= c.frameCount;
        next.frame %= c.frameCount;
    }

    if (next.actionState == 6) {
        if (next.reloadRemaining <= 0.f) {
            next.reloadRemaining = c.reloadTime;
            next.actionState = 7;
            next.pendingHit = true;
        }
        // State6 dispatch returns even when it just entered state7.
        state = next;
        return {true, false};
    }

    if (!next.pendingHit) {
        if (next.reloadRemaining <= 0.f) {
            next.reloadRemaining = c.reloadTime;
            next.pendingHit = true;
        } else if (next.cycleCompleted) {
            next.actionState = 6;
            state = next;
            return {true, false};
        }
    }
    const int releaseFrame = c.frameCount / 2;
    const bool crossed = next.frameAdvanced &&
        (next.cycleCompleted ||
         (next.frame >= releaseFrame && next.previousFrame < releaseFrame));
    const bool hit = next.pendingHit && (releaseFrame == 0 || crossed);
    if (hit) next.pendingHit = false;
    state = next;
    return {true, hit};
}

} // namespace swgb
