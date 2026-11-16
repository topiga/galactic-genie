// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>
#include "rms_player_starts.h"

namespace swgb {
namespace dat { struct DatFile; }
struct RmsObjectRow;
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
    uint16_t owner=0; // Preserve interleaved owned rows after initial startup.
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
    uint8_t mode=1; // Original0 individual,1 loose or2 tight.
    uint32_t groups=1, perGroup=1, variance=0, radius=3;
};
// Verified Reytha/Arid Kaadu/Eopie override: minimum19, no maximum, counts4/8.
struct RmsResourcePlayerRange { int32_t minimum, maximum; };
// Original global selector-2 uses every land record, including neutral lands.
// No terrain selector, temporary spacing or zone-clearance modifiers here.
struct RmsGlobalGroupPlacement {
    std::vector<RmsPlayerStart> landCenters;
    int32_t minDistance=0, spacing=0;
};
// Explicit objectRow executes finalized Gaia rows for land1/-1/-2; count/range/spacing
// fields replace legacy profiles, with terrain filtering on group centers only.
// Positive owners and civilization remapping remain unsupported.
// Default Gaia modifiers, one grouped row. Shared original queue nodes are preserved.
// Optional starts select the verified near-Nova row: tight7, radius3, one group/player,
// distances12..16, spacing7, zone7 on uniformly passable terrain.
// Also supports forage59 tight3/6/12 on120-square dry/Forest21 maps with two starts:
// distances10..12, spacing6, zone7; occupiedTiles must include retained scenery,
// prior resources and owned footprints. Other forage rows remain unsupported.
// Forest21 rows use original connected-zone and eight-point clearance gates.
// Explicit Medium/Far selects stock Nova4 distances18..26/25..35 on120 maps.
// Ore102 uses radius2: Near tight5/distances14..18, Far tight4/distances20..26.
// Nerf594 uses loose groups: Near2/4/8 once/player at10..12, Far1/2/4 twice/player
// at14..30, radius3 and no spacing; includes original animated startup state.
// Kaadu1363/Eopie1365 support Far loose4/8, variance1, one group/player at14..30;
// their original constructor adds a military timer draw and species animation.
// Large hunt822 supports Near loose2/4 or one mode0 single;1249 supports loose2
// or mode0 singles1/2,
// distance16..22, no variance, clearance3x3 and no outermost tile placements.
// Far large hunt uses loose3/variance1, min35, no maximum, spacing5 and one group/player.
// Their constructors consume seven draws, including three action timers.
// Predators126/812/1197/1250/1129 support Far ungrouped mode0: two single objects/player at30..40,
// radius3, no variance/spacing; no grouped placement or variance draws.
// Trees348/349/350/411/413 support Near2/player at4..5 or Far3/player at6..8, mode0,
// spacing2, radius3, no variance; restriction11 and one static facet draw.
// Global trees348/349/411/413 support100 mode0 placements, player exclusion6 and spacing2.
// Reytha/Arid also support43 global411/350 placements, exclusion8 and no spacing.
// Global predators126/812/1197/1250/1129 support four scaled loose single-object groups (1197 also pairs), exclusion40,
// spacing12; Forest Eopie1365 also supports three objects with variance1.
// Tree occupancy must include all retained mobiles as well as scenery/deposits
// and building footprints; trees block later placements on their tile.
// Bird816 supports the scaled120-square global row: mode0,8 single birds,
// no spacing/variance, one heading draw and frame0, without an extra idle delay.
// Flying birds ignore occupied ground tiles and retain the eligibility mask.
// Herd occupancy includes blocking deposits/trees/buildings only: original Nerfs
// ignore workers/scouts/holocrons/other Nerfs, and do not block later herd groups.
// Aquatic69 supports four scaled single placements with cardinal shore2/35 contact;
// wave1585 supports eight single placements on terrain22, including animation frames.
// Optional rowEligibility preserves original spacing exclusions across rows; an
// empty vector starts all-eligible. Failure preserves it and RNG and clears output.
bool generateRmsGroupedResourcePlacements(uint32_t width, uint32_t height,
    const std::vector<uint8_t> &terrain, const dat::DatFile &data, int16_t unitId,
    const RmsResourceGroups &groups, uint32_t &randomState,
    std::vector<RmsTimberPlacement> &out, std::string *err=nullptr,
    const std::vector<uint32_t> &occupiedTiles={}, size_t maxWork=16000000,
    const std::vector<RmsPlayerStart> &novaPlayerStarts={},
    RmsResourceDistance distance=RmsResourceDistance::Near,
    std::vector<uint8_t> *rowEligibility=nullptr,
    const RmsGlobalGroupPlacement *globalPlacement=nullptr,
    const RmsResourcePlayerRange *playerRange=nullptr,
    const RmsObjectRow *objectRow=nullptr);
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
    const std::vector<uint32_t> &occupiedTiles={},
    const std::vector<uint8_t> *rowEligibility=nullptr);
} // namespace swgb
