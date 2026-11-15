// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <array>
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
struct RmsCliffObject {
    int unit=-1, facet=0; float x=0, y=0;
    uint32_t creationOrdinal=0;
};
// Original path + topology/art stage on an empty world, spacing >= 1 coarse cell.
// Original type10 cliff masters264..273 and graphics5256..5265 only. Produces
// surviving objects in creation order; ordinals include replaced pieces.
// No RMS parsing/scheduler, native-world registration or later terrain stages.
// Failure leaves objects and RNG unchanged.
bool generateRmsCliffs(uint32_t size, const std::vector<uint8_t> &terrain,
    const std::vector<uint8_t> &heights, const RmsCliffSettings &settings,
    const std::vector<RmsCliffProtection> &protection, uint32_t &randomState,
    std::vector<RmsCliffObject> &objects, std::string *err=nullptr);
struct RmsCliffArt { int unit=-1, facet=0; float offsetX=0, offsetY=0; };
// Clone Campaigns catalog and selection 0x5caf00/0x5cb1b0, edges E/S/W/N.
// No topology updates, object construction or game events. Unsupported edge
// patterns return false without changing art/RNG; valid patterns use original IDs.
bool selectRmsCliffArt(const std::array<int,4> &edges, uint32_t &randomState,
    RmsCliffArt &art);
} // namespace swgb
