// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace swgb {
namespace dat { struct DatFile; }
struct RmsSceneryPlacement {
    int16_t unitId = -1;
    float x = 0, y = 0;
    uint8_t facet = 0;
    float headingRadians = 0; // Original type30 heading; static tree heading remains zero.
};

// Original automatic scenery pass on empty flat terrain, before object rows.
// Supports inactive slots and verified centered density1000 static trees on
// terrains10/13/17/18/19/20/21; aliases retain their source scenery slots.
// Also supports verified type30 carbon deposits on49/50 with their three-draw appearance.
// Failure clears output and preserves RNG; this pass does not rewrite terrain.
bool generateRmsScenery(uint32_t width, uint32_t height,
    const std::vector<uint8_t> &terrain, const dat::DatFile &data,
    uint32_t &randomState, std::vector<RmsSceneryPlacement> &out,
    std::string *err = nullptr);
} // namespace swgb
