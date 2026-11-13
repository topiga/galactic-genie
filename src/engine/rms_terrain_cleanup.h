// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace swgb {
// Original 0x4e6040, after terrain-1 smoothing: convert water-adjacent tiles
// to shore using stored IDs. No RNG, elevation, flags or object side effects.
// Failure leaves terrain unchanged.
bool cleanupRmsTerrain(uint32_t width, uint32_t height,
    std::vector<uint8_t> &terrain, std::string *err = nullptr);
} // namespace swgb
