// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <algorithm>
#include <cmath>
#include <limits>

namespace swgb {

struct GenieGatherResult {
    float remaining;
    float carried;
    float credited;
};

// Original regular gathering arithmetic 405591..4056b1, including 5519b0 stores.
// Inputs are stored floats; resolve the effective owner prototype/task beforehand.
// Double intermediates match 93 original PC64 controls, not every possible x87
// extended-precision input. Do not replace them with float intermediates.
inline GenieGatherResult genieGather(float dt, float workRate, float taskFactor,
                                    float multiplier, float remaining,
                                    float carried, float capacity) {
    // Outside the finite normal-data domain, preserve state without a transfer.
    if (!std::isfinite(dt) || !std::isfinite(workRate) ||
        !std::isfinite(taskFactor) || !std::isfinite(multiplier) ||
        !std::isfinite(remaining) || !std::isfinite(carried) ||
        !std::isfinite(capacity) || dt < 0.f || workRate < 0.f ||
        taskFactor < 0.f || remaining < 0.f || carried < 0.f ||
        capacity < carried)
        return {remaining, carried, 0.f};

    double removed = std::min(double(remaining), double(dt) * workRate * taskFactor);
    const double factor = multiplier > 0.f ? double(multiplier) : 1.0;
    const double product = removed * factor;
    if (product > double(std::numeric_limits<float>::max()))
        return {remaining, carried, 0.f};
    float credited = float(product);
    const double total = product + double(carried);
    if (total > double(capacity)) {
        const double excess = total - double(capacity);
        credited = float(double(credited) - excess);
        removed -= excess / factor;
    }

    remaining = float(double(remaining) + double(float(-removed)));
    if (remaining > 0.f && remaining < 0.1f) {
        credited = float(double(credited) + double(remaining) + double(0.01f));
        remaining = 0.f;
    }
    // The original does not clamp again after flushing the tiny remainder.
    carried = float(double(carried) + double(credited));
    return {remaining, carried, credited};
}

} // namespace swgb
