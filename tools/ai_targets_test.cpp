// SPDX-License-Identifier: GPL-3.0-or-later
#include "engine/ai_targets.h"

#include <cstdlib>
#include <iostream>
#include <limits>

using namespace swgb;
static void check(bool ok, const char *message) {
    if (!ok) { std::cerr << "FAIL: " << message << '\n'; std::exit(1); }
}
int main() {
    // These five expected outputs were executed from the original 592980 and
    // 592c30 machine code in Unicorn; local target-score-oracle-fixtures.json.
    std::vector<AiTargetCandidate> candidates{
        {1, 10, 1, true, false, true, false},
        {2, 0, 2, true, false, true, false},
        {3, 5, 3, true, false, true, false}};
    auto ranking = rankAiTargets(candidates);
    check(ranking && ranking->scores == std::vector<float>{100, 62.5f, 25} &&
          ranking->order == std::vector<size_t>{0, 1, 2}, "original repeats metric A in second component");
    candidates = {{2, 4, 3, true, false, true, false}, {2, 4, 3, true, false, true, false}};
    ranking = rankAiTargets(candidates);
    check(ranking && ranking->scores == std::vector<float>{100, 100} &&
          ranking->order == std::vector<size_t>{0, 1}, "original equal metrics preserve first tie");
    candidates[0].classSix = true;
    candidates[0].wallLike = true;
    ranking = rankAiTargets(candidates);
    check(ranking && ranking->scores == std::vector<float>{99, 100} &&
          ranking->order == std::vector<size_t>{1, 0}, "original class-six cap");
    candidates = {{1, 1, 1, true, false, false, false}, {3, 3, 3, true, false, true, false}};
    ranking = rankAiTargets(candidates);
    check(ranking && ranking->scores == std::vector<float>{75, 25} &&
          ranking->order == std::vector<size_t>{1, 0}, "original reachable bucket precedes greater score");

    // All ordinary scores below 100, so every priority bucket is observable.
    candidates.clear();
    for (int i = 0; i < 8; ++i)
        candidates.push_back({2, 4, 3, (i & 1) != 0, (i & 2) != 0, (i & 4) != 0, true});
    ranking = rankAiTargets(candidates);
    check(ranking && ranking->order == std::vector<size_t>{5, 4, 7, 1, 0, 3}, "bucket order and omitted nonpreferred walls");
    candidates[2].priority = AiTargetPriority::Force100;
    candidates[6].priority = AiTargetPriority::Force99Point9;
    ranking = rankAiTargets(candidates);
    check(ranking && ranking->order == std::vector<size_t>{2, 5, 4, 7, 1, 0, 3} &&
          ranking->scores[2] == 100 && ranking->scores[6] == 99.9f,
          "priority override bypasses class cap but not sub-100 bucket exclusion");
    candidates[2].killTime = std::numeric_limits<float>::max();
    ranking = rankAiTargets(candidates);
    check(ranking && ranking->scores[2] != 100, "FLT_MAX sentinel prevents override");

    check(rankAiTargets({})->order.empty(), "empty input");
    check(!rankAiTargets({{1, 0, 0, true, false, true, false},
                          {1, 1, 0, true, false, true, false}}), "reject original undefined repeated-A division");
    candidates = {{-1, 1, 1, true, false, true, false}};
    check(!rankAiTargets(candidates), "negative metric rejected");
    candidates[0].killTime = std::numeric_limits<float>::quiet_NaN();
    check(!rankAiTargets(candidates), "NaN rejected");
    candidates[0].killTime = 1;
    candidates[0].distance = std::numeric_limits<float>::infinity();
    check(!rankAiTargets(candidates), "infinity rejected");
    candidates[0].distance = 1;
    candidates[0].priority = static_cast<AiTargetPriority>(17);
    check(!rankAiTargets(candidates), "invalid priority rejected");
    candidates.assign(1025, {1, 1, 1, true, false, true, false});
    check(!rankAiTargets(candidates), "original scratch capacity enforced");
    std::cout << "AI target scoring and ordering tests passed\n";
}
