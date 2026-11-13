// SPDX-License-Identifier: GPL-3.0-or-later
#include "rms_player_starts.h"
#include "../core/genie_random.h"
#include <algorithm>

namespace swgb {
bool generateRmsPlayerStarts(uint32_t width, uint32_t height, int32_t radius,
    uint32_t &randomState, std::array<RmsPlayerStart, 2> &out, std::string *err) {
    out = {};
    if (err) err->clear();
    auto fail = [&](const char *message) {
        if (err) *err = std::string("RMS player starts: ") + message;
        return false;
    };
    if (!width || !height || width > 1024 || height > 1024 || radius < 0 ||
        radius > 1024 || 2 * radius >= static_cast<int32_t>(std::min(width, height)))
        return fail("unsupported dimensions or base radius");
    const int32_t innerWidth = static_cast<int32_t>(width) - 2 * radius;
    const int32_t innerHeight = static_cast<int32_t>(height) - 2 * radius;
    const int32_t a = innerWidth * 6 / 10, b = innerHeight * 6 / 10;
    const int32_t margin = innerWidth * 2 / 10, halfPerimeter = a + b;
    if (!halfPerimeter) return fail("player perimeter is empty");
    uint32_t rng = randomState;
    std::array<uint32_t, 8> order{};
    std::array<bool, 9> used{};
    size_t draws=0;
    for (size_t i = 0; i < order.size();) {
        if (++draws>65536) return fail("player-order randomization work limit exceeded");
        const uint32_t owner = std::min(8u, 1u + genieRandomDraw(rng) * 8u / 32767u);
        if (!used[owner]) { used[owner] = true; order[i++] = owner; }
    }
    int32_t position = static_cast<int32_t>(genieRandomDraw(rng) *
        static_cast<uint32_t>(2 * halfPerimeter) / 32767u);
    std::array<int32_t, 2> positions{};
    for (uint32_t owner : order) {
        if (owner > 2) continue;
        positions[owner - 1] = position;
        position += halfPerimeter;
        if (position >= 2 * halfPerimeter) position -= 2 * halfPerimeter;
    }
    std::array<RmsPlayerStart, 2> result{};
    for (size_t i = 0; i < result.size(); ++i) {
        const int32_t p = positions[i];
        auto &v = result[i];
        // The original deliberately uses the width-derived margin on both axes.
        if (p < a) v = {p + margin + radius, margin / 2 + radius};
        else if (p < halfPerimeter) v = {margin / 2 + a + radius, p - a + margin + radius};
        else if (p < halfPerimeter + a)
            v = {halfPerimeter - p + margin + a + radius, margin / 2 + b + radius};
        else v = {margin / 2 + radius, halfPerimeter + a - p + margin + b + radius};
        if (v.x < 0 || v.y < 0 || v.x >= static_cast<int32_t>(width) ||
            v.y >= static_cast<int32_t>(height))
            return fail("original placement falls outside the supported map");
    }
    out = result;
    randomState = rng;
    return true;
}
} // namespace swgb
