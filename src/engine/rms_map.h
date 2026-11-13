// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "rms_script.h"
#include "rms_objects.h"
#include "rms_player_starts.h"

namespace swgb {
struct RmsMap {
    uint32_t width = 0, height = 0;
    std::vector<uint8_t> terrain;
    std::vector<RmsTimberPlacement> timber;
    std::vector<RmsPlayerStart> playerStarts; // Owner order1,2; no starting units implied.
};

// Strict script pipeline: explicit nonplayer lands OR one two-player land block
// on terrain0/3, zero separation, and optionally one default timber348 row.
// Player lands use the game's randomized-position mode, with default borders.
// The RMS random_placement directive itself does not select this game setting.
// Rejects other stages/commands; not a complete packaged random-map generator.
// RNG is the shared CRT state at script entry. Failure clears output and leaves RNG unchanged.
bool generateRmsMap(const RmsScript &script, const std::unordered_set<std::string> &defines,
                    uint32_t width, uint32_t height, const dat::DatFile &data,
                    uint32_t &randomState, RmsMap &out, std::string *err = nullptr);
} // namespace swgb
