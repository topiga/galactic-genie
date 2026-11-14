// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace swgb {
// Original4925d0 over the whole map with ceiling8; accepts heights0..8.
// Failure preserves heights. Does not run object or visual refresh callbacks.
bool smoothRmsHeights(uint32_t width, uint32_t height,
    std::vector<uint8_t> &heights, std::string *err = nullptr);
// Original tile+6 heights to tile+4 slopes; does not smooth the input heights.
// Failure preserves output. Output may alias input.
bool generateRmsSlopes(uint32_t width, uint32_t height,
    const std::vector<uint8_t> &heights, std::vector<uint8_t> &slopes,
    std::string *err = nullptr);
}
