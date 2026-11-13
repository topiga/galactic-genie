// SPDX-License-Identifier: GPL-3.0-or-later
#include "ai_targets.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace swgb {

std::optional<AiTargetRanking> rankAiTargets(
    const std::vector<AiTargetCandidate> &candidates) {
    if (candidates.size() > 1024) return std::nullopt;
    AiTargetRanking result;
    if (candidates.empty()) return result;
    const float maximum = std::numeric_limits<float>::max();
    float minA = maximum, maxA = 0, minB = maximum, maxB = 0;
    float minDistance = maximum, maxDistance = 0;
    for (const auto &candidate : candidates) {
        if (!std::isfinite(candidate.killTime) || candidate.killTime < 0 ||
            !std::isfinite(candidate.vulnerability) || candidate.vulnerability < 0 ||
            !std::isfinite(candidate.distance) || candidate.distance < 0 ||
            (candidate.priority != AiTargetPriority::Normal &&
             candidate.priority != AiTargetPriority::Force100 &&
             candidate.priority != AiTargetPriority::Force99Point9))
            return std::nullopt;
        minA = std::min(minA, candidate.killTime);
        maxA = std::max(maxA, candidate.killTime);
        minB = std::min(minB, candidate.vulnerability);
        maxB = std::max(maxB, candidate.vulnerability);
        minDistance = std::min(minDistance, candidate.distance);
        maxDistance = std::max(maxDistance, candidate.distance);
    }
    const float rangeA = maxA - minA, rangeB = maxB - minB;
    const float rangeDistance = maxDistance - minDistance;
    constexpr float epsilon = 1.0e-7f; // Exact float constant at original 6565f4.
    auto normalized = [](float value, float minimum, float range) {
        // shortcut: host long double may lack x87 precision; verify recorded
        // boundary cases before claiming bit-identical scores on Apple Silicon.
        return static_cast<float>(25.0L -
            (static_cast<long double>(value) - minimum) / range * 25.0L);
    };
    for (const auto &candidate : candidates) {
        float score;
        if (candidate.killTime < maximum && candidate.priority != AiTargetPriority::Normal) {
            score = candidate.priority == AiTargetPriority::Force100 ? 100.0f : 99.9f;
        } else {
            if (rangeB >= epsilon && rangeA == 0) return std::nullopt;
            const float a = rangeA < epsilon ? 25.0f : normalized(candidate.killTime, minA, rangeA);
            // Original 592b78 repeats metric A here, despite testing range B.
            const float b = rangeB < epsilon ? 25.0f : normalized(candidate.killTime, minA, rangeA);
            const float distance = rangeDistance < epsilon ? 25.0f :
                normalized(candidate.distance, minDistance, rangeDistance);
            score = static_cast<float>(static_cast<long double>(distance) + a +
                                       (candidate.sameRegion ? 25.0L : 0.0L) + b);
            if (!std::isfinite(score)) return std::nullopt;
            if (candidate.classSix && score >= 100.0f) score = 99.0f;
        }
        result.scores.push_back(score);
    }
    auto bucket = [&](size_t i) {
        const auto &candidate = candidates[i];
        if (result.scores[i] >= 100.0f) return 0;
        if (candidate.sameRegion && !candidate.wallLike)
            return candidate.preferredClass ? 1 : 2;
        if (candidate.sameRegion && candidate.wallLike && candidate.preferredClass) return 3;
        if (!candidate.sameRegion && !candidate.wallLike)
            return candidate.preferredClass ? 4 : 5;
        if (!candidate.sameRegion && candidate.wallLike && candidate.preferredClass) return 6;
        return 7; // Original leaves nonpreferred walls below 100 out of the order.
    };
    for (size_t i = 0; i < candidates.size(); ++i) {
        if (bucket(i) != 7) result.order.push_back(i);
    }
    std::stable_sort(result.order.begin(), result.order.end(), [&](size_t a, size_t b) {
        return bucket(a) != bucket(b) ? bucket(a) < bucket(b) : result.scores[a] > result.scores[b];
    });
    return result;
}

} // namespace swgb
