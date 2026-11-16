// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "rms_script.h"
#include "rms_player_starts.h"

namespace swgb {
struct RmsObjectRow;
struct RmsMapLand {
    RmsPlayerStart center;
    int id=0,owner=0;
};
struct RmsObjectTarget {
    int landIndex=-1,owner=0;
};
// Original4dddf0 source-order dispatch; global selectors yield one target with
// landIndex=-1. Missing selected lands yield none, even when groups is zero.
// Original4de8fb resolves explicit owner, then nonnegative land owner, then Gaia.
// No RNG or placement; failure preserves out.
bool selectRmsObjectTargets(const RmsObjectRow &row,
    const std::vector<RmsMapLand> &lands,uint32_t playerCount,
    std::vector<RmsObjectTarget> &out,std::string *err=nullptr);
// Original 60-byte row semantics, not a binary overlay: bytes08/09 are mode/scale.
struct RmsObjectRow {
    int32_t unitId=-1, terrain=-1;
    uint8_t mode=0, scaling=0;
    int32_t perGroup=1, variance=0, groups=1, radius=3;
    int32_t owner=-1, landId=-1, minDistance=-1, maxDistance=-1;
    int32_t spacing=0, temporarySpacing=0, zoneDistance=0, matchPlayerCiv=0;
};
// Index points to the resolved unit argument after create_object. On success it
// points past }. Failure preserves index/output. No required attributes or RNG.
// Native bounds reject negative counts/radii/exclusions and distances below -1;
// they are safety limits, not assertions about original parser rejection.
bool parseRmsObjectRow(const std::vector<RmsToken> &tokens, size_t &index,
    RmsObjectRow &out, std::string *err=nullptr);
// Original4dddf0 scales groups only, once per row; playerCount excludes Gaia.
// Checked signed arithmetic rejects original-overflow cases. Failure preserves out.
bool finalizeRmsObjectRow(const RmsObjectRow &row, uint32_t width, uint32_t height,
    uint32_t playerCount, RmsObjectRow &out, std::string *err=nullptr);
} // namespace swgb
