// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>
#include "rms_player_starts.h"

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
    uint8_t animationFrame=0;
    float initialIdleDelay=0;
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
enum class RmsResourceDistance { Near, Medium, Far };
struct RmsResourceGroups {
    uint8_t mode=1; // Original1 loose or2 tight.
    uint32_t groups=1, perGroup=1, variance=0, radius=3;
};
// Default Gaia modifiers, one grouped row. Shared original queue nodes are preserved.
// Optional starts select the verified near-Nova row: tight7, radius3, one group/player,
// distances12..16, spacing7, zone7 on uniformly passable terrain.
// Also supports forage59 tight6/12 on120-square dry/Forest21 maps with two starts:
// distances10..12, spacing6, zone7; occupiedTiles must include retained scenery,
// prior resources and owned footprints. Other forage rows remain unsupported.
// Forest21 rows use original connected-zone and eight-point clearance gates.
// Explicit Medium/Far selects stock Nova4 distances18..26/25..35 on120 maps.
// Ore102 uses radius2: Near tight5/distances14..18, Far tight4/distances20..26.
// Nerf594 uses loose groups: Near4 once/player at10..12, Far2 twice/player
// at14..30, radius3 and no spacing; includes original animated startup state.
// Kaadu1363/Eopie1365 support Far loose4, variance1, one group/player at14..30;
// their original constructor adds a military timer draw and species animation.
// Herd occupancy includes blocking deposits/trees/buildings only: original Nerfs
// ignore workers/scouts/holocrons/other Nerfs, and do not block later herd groups.
// Optional rowEligibility preserves original spacing exclusions across rows; an
// empty vector starts all-eligible. Failure preserves it and RNG and clears output.
bool generateRmsGroupedResourcePlacements(uint32_t width, uint32_t height,
    const std::vector<uint8_t> &terrain, const dat::DatFile &data, int16_t unitId,
    const RmsResourceGroups &groups, uint32_t &randomState,
    std::vector<RmsTimberPlacement> &out, std::string *err=nullptr,
    const std::vector<uint32_t> &occupiedTiles={}, size_t maxWork=16000000,
    const std::vector<RmsPlayerStart> &novaPlayerStarts={},
    RmsResourceDistance distance=RmsResourceDistance::Near,
    std::vector<uint8_t> *rowEligibility=nullptr);
// Original stock holocron285 row: five global placements, player exclusion25,
// temporary spacing20. Bounded to64-square grass or120-square dry/Forest21 terrain
// with two starts;120 may retain centered411/413 trees. Slopes do not restrict it. Owned starting
// objects must remain within9 tiles of starts; other Gaia objects are unsupported.
// This helper does not import holocrons or implement their pickup/gameplay.
// Partial placement when the candidate queue exhausts is successful.
bool generateRmsHolocronPlacements(uint32_t width, uint32_t height,
    const std::vector<uint8_t> &terrain, const dat::DatFile &data,
    const std::vector<RmsPlayerStart> &playerStarts, uint32_t &randomState,
    std::vector<RmsTimberPlacement> &out, std::string *err=nullptr,
    const std::vector<uint32_t> &occupiedTiles={});
} // namespace swgb
