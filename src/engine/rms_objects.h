// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace swgb {
namespace dat { struct DatFile; }
// Original initial, full-map, all-eligible candidate list (0x4dded0).
// Entries are y*width+x. This does not test placement or create objects.
// Input is RNG state at candidate construction, not a map seed; only success advances it.
bool generateRmsObjectCandidates(uint32_t width,uint32_t height,uint32_t &randomState,
                                 std::vector<uint32_t> &out,std::string *err=nullptr);

// Original all-eligible rectangular queue around a land center; inclusive clipped bounds.
// Any negative center/radius selects the full map. Otherwise center must be inside
// the map and radius<=1024. Inner exclusion, eligibility, ownership and placement
// checks belong to later stages and are deliberately not performed here.
bool generateRmsObjectCandidatesInArea(uint32_t width,uint32_t height,
    int32_t centerX,int32_t centerY,int32_t outerRadius,uint32_t &randomState,
    std::vector<uint32_t> &out,std::string *err=nullptr);

// Historical name; also carries verified ungrouped Gaia deposit placements.
struct RmsTimberPlacement {
    float x, y;
    uint8_t facet;
    float headingRadians=0;
    int16_t unitId=348;
};
// One original ungrouped unit348 row, default modifiers, on a flat map.
// Occupied tiles represent verified centered, half-tile scenery footprints.
// Validates the supported original prototype. Output may contain fewer than
// requested objects when terrain rejects candidates. Only success advances RNG.
bool generateRmsTimberPlacements(uint32_t width, uint32_t height,
    const std::vector<uint8_t> &terrain, const dat::DatFile &data,
    uint32_t requested, uint32_t &randomState,
    std::vector<RmsTimberPlacement> &out, std::string *err=nullptr,
    const std::vector<uint32_t> &occupiedTiles={});
// Same original queue/gate path for348 carbon,66 nova and102 ore; no grouping.
bool generateRmsResourcePlacements(uint32_t width, uint32_t height,
    const std::vector<uint8_t> &terrain, const dat::DatFile &data, int16_t unitId,
    uint32_t requested, uint32_t &randomState,
    std::vector<RmsTimberPlacement> &out, std::string *err=nullptr,
    const std::vector<uint32_t> &occupiedTiles={});
} // namespace swgb
