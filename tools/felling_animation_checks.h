// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

static void fellingAnimationChecks(swgb::Assets &assets) {
    swgb::Game game(assets);std::string error;
    check(game.init(7,96,&error,1,1),error.c_str());
    const auto worker=game.spawnObjectForTesting(1,83,1,27.f,40.5f);
    const auto tree=game.spawnObjectForTesting(0,349,0,27.5f,40.5f);
    check(worker && tree && game.objectHitPoints(tree)==50.f &&
          game.issueGatherForTesting(worker,tree),"Original felling fixtures");
    game.update(.1f,{}); // Native in-range entry at the verified attack-state7 boundary.
    for(int phase=1;phase<=24;++phase) {
        game.update(.1f,{});
        const float hp=phase<4?50.f:phase<24?25.f:0.f;
        check(game.objectHitPoints(tree)==hp && game.objectResourceAmount(tree)==100.f &&
              !game.objectFelled(tree),"Frame crossing and preserved animation phase drive two felling hits");
        check(game.objectCarriedAmount(worker)==0.f,"Attack preparation does not gather resources");
        if(phase==3) {
            game.updatePaused(10.f,{});
            check(game.objectHitPoints(tree)==50.f,"Pause freezes felling animation and damage");
        }
    }
    game.update(.1f,{});
    check(game.objectFelled(tree) && game.objectActive(tree),
          "Target owner admits felling on its next update without deleting the resource");
    for(int phase=0;phase<2;++phase) {
        game.update(.1f,{});
        check(game.objectCarriedAmount(worker)==0.f,"Preparation transitions do not transfer resources");
    }
    game.update(.1f,{});
    check(game.objectCarriedAmount(worker)>0.f && game.objectResourceAmount(tree)<100.f,
          "Completed animation preparation returns to resource gathering");
    auto &raw=const_cast<swgb::dat::DatFile &>(assets.dat()).civs.at(0).units.at(349);
    struct Restore { swgb::dat::Unit &unit, original; ~Restore(){unit=std::move(original);} } restore{raw,raw};
    for(auto &storage:raw.resourceStorages)
        if(storage.type==1)storage.amount=.2f;
    swgb::Game depletion(assets);
    check(depletion.init(7,96,&error,1,1),error.c_str());
    const auto cutter=depletion.spawnObjectForTesting(1,83,1,27.f,40.5f);
    const auto smallTree=depletion.spawnObjectForTesting(0,349,0,27.5f,40.5f);
    check(depletion.issueGatherForTesting(cutter,smallTree) && depletion.selectObjectForTesting(smallTree),
          "Selected tree assigned for deferred exhaustion");
    for(int tick=0;tick<200 && depletion.objectResourceAmount(smallTree)>0.f;++tick)
        depletion.update(.1f,{});
    check(depletion.objectResourceAmount(smallTree)==0.f && depletion.objectActive(smallTree) &&
          depletion.objectSelected(smallTree),"Gather exhaustion retains dying resource until its owner update");
    depletion.update(.1f,{});
    check(!depletion.objectActive(smallTree) && depletion.objectRegisteredForTesting(smallTree) &&
          !depletion.objectSelected(smallTree),"State7 removes actionability and selection but preserves registration");
    depletion.update(.1f,{});
    check(depletion.objectRegisteredForTesting(smallTree),"State8 remains registered for one further owner update");
    depletion.update(.1f,{});
    check(!depletion.objectRegisteredForTesting(smallTree),"Final owner update deregisters the dead resource");

    swgb::Game canceled(assets);
    check(canceled.init(7,96,&error,1,1),error.c_str());
    const auto interrupted=canceled.spawnObjectForTesting(1,83,1,27.f,40.5f);
    const auto uncut=canceled.spawnObjectForTesting(0,349,0,27.5f,40.5f);
    check(canceled.issueGatherForTesting(interrupted,uncut),"Start interruptible felling");
    for(int tick=0;tick<4;++tick)canceled.update(.1f,{});
    check(canceled.stopUnitForTesting(interrupted),"Stop pending preparation attack");
    for(int tick=0;tick<10;++tick)canceled.update(.1f,{});
    check(canceled.objectHitPoints(uncut)==50.f,"Canceled preparation cannot deliver its pending hit");
    check(canceled.issueGatherForTesting(interrupted,uncut),"Reassign the same tree after Stop");
    for(int tick=0;tick<5;++tick)canceled.update(.1f,{});
    check(canceled.objectHitPoints(uncut)==25.f,"Reassigned preparation starts a fresh attack phase");

}
