// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "rms_script.h"
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
struct RmsMap {
    uint32_t width = 0, height = 0;
    std::vector<uint8_t> terrain;
    std::vector<RmsTimberPlacement> objects;
    std::vector<RmsSceneryPlacement> scenery; // Creation order, including removed early scenery.
    std::vector<RmsPlayerStart> playerStarts; // Owner order1..playerCount; no starting units implied.
    std::vector<RmsOwnedPlacement> ownedObjects; // Completed buildings, in creation order.
};

// Strict script pipeline: explicit nonplayer lands OR one player-land block (1..8 players)
// on verified flat terrain, bounded land separation, terrain clumps, automatic scenery and one optional Gaia348/66/102 row.
// Player lands use the game's randomized-position mode, with default borders.
// The RMS random_placement directive itself does not select this game setting.
// Terrain rows support spacing, scaling and player-start protection on flat maps.
// Rejects other stages/commands; not a complete packaged random-map generator.
// Also supports one to eight owned109 starts separated from the local owner on bare grass0 with explicit
// civilizations and zero placement distances. Enemy foundation remains deferred.
// One or two owners may append83 workers with explicit count1 and distances1..3.
// A bounded two-civ1 dry forest profile retains live411 trees outside CC footprints.
// RNG is the shared CRT state at script entry. Failure clears output and leaves RNG unchanged.
bool generateRmsMap(const RmsScript &script, const std::unordered_set<std::string> &defines,
                    uint32_t width, uint32_t height, const dat::DatFile &data,
                    uint32_t &randomState, RmsMap &out, std::string *err = nullptr, uint32_t playerCount = 2,
                    const std::vector<uint16_t> &playerCivilizations = {});
} // namespace swgb
