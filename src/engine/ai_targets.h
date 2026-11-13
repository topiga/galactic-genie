// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <cstddef>
#include <optional>
#include <vector>

namespace swgb {

enum class AiTargetPriority { Normal, Force100, Force99Point9 };

// Explicit, already-resolved inputs to original 592980/592c30. This stage
// neither discovers enemies nor computes damage, fog, diplomacy or priorities.
struct AiTargetCandidate {
    float killTime;
    float vulnerability;
    float distance;
    bool preferredClass;
    bool wallLike;
    bool sameRegion;
    bool classSix;
    AiTargetPriority priority = AiTargetPriority::Normal;
};

struct AiTargetRanking {
    std::vector<float> scores;       // Input order, including omitted candidates.
    std::vector<size_t> order;       // Original bucket order; may omit candidates.
};

// At most 1024 candidates, matching original scratch storage. FLT_MAX is the
// original unavailable metric sentinel. Priority overrides only apply when
// killTime < FLT_MAX. Invalid/nonfinite inputs or undefined scoring return nullopt.
std::optional<AiTargetRanking> rankAiTargets(
    const std::vector<AiTargetCandidate> &candidates);

} // namespace swgb
