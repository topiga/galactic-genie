// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <algorithm>
#include <array>
#include <cstddef>
#include <cmath>
#include <limits>
#include <vector>

namespace swgb {
inline bool genieGenericTroopController(int cls) {
    return cls==43 || cls==44 || cls==47 || cls==48 || cls==49 || cls==52 ||
           cls==53 || cls==55 || cls==56 || cls==57;
}
// Original551b80/551ba0 uses axis-aligned footprint gaps. Its X delta and
// X gap are stored as floats; the Y calculation stays in the x87 register.
// Double models the unstored intermediates here; extreme x87 precision remains
// outside the finite-coordinate controls, rather than clamping original radii.
inline float genieTargetDistance(float sourceX, float sourceY, float sourceRadiusX,
                                 float sourceRadiusY, float targetX, float targetY,
                                 float targetRadiusX, float targetRadiusY) {
    const float deltaX = targetX - sourceX;
    float gapX = static_cast<float>(std::abs(double(deltaX)) - sourceRadiusX - targetRadiusX);
    double gapY = std::abs(double(targetY) - sourceY) - sourceRadiusY - targetRadiusY;
    if (gapX < 0) gapX = 0;
    if (gapY < 0) gapY = 0;
    return static_cast<float>(std::sqrt(double(gapX) * gapX + gapY * gapY));
}
// Original 41b050 stores the first quotient before dividing by the reload interval.
// This intentionally is not the conventional time-to-kill formula.
inline float genieTargetMetric(float hp, float damage, float reload, bool fallbackAllowed) {
    if (damage <= 1.0e-7f)
        return fallbackAllowed ? 5.0f : std::numeric_limits<float>::max();
    const float quotient = hp / damage;
    return quotient / reload;
}
struct GenieTargetCandidate {
    int id = -1;
    bool positiveSpeed = false; // Effective speed capability, not current movement.
    float metric = 0, distance = 0, maxRange = 0;
    int unitClass = -1;
    float currentBonus = 0;
};
struct GenieTargetRanking {
    std::vector<float> scores;
    std::vector<size_t> indices;
};
// Actual 41b300/41b450 controls. Input order is already filtered query order.
// No visibility, pathfinding, current-target retention or exclusion-cache policy here.
inline GenieTargetRanking genieRankTargets(const std::vector<GenieTargetCandidate>& input,
                                           bool preferMobile, bool ownerMode,
                                           float uniformHistory = 0) {
    const size_t n = std::min<size_t>(128, input.size());
    GenieTargetRanking result;
    result.scores.resize(n);
    result.indices.reserve(n);
    if (!n) return result;
    struct Range {
        float low = std::numeric_limits<float>::max(), high = -1;
        void add(float v) { if (v < low) low = v; if (v > high) high = v; }
    };
    Range metric, distance, mobileMetric, mobileDistance;
    size_t mobileCount = 0;
    for (size_t i = 0; i < n; ++i) {
        const auto& c = input[i];
        metric.add(c.metric); distance.add(c.distance);
        if (c.positiveSpeed) {
            ++mobileCount; mobileMetric.add(c.metric); mobileDistance.add(c.distance);
        }
    }
    const auto normalized = [](float value, const Range& range, float weight) {
        const float spread = range.high - range.low;
        if (spread < 1.0e-7f) return weight;
        // Original x87 retains intermediate precision until each contribution is stored.
        return static_cast<float>(double(weight) -
            (double(value) - range.low) / spread * weight);
    };
    const float history = 10.0f * uniformHistory;
    for (size_t i = 0; i < n; ++i) {
        const auto& c = input[i];
        const float m = normalized(c.metric, c.positiveSpeed ? mobileMetric : metric, 10);
        const float d = normalized(c.distance, c.positiveSpeed ? mobileDistance : distance, 75);
        result.scores[i] = static_cast<float>(double(c.currentBonus) + m + history + d);
    }
    std::vector<bool> used(n, false);
    const auto emit = [&](auto eligible, size_t limit) {
        while (result.indices.size() < limit) {
            size_t best = n;
            float score = 0;
            for (size_t i = 0; i < n; ++i) {
                if (used[i] || !eligible(input[i])) continue;
                if (best == n || result.scores[i] > score) { best = i; score = result.scores[i]; }
            }
            if (best == n) break;
            used[best] = true; result.indices.push_back(best);
        }
    };
    if (preferMobile) emit([](const auto& c) { return c.positiveSpeed; }, mobileCount);
    if (ownerMode) emit([](const auto& c) { return !c.positiveSpeed && c.maxRange >= 1.0e-7f; }, n);
    emit([](const auto& c) { return !c.positiveSpeed && c.unitClass != 7; }, n);
    emit([](const auto& c) { return !c.positiveSpeed && c.unitClass == 7; }, n);
    // Original compares TOTAL emitted indices against mobileCount, not n. It leaves
    // the remaining global index slots untouched. Return only freshly written slots;
    // callers must not interpret this as reproducing that persistent stale buffer.
    if (!preferMobile) emit([](const auto& c) { return c.positiveSpeed; }, mobileCount);
    return result;
}
// Hit selector flags(0,0), after scoring/sorting; original41a150 controls.
inline bool genieRetainHitTarget(size_t mobileCount,bool currentMobile,bool currentThreat,size_t threatCount) {
    return mobileCount<5 && (currentMobile || mobileCount==0) && (currentThreat || threatCount==0);
}
// Original output slots outlive each query; cleared candidate records resolve to ID0.
class GenieTargetSlots {
    std::array<size_t,128> indices_{};
public:
    void update(const GenieTargetRanking &ranking) {
        std::copy_n(ranking.indices.begin(),std::min(ranking.indices.size(),indices_.size()),indices_.begin());
    }
    GenieTargetCandidate candidate(const std::vector<GenieTargetCandidate> &input,size_t slot) const {
        const size_t index=indices_.at(slot);
        return index<input.size()?input[index]:GenieTargetCandidate{0};
    }
};
} // namespace swgb
