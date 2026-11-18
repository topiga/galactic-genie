// SPDX-License-Identifier: GPL-3.0-or-later
#include "engine/ai_military.h"

#include <cstdlib>
#include <iostream>
#include <limits>

using namespace swgb;

static void check(bool value, const char *message) {
    if (!value) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

#include "ai_military_scheduler_checks.h"
#include "military_admission_checks.h"

int main() {
    aiMilitarySchedulerChecks();
    militaryAdmissionChecks();
    // Fixtures derive from original 4506f0 and final truncation at 5ae726;
    // local research: ai-controller-facts/attack-group-findings.txt.
    std::vector<AiMilitaryCandidate> pair{{10, 0, 0, 1}, {20, 3, 4, 1}};
    auto groups = selectAiMilitaryGroups(pair, {1, 1, 1, 1}, 2, 5);
    check(groups && *groups == AiMilitaryGroups{{10, 20}}, "inclusive 3-4-5 radius");
    groups = selectAiMilitaryGroups(pair, {1, 1, 1, 1}, 1, 5);
    check(groups && *groups == AiMilitaryGroups{{10}}, "final cluster truncated to request");
    groups = selectAiMilitaryGroups(pair, {1, 1, 1, 1}, 1, 4);
    check(groups && *groups == AiMilitaryGroups{{10}}, "first center wins density tie");
    groups = selectAiMilitaryGroups(pair, {1, 1, 1, 1}, 2, 4);
    check(groups && *groups == AiMilitaryGroups{{10}, {20}}, "separate distant clusters");

    groups = selectAiMilitaryGroups({{10,0,0,1},{20,100,0,1},{30,101,0,1},{40,102,0,1}},
                                    std::vector<uint8_t>(16,1),4,1);
    check(groups && *groups == AiMilitaryGroups{{30,20,40},{10}},
          "densest center first, then members in admission order");

    // Center 20 scores higher because of center-map projection, but cannot
    // recruit 10 because actual membership compares each object's own region.
    pair = {{10, 0, 0, 1}, {20, 0, 0, 2}};
    groups = selectAiMilitaryGroups(pair, {1, 0, 2, 2}, 2, 1);
    check(groups && *groups == AiMilitaryGroups{{20}, {10}}, "asymmetric region scoring and membership");

    std::vector<AiMilitaryCandidate> many;
    for (int i = 0; i < 100; ++i) many.push_back({i, static_cast<float>(i), 0, 1});
    const std::vector<uint8_t> regions(40 * 40, 1);
    groups = selectAiMilitaryGroups(many, regions, 50, 0);
    check(groups && *groups == AiMilitaryGroups{{0}}, "full-input threshold with first-40 cap");
    groups = selectAiMilitaryGroups(many, regions, 100, 0);
    check(groups && groups->size() == 40 && groups->back() == std::vector<int>{39}, "never consumes candidate 41");
    groups = selectAiMilitaryGroups(many, regions, 200, 1000);
    check(groups && groups->size() == 1 && groups->front().size() == 40, "oversized request bounded by candidates");

    check(aiExploreCandidateEligible(0,.81f) && !aiExploreCandidateEligible(1,2) &&
          !aiExploreCandidateEligible(0,.8f) && !aiExploreCandidateEligible(0,std::numeric_limits<double>::quiet_NaN()),
          "Original flag1 admission rejects heroes, slow actors and unordered speed");
    std::vector<AiMilitaryCandidate> holes{{10,0,0,1,false},{20,1,0,1,true},{30,2,0,1,false},{40,3,0,1,true}};
    groups=selectAiMilitaryGroups(holes,std::vector<uint8_t>(16,1),4,10);
    check(groups && *groups==AiMilitaryGroups{{20,40}},"Rejected exploration candidates preserve slots and ordered survivors");
    many.assign(41,{0,0,0,1,false});
    for(int i=0;i<41;++i)many[i].id=i;
    many[40].eligible=true;
    check(selectAiMilitaryGroups(many,regions,41,10)->empty(),
          "Rejected first40 slots do not promote candidate41 into the original selection window");

    check(selectAiMilitaryGroups({}, {}, 0, 1)->empty(), "empty candidates");
    check(selectAiMilitaryGroups(pair, {1, 0, 2, 2}, 0, 1)->empty(), "zero request");
    check(!selectAiMilitaryGroups(pair, {}, 1, 1), "missing region data rejected");
    check(!selectAiMilitaryGroups(pair, {1, 0, 2, 2}, -1, 1), "negative request rejected");
    check(!selectAiMilitaryGroups(pair, {1, 0, 2, 2}, 1, -1), "negative radius rejected");
    check(!selectAiMilitaryGroups(pair, {1, 0, 2, 2}, 1,
                                  std::numeric_limits<float>::infinity()), "infinite radius rejected");
    pair[1].id = pair[0].id;
    check(!selectAiMilitaryGroups(pair, {1, 0, 2, 2}, 1, 1), "duplicate IDs rejected");
    pair[1].id = -1;
    check(!selectAiMilitaryGroups(pair, {1, 0, 2, 2}, 1, 1), "invalid ID rejected");
    pair[1].id = 20;
    pair[1].x = std::numeric_limits<float>::quiet_NaN();
    check(!selectAiMilitaryGroups(pair, {1, 0, 2, 2}, 1, 1), "NaN coordinate rejected");
    pair[1].x = std::numeric_limits<float>::max();
    pair[0].x = -pair[1].x;
    check(!selectAiMilitaryGroups(pair, {1, 0, 2, 2}, 1, 1), "overflowing float delta rejected");
    std::cout << "AI military grouping tests passed\n";
}
