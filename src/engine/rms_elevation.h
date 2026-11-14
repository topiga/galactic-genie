// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace swgb {
// One finalized original 24-byte elevation row. create_elevation expands into
// successive levels before this worker; target/clumps are already map-scaled.
struct RmsElevationRow {
    uint32_t targetTiles = 0;
    uint8_t elevation = 1;
    uint32_t clumps = 1, spacing = 2;
    uint8_t baseHeight = 0, baseTerrain = 0;
};
// Original 0x4d9e50 only, before height smoothing and slope classification.
// Square maps preserve the original exclusion rectangle's x-derived y bound.
// Protection is the stage's existing 0..101 mask; rejected growth cells become
// 101 and remain protected for later rows. Failure preserves all three outputs.
// This does not clear initial heights or generate the stage's protection mask.
bool generateRmsElevationRow(uint32_t width, uint32_t height, const RmsElevationRow &row,
    const std::vector<uint8_t> &terrain, uint32_t &randomState,
    std::vector<uint8_t> &heights, std::vector<uint8_t> &protection,
    std::string *err = nullptr);
} // namespace swgb
