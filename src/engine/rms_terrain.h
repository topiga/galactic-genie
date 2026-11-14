// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace swgb {
struct RmsTerrainRow {
    uint32_t targetTiles = 0;
    uint8_t terrain = 3;
    uint8_t baseTerrain = 0;
    uint32_t clumps = 1;
    uint32_t spacing = 0;
    uint32_t clumping = 20;
    bool avoidPlayerStarts = false;
    int32_t minHeight = 0, maxHeight = 255;
    bool flatTerrainOnly = false;
};

struct RmsTerrainProtection {
    int32_t x = 0, y = 0;
    uint32_t radius = 13, weight = 20;
};

// Original terrain rows; connectivity-based avoidance is unsupported.
// Empty height/slope arrays mean zero. Nonempty arrays are original tile+6/+4
// bytes, not inferred gradients. Flat-only checks the spacing neighborhood only
// when spacing>0, matching the original; it is not a global flat-map requirement.
// Failure preserves terrain and RNG; terrain smoothing is a separate stage.
bool generateRmsTerrain(uint32_t width, uint32_t height,
    const std::vector<RmsTerrainRow> &rows, uint32_t &randomState,
    std::vector<uint8_t> &terrain, std::string *err = nullptr,
    const std::vector<RmsTerrainProtection> &protection = {},
    const std::vector<uint8_t> &heights = {}, const std::vector<uint8_t> &slopes = {});
} // namespace swgb
