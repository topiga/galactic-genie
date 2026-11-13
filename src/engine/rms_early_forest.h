// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "rms_scenery.h"

namespace swgb {
// Bounded original base10 smoothing on an initially empty, flat world.
// paintedTiles is the exact ordered setter stream from smoothRmsTerrain.
// Failure clears output and preserves RNG.
bool generateRmsEarlyForest(uint32_t width, uint32_t height,
    const std::vector<uint32_t> &paintedTiles, const dat::DatFile &data,
    uint32_t &randomState, std::vector<RmsSceneryPlacement> &out, std::string *err=nullptr);
// Final automatic scan after raw terrain changes, with no intervening setters.
// Water smoothing is outside this scope. Supports terrain10 and inactive
// terrain slots only; retained early trees may stand on a changed inactive tile.
// Keeps creation ordinals/tombstones; does not emulate visibility notifications.
bool finishRmsEarlyForest(uint32_t width, uint32_t height,
    const std::vector<uint8_t> &terrain, const dat::DatFile &data,
    const std::vector<RmsSceneryPlacement> &early, uint32_t &randomState,
    std::vector<RmsSceneryPlacement> &out, std::string *err=nullptr);
}
