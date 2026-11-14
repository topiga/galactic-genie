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
struct RmsConnectionLand { RmsConnectionPoint center; int owner=0; };
struct RmsConnectionRecord {
    std::vector<RmsConnectionPoint> points;
    RmsConnectionSettings settings;
};
struct RmsConnectionCommand {
    int kind=63; // 63 players, 64 teams, 65/66 all lands
    RmsConnectionSettings settings;
};
// Original cross-check selection tail0x4e0505: team ordinals are already resolved
// by diplomacy, indexed by player (slot0 Gaia), contiguous1..teamCount. Team1
// retains the command's original position; other team records append at the end.
// This helper does not resolve diplomacy or choose land locations.
bool resolveRmsConnectionRecords(const std::vector<RmsConnectionCommand> &commands,
    const std::vector<RmsConnectionLand> &lands,const std::vector<int> &playerTeamOrdinals,
    std::vector<RmsConnectionRecord> &out,std::string *err=nullptr);
// Original generate0x4e42b0: ordered records, unordered point pairs, terrain-1
// smoothing and final shore pass. Resolved records only; no scheduler draw.
// Validated on an initially empty world: water-1 setters create no objects or
// RNG effects. Terrain cache/dirty-render bookkeeping is not represented here.
// Failure leaves terrain and RNG unchanged.
bool generateRmsConnections(uint32_t width,uint32_t height,
    std::vector<uint8_t> &terrain,const std::vector<RmsConnectionRecord> &records,
    uint32_t &randomState,std::string *err=nullptr);
} // namespace swgb
