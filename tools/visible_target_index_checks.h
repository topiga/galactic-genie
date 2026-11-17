// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "../src/core/genie_visible_targets.h"
static void visibleTargetIndexChecks() {
    swgb::GenieVisibleTargets index;
    for(uint32_t id:{10u,11u,12u})index.update(id,6,5);
    check(index.bucket(0,6)==std::vector<uint32_t>({10,11,12}),"Original visible-index append order");
    index.update(10,6,4);
    check(index.bucket(0,6)==std::vector<uint32_t>({12,11}) && index.slot(0,12)==0 &&
          index.bucket(2,6)==std::vector<uint32_t>({10,11,12}),"Loss swaps last and repairs only the affected viewer slot");
    index.update(10,6,5);index.update(10,6,5);
    check(index.bucket(0,6)==std::vector<uint32_t>({12,11,10}),"Reappearance appends; unchanged masks retain order");
    index.update(12,7,5);
    check(index.bucket(0,6)==std::vector<uint32_t>({10,11}) && index.bucket(0,7)==std::vector<uint32_t>({12}),
          "Class replacement removes old membership before inserting new membership");
    for(uint32_t id=20;id<400;++id)index.update(id,6,1);
    index.update(10,6,0);
    check(index.slot(0,399)==0 && index.slot(0,10)==size_t(-1),"Growth preserves ID slot repair without record pointers");
    index.clear();check(index.bucket(0,6).empty() && index.slot(0,399)==size_t(-1),"New session clears visible membership");
    std::puts("PASS: visible-index append, swap removal, reappearance, class changes and reset");
}
static void visibleTargetIndexGameplayChecks(swgb::Assets &assets) {
    swgb::Game game(assets);std::string error;swgb::Scenario scenario;
    scenario.map.width=scenario.map.height=64;scenario.map.tiles.resize(64*64);
    scenario.nextUnitId=1;scenario.playerSlotCount=2;scenario.civilizations.fill(1);
    for(int i=0;i<2;++i){scenario.players[i].active=true;scenario.players[i].human=i==0;scenario.players[i].civilization=1;}
    check(game.initScenario(scenario,&error),error.c_str());
    game.setDiplomacyForTesting(1,2,3);game.setDiplomacyForTesting(2,1,3);
    const auto observer=game.spawnObjectForTesting(1,83,1,27.f,40.5f);
    game.setAttackModeForTesting(observer,3);
    const auto a=game.spawnObjectForTesting(1,307,2,29.f,40.f);
    const auto b=game.spawnObjectForTesting(1,307,2,29.f,41.f);
    const auto c=game.spawnObjectForTesting(1,307,2,30.f,40.5f);
    for(auto id:{a,b,c})game.setAttackModeForTesting(id,3);
    const int cls=game.unitPrototypeForTesting(1,307)->cls;
    check(game.visibleTargetsForTesting(1,cls)==std::vector<uint32_t>({a,b,c}),"Sight populates actual target buckets");
    game.moveObjectForTesting(a,55.f,55.f);
    check(game.visibleTargetsForTesting(1,cls)==std::vector<uint32_t>({c,b}),"Leaving sight swaps the last target into its slot");
    game.moveObjectForTesting(a,29.f,40.f);
    check(game.visibleTargetsForTesting(1,cls)==std::vector<uint32_t>({c,b,a}),"Returning sight appends instead of restoring spawn order");
    game.damageObjectForTesting(c,100000);
    check(game.visibleTargetsForTesting(1,cls)==std::vector<uint32_t>({a,b}),"Destroyed targets leave viewer membership");
    const auto actor=game.spawnObjectForTesting(1,307,1,27.f,40.5f);
    // a,b are equally distant and in the same class; deliberately put b first via loss/reappearance.
    game.moveObjectForTesting(a,55.f,55.f);game.visibleTargetsForTesting(1,cls);
    game.moveObjectForTesting(a,29.f,40.f);game.visibleTargetsForTesting(1,cls);
    for(int tick=0;tick<20 && !game.attackTargetForTesting(actor);++tick)game.update(.1f,{});
    check(game.attackTargetForTesting(actor)==b,"Scored acquisition consumes persistent visible order, not spawn or cell order");
    std::puts("PASS: native visibility membership and equal-score reacquisition ordering");
}
