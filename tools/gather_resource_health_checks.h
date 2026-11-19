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
    game.update(.1f,{});
    check(game.objectCarriedAmount(worker)>0.f && game.objectAnimationForTesting(worker)[0]==0.f,
          "Stationary worker advances its old sprite before changing to the gathering graphic");
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

    swgb::Scenario blocked;
    blocked.map.width=blocked.map.height=32;
    blocked.map.tiles.resize(32*32);
    blocked.civilizations.fill(1);
    blocked.nextUnitId=1;
    blocked.players[0].active=blocked.players[0].human=true;
    blocked.players[0].civilization=1;
    swgb::Game recovery(assets);
    check(recovery.initScenario(blocked,&error),error.c_str());
    const auto center=recovery.spawnObjectForTesting(1,109,1,16.f,16.f);
    const auto buried=recovery.spawnObjectForTesting(0,349,0,16.f,16.f);
    const auto accessible=recovery.spawnObjectForTesting(0,349,0,19.5f,16.5f);
    const auto cutter=recovery.spawnObjectForTesting(1,83,1,18.5f,16.5f);
    check(center && buried && accessible && cutter &&
          recovery.setWorkerCargoForTesting(cutter,1,4.f) && recovery.issueGatherForTesting(cutter,buried),
          "Loaded worker ordered to a tree inside a Command Center footprint");
    const float bank=recovery.resource(1,1);
    for(int i=0;i<400 && recovery.resource(1,1)==bank;++i) recovery.update(.1f,{});
    check(recovery.objectTaskReferencesForTesting(buried)==0 && recovery.resource(1,1)==bank+4 &&
          recovery.aiGatherTargetForTesting(cutter)==accessible,
          "Failed felling releases the blocked tree, deposits partial carbon and resumes at the reachable tree");

    check(recovery.initScenario(blocked,&error),error.c_str());
    recovery.spawnObjectForTesting(1,109,1,16.f,16.f);
    const auto isolated=recovery.spawnObjectForTesting(0,349,0,16.f,16.f);
    const auto idle=recovery.spawnObjectForTesting(1,293,1,18.5f,16.5f);
    check(recovery.issueGatherForTesting(idle,isolated),"Empty worker assigned to isolated tree");
    for(int i=0;i<400 && recovery.aiGatherTargetForTesting(idle);++i) recovery.update(.1f,{});
    check(recovery.aiGatherTargetForTesting(idle)==0 && recovery.objectTaskReferencesForTesting(isolated)==0,
          "An empty worker releases an unreachable tree with no alternative");

    check(recovery.initScenario(blocked,&error),error.c_str());
    recovery.spawnObjectForTesting(1,109,1,12.f,16.f);
    const auto finisher=recovery.spawnObjectForTesting(1,83,1,17.f,16.5f);
    const auto lastTree=recovery.spawnObjectForTesting(0,349,0,17.5f,16.5f);
    const auto nextTree=recovery.spawnObjectForTesting(0,349,0,18.5f,16.5f);
    recovery.setObjectResourceForTesting(lastTree,2.f);
    check(recovery.issueGatherForTesting(finisher,lastTree),"Gather last two carbon beside another tree");
    const auto openingCarbon=recovery.resource(1,1);
    for(int i=0;i<1000 && recovery.resource(1,1)==openingCarbon;++i) recovery.update(.1f,{});
    check(std::abs(recovery.resource(1,1)-openingCarbon-2.f)<.05f &&
          recovery.aiGatherTargetForTesting(finisher)==nextTree,
          "Exhausting a tree deposits partial cargo before gathering from the next tree");

    check(recovery.initScenario(blocked,&error),error.c_str());
    // A nearer depot across water or a continuous wall cannot receive this load.
    for (bool water : {true,false}) {
        check(recovery.initScenario(blocked,&error),error.c_str());
        for (int y=0;y<32;++y) {
            if (water) recovery.setTerrainForTesting(18,y,1);
            else recovery.spawnObjectForTesting(1,72,1,18.5f,y+.5f);
        }
        recovery.spawnObjectForTesting(1,562,1,21.5f,16.5f);
        recovery.spawnObjectForTesting(1,109,1,7.5f,16.5f);
        const auto carrier=recovery.spawnObjectForTesting(1,83,1,16.5f,16.5f);
        const auto tree=recovery.spawnObjectForTesting(0,349,0,16.f,16.f);
        check(recovery.issueGatherForTesting(carrier,tree) && recovery.setWorkerCargoForTesting(carrier,1,10),
              "Assign loaded worker on the Command Center side of the barrier");
        const float initial=recovery.resource(1,1);
        for(int i=0;i<600 && recovery.resource(1,1)==initial;++i) recovery.update(.1f,{});
        check(recovery.resource(1,1)==initial+10,
              "A worker deposits at the reachable building despite a nearer blocked processor");
    }

    check(recovery.initScenario(blocked,&error),error.c_str());
    const auto lumberjack=recovery.spawnObjectForTesting(1,83,1,16.f,16.5f);
    const auto timber=recovery.spawnObjectForTesting(0,349,0,16.5f,16.5f);
    check(recovery.issueGatherForTesting(lumberjack,timber),"Assign isolated lumberjack");
    for(int i=0;i<600 && recovery.objectCarriedAmount(lumberjack)<1.f;++i) recovery.update(.1f,{});
    const float beforeUpgrade=recovery.objectCarriedAmount(lumberjack);
    recovery.update(.1f,{});
    const float normalWork=recovery.objectCarriedAmount(lumberjack)-beforeUpgrade;
    check(normalWork>0 && recovery.researchTechnologyForTesting(1,160),"Upgrade active carbon gatherer");
    const float onUpgrade=recovery.objectCarriedAmount(lumberjack);
    recovery.update(.1f,{});
    check(std::abs(recovery.objectCarriedAmount(lumberjack)-onUpgrade-normalWork*1.2f)<.0001f,
          "Carbon upgrade applies to the working prototype during an existing order");
    for(int i=0;i<500;++i) recovery.update(.1f,{});
    check(recovery.objectCarriedAmount(lumberjack)==10 && recovery.researchTechnologyForTesting(1,128),
          "Standard worker stops at ten carbon without a depot");
    for(int i=0;i<150;++i) recovery.update(.1f,{});
    check(recovery.objectCarriedAmount(lumberjack)==13,"Worker carry upgrade rounds 12.5 up to 13 carbon");

}
