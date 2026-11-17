// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "genie_random.h"
#include <array>
#include <cstdint>

namespace swgb {
struct GenieTroopIdleClock {
    uint32_t elapsed = 0, threshold = 0;
    uint32_t initialOffset = 0, secondaryThreshold = 0;
    void initialize(uint32_t &randomState) {
        elapsed = 0;
        initialOffset = genieRandomDraw(randomState) % 500;
        genieRandomDraw(randomState); // Base 418320 threshold is overwritten by 5b6af0.
        secondaryThreshold = 750 + genieRandomDraw(randomState) % 500;
        threshold = 1125 + genieRandomDraw(randomState) % 750;
    }
    bool advance(uint32_t ownerMilliseconds, bool idle) {
        elapsed += ownerMilliseconds;
        if (!idle || elapsed <= threshold) return false;
        elapsed = 0;
        return true;
    }
};

// World61f759 accumulates before callbacks;61f982 clears the selected slot at the tail.
struct GenieOwnerSchedule {
    uint32_t selected = 0;
    std::array<uint32_t,17> elapsed{};
    void beginOwner(uint32_t owner,uint32_t stepMilliseconds) { elapsed[owner] += stepMilliseconds; }
    uint32_t delta(uint32_t owner) const { return owner == selected ? elapsed[owner] : 0; }
    void finish(uint32_t slotsIncludingGaia) {
        elapsed[selected] = 0;
        if (++selected >= slotsIncludingGaia) selected = 0;
    }
};
}
