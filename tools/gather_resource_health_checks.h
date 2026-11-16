// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

static void gatherResourceHealthChecks(swgb::Assets &assets) {
    auto &node=const_cast<swgb::dat::DatFile &>(assets.dat()).civs.at(0).units.at(102);
    struct Restore {
        swgb::dat::Unit &unit, original;
        ~Restore() { unit=std::move(original); }
    } restore{node,node};
    // Isolate the resource callback from felling and depletion, with observable HP.
    node.hitPoints=50;
    swgb::Game game(assets);std::string error;
    check(game.init(7,96,&error,1,1),error.c_str());
    const auto worker=game.spawnObjectForTesting(1,83,1,27.f,40.5f);
    const auto deposit=game.spawnObjectForTesting(0,102,0,27.5f,40.5f);
    check(worker && deposit && game.objectHitPoints(deposit)==50.f,
          "Positive-HP resource fixture");
    const float before=game.objectResourceAmount(deposit);
    check(game.issueGatherForTesting(worker,deposit),"Assign resource work");
    for(int step=0;step<300 && game.objectCarriedAmount(worker)<.25f;++step)
        game.update(.1f,{});
    check(game.objectResourceAmount(deposit)<before && game.objectCarriedAmount(worker)>0.f,
          "Resource callback actually transfers resources");
    check(game.objectHitPoints(deposit)==50.f,
          "Original resource subtraction does not damage the target");
    check(game.damageObjectForTesting(deposit,13),"Damage resource independently of gathering");
    check(game.objectHitPoints(deposit)==37.f,"Resource has independently damaged HP");
    const float damagedAmount=game.objectResourceAmount(deposit);
    for(int step=0;step<10;++step) game.update(.1f,{});
    check(game.objectResourceAmount(deposit)<damagedAmount && game.objectHitPoints(deposit)==37.f,
          "Gathering neither damages nor heals an already damaged resource");
    for (auto &storage:node.resourceStorages)
        if (storage.type>=0 && storage.type<=3 && storage.amount>0) storage.amount=2.f;
    swgb::Game exhausted(assets);
    check(exhausted.init(7,96,&error,1,1),error.c_str());
    const auto gatherer=exhausted.spawnObjectForTesting(1,83,1,27.f,40.5f);
    const auto living=exhausted.spawnObjectForTesting(0,102,0,27.5f,40.5f);
    check(exhausted.issueGatherForTesting(gatherer,living) && exhausted.selectObjectForTesting(living),
          "Selected living resource assigned for exhaustion");
    for(int step=0;step<200 && exhausted.objectResourceAmount(living)>0.f;++step)
        exhausted.update(.1f,{});
    check(exhausted.objectResourceAmount(living)==0.f && exhausted.objectCarriedAmount(gatherer)>0.f,
          "Living resource actually exhausted");
    for(int step=0;step<4;++step) exhausted.update(.1f,{});
    check(exhausted.objectActive(living) && exhausted.objectHitPoints(living)==50.f,
          "Exhaustion alone does not kill a living base resource");
    check(exhausted.objectSelected(living) && exhausted.selectObjectForTesting(living),
          "Exhausted living resource retains selection and inspection");

}
