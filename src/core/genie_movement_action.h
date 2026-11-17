// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

namespace swgb {

// Controller outcomes, not the original movement action's state byte.
enum class GenieMovementStatus { Active, Completed, Failed };

// Ordinary point movement only; task-owned movement children are separate slots.
// movement-head-lifecycle-controls-v1-oracle-2026-11-17.json: 409800/406b60.
struct GenieMovementAction {
    bool present = false;
    GenieMovementStatus status = GenieMovementStatus::Active;
    // Native coordinates. Original getter1ac uses mapSize - destinationY.
    float destinationX = -1.f;
    float destinationY = -1.f;

    void begin(float x, float y) {
        present = true;
        status = GenieMovementStatus::Active;
        destinationX = x;
        destinationY = y;
    }

    void complete() {
        if (present) status = GenieMovementStatus::Completed;
    }

    void fail() {
        if (present) status = GenieMovementStatus::Failed;
    }

    void cancel() { *this = {}; }

    // Completion remains attached until the owning controller admits its update.
    // Failed outcomes (original statuses3..5) do not remove the head.
    bool retire() {
        if (!present || status != GenieMovementStatus::Completed) return false;
        cancel();
        return true;
    }
};

} // namespace swgb
