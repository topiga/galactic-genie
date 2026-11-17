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
    using namespace swgb;
    check(genieQueryDistance(50,50,5,{1,0,2,55,51})==5,
          "Original generated distance admits diagonal beyond center radius");
    check(genieQueryDistance(50,50,5,{1,1,2,56,50})==5 &&
          genieQueryDistance(50,50,5,{1,7,2,56,50})==1,
          "Original footprint adjustment and minimum distance");
    check(genieQueryDistance(50,50,5,{1,255,2,62,50})==1 &&
          genieQueryDistance(50,50,5,{1,255,2,63,50})==-1,
          "Coarse query bounds precede footprint subtraction");
    check(genieQueryDistance(50,50,100,{1,0,2,66,50})==-1 &&
          genieQueryDistance(50,50,0,{1,0,2,50,50})==0,
          "Original radius cap and zero-distance record");
    check(genieTargetQueryRadius(4.9f,6.9f)==7 && genieTargetQueryRadius(12.9f,4.f)==12 &&
          genieTargetQueryRadius(100.f,100.f)==15,"Query radius truncates search and sight independently");
    check(genieDiplomacyPartition(1,0,3)==0 && genieDiplomacyPartition(1,1,3)==1 &&
          genieDiplomacyPartition(1,2,0)==2 && genieDiplomacyPartition(1,2,1)==3 &&
          genieDiplomacyPartition(1,2,3)==4,"Original diplomacy query partitions");
    std::array<uint8_t,17> partitions{};
    const int classes[]={6};
    for(uint8_t owner=0;owner<5;++owner) {
        partitions[owner]=owner;
        for(uint32_t n=0;n<260;++n)index.update(1+owner*260+n,6,1,owner,50,50);
    }
    const auto groups=index.query(0,50,50,5,classes,partitions);
    bool caps=true;
    for(size_t n=0;n<5;++n)caps=caps && groups[n].size()==256 && groups[n].front().id==1+n*260 &&
        groups[n].back().id==256+n*260;
    check(caps,"Five independent 256-record caps preserve bucket order");
    index.clear();index.update(1,6,1,2,50,50);index.update(2,6,1,2,50,50);
    index.update(1,6,1,2,90,90);
    const int repeated[]={6,6};
    const auto moved=index.query(0,50,50,5,repeated,partitions);
    check(index.slot(0,1)==0 && moved[2].size()==2 && moved[2][0].id==2 && moved[2][1].id==2,
          "Metadata moves without bucket reordering; repeated query classes repeat records");
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
    auto &troop=const_cast<swgb::dat::DatFile &>(assets.dat()).civs.at(1).units.at(307);
    struct Restore {swgb::dat::Unit &unit,original;~Restore(){unit=std::move(original);}} restore{troop,troop};
    troop.lineOfSight=6;troop.searchRadius=4;troop.maxRange=1;
    swgb::Game queryGame(assets);
    check(queryGame.initScenario(scenario,&error),error.c_str());
    queryGame.setDiplomacyForTesting(1,2,3);queryGame.setDiplomacyForTesting(2,1,3);
    const auto spotter=queryGame.spawnObjectForTesting(1,83,1,34.f,38.f);
    queryGame.setAttackModeForTesting(spotter,3);
    const auto distant=queryGame.spawnObjectForTesting(1,307,2,34.75f,40.5f);
    queryGame.setAttackModeForTesting(distant,3);
    const auto seeker=queryGame.spawnObjectForTesting(1,307,1,27.25f,40.5f);
    for(int tick=0;tick<20 && !queryGame.attackTargetForTesting(seeker);++tick)queryGame.update(.1f,{});
    check(queryGame.attackTargetForTesting(seeker)==distant,
          "Native acquisition uses truncated query coordinates and LOS+1 beyond the old circle");
    std::puts("PASS: native visibility membership and equal-score reacquisition ordering");
}
