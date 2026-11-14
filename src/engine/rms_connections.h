// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace swgb {
struct RmsConnectionPoint { int x=0,y=0; };
struct RmsConnectionTerrain {
    int width=1, variance=0, replacement=-1;
    float cost=1;
};
using RmsConnectionSettings=std::array<RmsConnectionTerrain,99>;
// Original pair worker0x4e4b40 and route search0x4d6db0, resolved inputs only.
// No script parser, player-land selection, scheduler RNG, terrain refresh or
// final connection border pass. Failure leaves terrain and RNG unchanged.
// A blocked/unreachable pair succeeds without painting or consuming RNG.
bool connectRmsLandPair(uint32_t width,uint32_t height,
    std::vector<uint8_t> &terrain,RmsConnectionPoint start,RmsConnectionPoint end,
    const RmsConnectionSettings &settings,uint32_t &randomState,
    std::string *err=nullptr);
} // namespace swgb
