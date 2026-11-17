// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "../src/core/genie_target_selection.h"
#include <cstdio>
#include <cstdint>
#include <cstring>

static void targetSelectionChecks() {
    // Actual original 41b050/41b300/41b450 controls retained locally as
    // visible-score-{producer,stage}-controls-oracle-2026-11-17.json.
    using swgb::GenieTargetCandidate;
    using swgb::genieRankTargets;
    const float metric = swgb::genieTargetMetric(101, 3, 7, false);
    uint32_t bits = 0; std::memcpy(&bits, &metric, sizeof(bits));
    check(bits == 1083828127u, "Original producer stores HP/damage before reload division");
    check(swgb::genieTargetMetric(100, 0, 2, true) == 5 &&
          swgb::genieTargetMetric(100, 0, 2, false) == std::numeric_limits<float>::max(),
          "Original low-damage producer distinguishes capability fallback");
    std::vector<GenieTargetCandidate> input = {
        {1,false,2,10,1,49,0}, {2,false,6,30,1,49,0}, {3,false,4,20,1,49,15}};
    check(genieRankTargets(input,false,false).scores == std::vector<float>({85,0,57.5f}),
          "Original normalized damage and distance weights plus current bonus");
    input = {{1,false,4,9,1,49,0},{2,false,4,9,1,49,0}};
    check(genieRankTargets(input,false,false).scores == std::vector<float>({85,85}),
          "Equal extrema receive full original weights");
    input = {{1,true,2,4,1,49,0},{2,true,4,8,1,49,0},{3,false,10,20,1,49,0}};
    check(genieRankTargets(input,true,false).scores == std::vector<float>({85,0,0}),
          "Mobile-capable records use their separate normalization extrema");
    input = {{1,false,0,0,1,49,0},{2,false,5e-8f,5e-8f,1,49,0}};
    check(genieRankTargets(input,false,false).scores == std::vector<float>({85,85}),
          "Sub-threshold spread uses full weights");
    input = {{10,true,0,0,1,49,0},{11,true,0,0,1,49,0},{12,true,0,0,1,49,0}};
    check(genieRankTargets(input,true,false).indices == std::vector<size_t>({0,1,2}),
          "Original equal-score mobile tier preserves input order");
    input = {{10,false,0,0,1,49,15},{11,true,0,0,1,49,0},
             {12,false,0,0,1,7,115},{13,true,0,0,1,49,0}};
    check(genieRankTargets(input,true,false).indices == std::vector<size_t>({1,3,0,2}),
          "Mobile preference and class7 tier outrank raw score");
    check(genieRankTargets(input,false,false).indices == std::vector<size_t>({0,2}),
          "Original final mobile loop uses total-count bound and leaves unwritten slots");
    input = {{10,false,0,0,1,49,100},{11,true,0,0,1,49,1},
             {12,true,0,0,1,49,3},{13,true,0,0,1,49,2}};
    check(genieRankTargets(input,false,false).indices == std::vector<size_t>({0,2,3}),
          "One static and three mobile candidates write only three original slots");
    input = {{10,false,0,0,1,7,100},{11,false,0,0,1,49,1},{12,true,0,0,1,49,2}};
    check(genieRankTargets(input,false,true).indices == std::vector<size_t>({0,1}),
          "Original owner-mode range tier precedes non-class7 tier");
    check(genieRankTargets(input,false,false,2).scores == std::vector<float>({205,106,107}),
          "Zeroed producer category applies uniform history contribution");
    check(genieRankTargets({},true,false).indices.empty(), "Empty selection is safe");
    input.assign(140,{10,true,0,0,1,49,0});
    const auto capped = genieRankTargets(input,true,false);
    check(capped.indices.size()==128 && capped.scores.size()==128 && capped.indices.back()==127,
          "Original candidate cap is128 with stable equal-score order");
    std::puts("PASS: original target metric, normalization, preference tiers, stable ties and bounded output");
}

static void targetSelectionGameplayChecks(swgb::Assets &assets) {
    swgb::Game game(assets);std::string error;swgb::Scenario scenario;
    scenario.map.width=scenario.map.height=64;scenario.map.tiles.resize(64*64);
    scenario.nextUnitId=1;scenario.playerSlotCount=2;scenario.civilizations.fill(1);
    for(int i=0;i<2;++i){scenario.players[i].active=true;scenario.players[i].human=i==0;scenario.players[i].civilization=1;}
    check(game.initScenario(scenario,&error),error.c_str());
    game.setDiplomacyForTesting(1,2,3);game.setDiplomacyForTesting(2,1,3);
    const auto actor=game.spawnObjectForTesting(1,307,1,27.f,40.5f);
    const auto wall=game.spawnObjectForTesting(1,72,2,29.f,40.5f);
    const auto mobile=game.spawnObjectForTesting(1,307,2,31.f,40.5f);
    game.setAttackModeForTesting(mobile,3);
    for(int tick=0;tick<20 && !game.attackTargetForTesting(actor);++tick)game.update(.1f,{});
    check(wall && game.attackTargetForTesting(actor)==mobile,
          "Generic troop acquisition prioritizes mobile-capable target over nearer static target");
    game.stopUnitForTesting(actor);game.setAttackModeForTesting(actor,3);
    const auto tiedActor=game.spawnObjectForTesting(1,307,1,27.f,20.5f);
    const auto first=game.spawnObjectForTesting(1,307,2,30.f,20.f);
    const auto second=game.spawnObjectForTesting(1,307,2,30.f,21.f);
    game.setAttackModeForTesting(first,3);game.setAttackModeForTesting(second,3);
    for(int tick=0;tick<20 && !game.attackTargetForTesting(tiedActor);++tick)game.update(.1f,{});
    check(game.attackTargetForTesting(tiedActor)==first,
          "Equal candidate scores preserve native query order within one class and cell");
    std::puts("PASS: integrated mobile-preference and equal-score troop acquisition");
}
