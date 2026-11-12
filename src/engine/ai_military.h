// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <cstdint>
#include <optional>
#include <vector>

namespace swgb {

struct AiMilitaryCandidate {
    int id;
    float x;
    float y;
    uint8_t ownRegion;
};

using AiMilitaryGroups = std::vector<std::vector<int>>;

// Pure Clone Campaigns grouping, not eligibility, target selection or commands.
// Candidates must be unique, eligible, valid objects in original admission order.
// For n=min(candidates.size(),40), projectedRegions is an n*n row-major matrix:
// [center*n+other] samples CENTER's movement-region map at OTHER's truncated tile.
// ownRegion samples each object's own map at its own truncated tile. These are
// deliberately different tests in the original. Callers must supply real regions.
// Invalid input returns nullopt; a zero request returns an empty group list.
std::optional<AiMilitaryGroups> selectAiMilitaryGroups(
    const std::vector<AiMilitaryCandidate> &candidates,
    const std::vector<uint8_t> &projectedRegions,
    int requestedCount,
    float radius);

} // namespace swgb
