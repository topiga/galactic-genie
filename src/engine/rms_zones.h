// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace swgb {
struct RmsZones {
    std::vector<uint8_t> tiles;
    uint32_t count=0; // Original caps at255; component IDs wrap through0..254.
};
// Original terrain-only zone generation (624ee0), for square maps.
// Both positive and nonpositive passability components receive IDs, row-major.
// Diagonal contact does not connect zones. Adjacency tables/cache lifetime are separate.
// Failure clears output; no RNG used.
bool generateRmsZones(uint32_t size,const std::vector<uint8_t> &terrain,
    const std::vector<float> &passability,RmsZones &out,std::string *err=nullptr);
}
