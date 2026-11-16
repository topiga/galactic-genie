// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cmath>
#include <cstdint>
#include <limits>

namespace swgb {

// Original normal-local branch: 61e902..61e990 and 61ee3e..61ee50.
// Supply absolute monotonic host milliseconds, not a clamped frame delta.
// Replay-forced and synchronized/network admission are outside this helper.
class LocalSimulationClock {
public:
    struct Step {
        // Whether to run the simulation body, including the initial zero step.
        bool admitted = false;
        uint32_t deltaMilliseconds = 0;
    };

    void reset() { lastHost_ = 0; rebase_ = false; }

    // Mirrors world+28. Do not call advance while the session is suspended.
    // The next accepted timestamp rebases without running the simulation body,
    // except that the original initial zero-time step remains admitted.
    void suspend() { rebase_ = true; }

    Step advance(uint32_t hostMs, float speed = 1.f) {
        // Bound conversion explicitly; invalid configuration preserves the clock.
        if (!std::isfinite(speed) || speed < 0.f ||
            double(speed) * 100.0 > double(std::numeric_limits<uint32_t>::max()))
            return {};

        // Zero is the original sentinel, including when the host clock wraps to it.
        const bool first = lastHost_ == 0;
        uint32_t elapsed = first ? 0 : hostMs - lastHost_;
        if (!first && elapsed < 50) return {};
        if (elapsed > 100) elapsed = 100;

        const uint32_t delta = rebase_ ? 0 : uint32_t(double(elapsed) * double(speed));
        lastHost_ = hostMs;
        rebase_ = false;
        return {first || delta != 0, delta};
    }

private:
    uint32_t lastHost_ = 0;
    bool rebase_ = false;
};

} // namespace swgb
