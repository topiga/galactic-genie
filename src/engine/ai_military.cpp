// SPDX-License-Identifier: GPL-3.0-or-later
#include "ai_military.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <unordered_set>

namespace swgb {

std::optional<AiMilitaryGroups> selectAiMilitaryGroups(
    const std::vector<AiMilitaryCandidate> &candidates,
    const std::vector<uint8_t> &projectedRegions,
    int requestedCount,
    float radius) {
    const size_t n = std::min<size_t>(candidates.size(), 40);
    if (requestedCount < 0 || !std::isfinite(radius) || radius < 0 ||
        candidates.size() > static_cast<size_t>(std::numeric_limits<int>::max()) ||
        projectedRegions.size() != n * n)
        return std::nullopt;
    std::unordered_set<int> ids;
    for (const auto &candidate : candidates) {
        if (candidate.id < 0 || !std::isfinite(candidate.x) ||
            !std::isfinite(candidate.y) || !ids.insert(candidate.id).second)
            return std::nullopt;
    }

    bool nearby[40][40] = {};
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            const float dx = candidates[i].x - candidates[j].x;
            const float dy = candidates[i].y - candidates[j].y;
            if (!std::isfinite(dx) || !std::isfinite(dy))
                return std::nullopt;
            // Original 4508f9 squares float deltas in x87 and takes sqrt.
            // shortcut: host long double may lack x87 precision; revisit if a
            // recorded near-boundary position shows a membership difference.
            nearby[i][j] = std::sqrt(static_cast<long double>(dx) * dx +
                                     static_cast<long double>(dy) * dy) <= radius;
        }
    }
    AiMilitaryGroups groups;
    if (requestedCount == 0 || n == 0)
        return groups;
    bool remaining[40];
    std::fill_n(remaining, n, true);
    int remainingCount = static_cast<int>(n);
    int emitted = 0;
    do {
        size_t center = n;
        int bestScore = 0;
        for (size_t i = 0; i < n; ++i) {
            if (!remaining[i]) continue;
            int score = 1;
            for (size_t j = 0; j < n; ++j) {
                if (j != i && remaining[j] && nearby[i][j] &&
                    projectedRegions[i * n + j] == candidates[i].ownRegion)
                    ++score;
            }
            if (score > bestScore) {
                bestScore = score;
                center = i;
            }
        }
        std::vector<int> group{candidates[center].id};
        remaining[center] = false;
        for (size_t j = 0; j < n; ++j) {
            if (remaining[j] && nearby[center][j] &&
                candidates[j].ownRegion == candidates[center].ownRegion) {
                group.push_back(candidates[j].id);
                remaining[j] = false;
            }
        }
        remainingCount -= static_cast<int>(group.size());
        // 450b47 uses FULL input size despite examining only its first 40 IDs.
        // 5ae726 separately limits actual group membership to the request.
        group.resize(std::min(group.size(), static_cast<size_t>(requestedCount - emitted)));
        emitted += static_cast<int>(group.size());
        groups.push_back(std::move(group));
    } while (emitted < requestedCount && remainingCount > 0 &&
             remainingCount > static_cast<int>(candidates.size()) - requestedCount);
    return groups;
}

} // namespace swgb
