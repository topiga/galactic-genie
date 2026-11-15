// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace swgb {
struct RmsCliffSettings {
    int minCount=0, maxCount=0, minLength=0, maxLength=0, curl=0;
    int waterDistance=0, cliffDistance=0;
};
struct RmsCliffProtection { int x=0, y=0, radius=0; };
struct RmsCliffSegment { int x1=0, y1=0, x2=0, y2=0; };
// Original 0x4d76b0 path stage with resolved parameters and inert map callbacks.
// shortcut: inert map callbacks; integrate only after object/RNG effects are verified.
// Failure leaves segments and RNG unchanged. Square maps only.
bool planRmsCliffs(uint32_t size, const std::vector<uint8_t> &terrain,
    const std::vector<uint8_t> &heights, const RmsCliffSettings &settings,
    const std::vector<RmsCliffProtection> &protection, uint32_t &randomState,
    std::vector<RmsCliffSegment> &segments, std::string *err=nullptr);
} // namespace swgb
