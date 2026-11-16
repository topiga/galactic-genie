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
// Original 0x4e76a0 when randomizePositions is disabled. Bounded to mutually
// enemy players (or exactly two mutual allies), one equal-radius land per owner
// and a shared border rectangle. Larger allied groups require the general path.
// Same output order and failure/RNG contract as generateRmsPlayerStarts.
bool generateRmsFixedPlayerStarts(uint32_t width, uint32_t height, int32_t radius, uint32_t playerCount,
    uint32_t &randomState, std::vector<RmsPlayerStart> &out,
    std::string *err = nullptr);
// Original fixed-start aggregation scans owned lands; neutral regions do not constrain starts.
struct RmsPlayerStartRegion { int32_t owner, left, top, right, bottom, radius; };
bool generateRmsFixedPlayerStartsForLands(uint32_t width, uint32_t height,
    const std::vector<RmsPlayerStartRegion> &lands, uint32_t playerCount,
    uint32_t &randomState, std::vector<RmsPlayerStart> &out, std::string *err = nullptr);
} // namespace swgb
