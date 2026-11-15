// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "rms_script.h"
#include "rms_cliffs.h"
#include "rms_objects.h"
#include "rms_scenery.h"
#include "rms_player_starts.h"

namespace swgb {
struct RmsOwnedPlacement {
    float x=0, y=0;
    uint16_t owner=1, civilization=1;
    int16_t unitId=109;
    uint8_t pendingFoundation=255; // Original building+264; consumed by visible rendering.
    uint8_t facet=0, animationFrame=0;
    float initialIdleDelay=0;
};
struct RmsMapLand {
    RmsPlayerStart center;
    int id=0,owner=0;
};
struct RmsMap {
    uint32_t width = 0, height = 0;
    std::vector<uint8_t> terrain;
    std::vector<uint8_t> elevation, slopes; // Original tile+6/+4 bytes; empty without an elevation stage.
    std::vector<RmsCliffObject> cliffs; // Before terrain/scenery; creation ordinals retain replacement gaps.
    std::vector<RmsTimberPlacement> objects;
    std::vector<RmsSceneryPlacement> scenery; // Creation order, including removed scenery.
    std::vector<RmsMapLand> lands; // Original source order, land ID and owner preserved.
    std::vector<RmsPlayerStart> playerStarts; // Owner order1..playerCount; no starting units implied.
    std::vector<RmsTimberPlacement> resourcesAfterOwned; // Verified stock resource prefix, after starting units.
    std::vector<RmsOwnedPlacement> ownedObjects; // Starting buildings and units, in creation order.
};

// Strict script pipeline: explicit/implicit neutral lands and at most one player-land block (1..8 players)
// with bounded land separation, elevation, terrain clumps, automatic scenery and one optional Gaia348/66/102 row.
// Player lands default to randomized positions; FIXED_POSITIONS selects original
// mutually-enemy, equal-radius starts. Both modes require default player-land borders.
// The RMS random_placement directive itself does not select this game setting.
// Elevation executes before terrain, including late/repeated/empty sections.
// Terrain rows use original height/slope filters. Post-elevation411/413 scenery
// is supported. A bounded120-square Empire Forest21 profile supports elevated
// CC/worker/scout startup, with terrain connectivity and final placement gates.
// Player/team-land connections execute after terrain. Optional diplomacy includes
// Gaia row/column0; allies currently require randomized starts and no owned rows.
// Cliffs support bounded120-square DIRT1 profiles: one explicit zero-growth
// neutral land with verified elevation/terrain combinations, or two randomized
// player lands (radius<=10, target<=8000) with grass/forest18/20/21 painting and optional
// start-area avoidance. Fixed positions support grass/forest21. No connections
// or owned objects in these cliff profiles.
// Rejects other stages/commands; not a complete packaged random-map generator.
// Also supports one to eight owned109 starts on bare grass0 with explicit
// civilizations and zero placement distances. Enemy foundation remains deferred.
// One to eight owners may append83 workers with count1 (also the default),
// diagnostic distances1..3 or packaged54103 distances6..6; areas may clip.
// A bounded dry forest profile retains live411 trees, including inside forced CC footprints.
// A leading Gaia573 row supports verified destructive starting-scenery cleanup.
// One or two owners may append the civilization-matched550 scout row at distances7..9.
// Two civilization1 owners on32-square grass/dry forest may append the exact
// tight near-Nova66 row (7 objects, radius3, distances12..16, spacing/zone7).
// Two civilization1 randomized starts may instead append the stock holocron285
// row on64 grass or120 grass/dry forest. The120 profile may continue with stock
// forage, near/medium/far Nova, near/far ore and two Nerf rows, in that order.
// The Forest resource/Nerf prefix may end with verified grouped/ungrouped822/1249,
// or either large hunt species with distant loose3/variance1 and spacing5.
// Forest21 also supports a terminal small-hunt1363/1365 row with count4/8.
// Flat profiles may append the verified FOODC3 hunt variant; Forest21 supports
// the resource/Nerf prefix with fixed or randomized enemy starts. The selected
// Forest sequence may continue with ungrouped predators126/812/1197, two global deposit rows
// and the two near-start tree348/349/413 rows, then birds, global predators, Eopie herds,
// an empty wave row, global trees and shore fish: all23 selected Forest54204 rows.
// Full terrain22 startup remains guarded; the aquatic helper has a positive wave fixture.
// RNG is the shared CRT state at script entry. Failure clears output and leaves RNG unchanged.
bool generateRmsMap(const RmsScript &script, const std::unordered_set<std::string> &defines,
                    uint32_t width, uint32_t height, const dat::DatFile &data,
                    uint32_t &randomState, RmsMap &out, std::string *err = nullptr, uint32_t playerCount = 2,
                    const std::vector<uint16_t> &playerCivilizations = {},
                    const std::vector<std::vector<uint8_t>> &diplomacy = {});
} // namespace swgb
