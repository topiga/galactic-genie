// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <vector>
#include <cstdint>
#include <string>

namespace swgb {
struct RmsPlayerStart { int32_t x = 0, y = 0; };
// Original 0x4e0740: 1..8 player lands, equal radius, default borders.
// Clone Campaigns delegates here when game randomizePositions is enabled.
// Results are ordered by owner ID (1..playerCount). Input RNG is at finalizer entry;
// this includes its permutation/offset draws, so do not consume them separately.
// Failure clears output and leaves RNG unchanged.
bool generateRmsPlayerStarts(uint32_t width, uint32_t height, int32_t radius, uint32_t playerCount,
    uint32_t &randomState, std::vector<RmsPlayerStart> &out,
    std::string *err = nullptr);
} // namespace swgb
