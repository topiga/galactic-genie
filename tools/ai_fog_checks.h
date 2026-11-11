// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

static void aiFogChecks(swgb::Assets &assets) {
    std::string error;
    swgb::Game game(assets);
    check(game.initSkirmish(0x5A17,&error),error.c_str());
    const int troop=game.objectUnitId(11);
    const auto hunter=game.spawnObjectForTesting(1,troop,3,20.5f,20.5f);
    const auto target=game.spawnObjectForTesting(1,troop,4,22.5f,20.5f);
    check(hunter && target,"AI fog pursuit fixture");
    game.setDiplomacyForTesting(3,4,3);game.setDiplomacyForTesting(4,3,3);
    game.setAttackModeForTesting(target,3);
    check(game.issueAttackForTesting(hunter,target),"Visible attack begins");
    game.update(.001f,{});
    check(game.attackTargetForTesting(hunter)==target,"Visible target stays engaged");
    check(game.moveObjectForTesting(target,55.5f,55.5f),"Target leaves player sight");
    game.update(.001f,{});
    const auto lastSeen=game.moveAnchorForTesting(hunter);
    check(game.attackTargetForTesting(hunter)==0 && std::abs(lastSeen[0]-22.5f)<.05f &&
          std::abs(lastSeen[1]-20.5f)<.05f,"Fog pursuit travels to last seen point, not hidden current position");
    check(game.moveObjectForTesting(target,5.5f,55.5f),"Unseen target changes direction");
    game.update(.001f,{});
    check(game.moveAnchorForTesting(hunter)==lastSeen,"Unseen motion does not steer pursuit");
    check(game.moveObjectForTesting(hunter,20.5f,20.5f) && game.moveObjectForTesting(target,22.5f,20.5f),"Restore seen target");
    check(game.issueAttackForTesting(hunter,target),"Reacquire target");
    game.update(.001f,{});
    check(game.moveObjectForTesting(target,55.5f,55.5f) && game.damageObjectForTesting(target,100000),"Target dies beyond sight");
    game.update(.001f,{});
    check(game.attackTargetForTesting(hunter)==0 && std::abs(game.moveAnchorForTesting(hunter)[0]-22.5f)<.05f,
          "Unobserved death does not reveal the last-known destination is obsolete");

    auto prepareWave=[&](swgb::Game &match) {
        check(match.initSkirmish(0x5A17,&error),error.c_str());
        const auto home=match.objectPosition(match.commandCenterId(2));
        std::vector<uint32_t> wave;
        for(int i=0;i<6;++i) wave.push_back(match.spawnObjectForTesting(1,troop,2,home[0]+i*.3f,home[1]+6));
        return wave;
    };
    auto wave=prepareWave(game);
    game.updateSkirmishArmyForTesting(2);
    for(auto id:wave) check(game.attackTargetForTesting(id)==0,"Searching wave does not target unseen command center");
    const auto search=game.moveAnchorForTesting(wave.front());
    check(!game.observedForTesting(2,int(search[0]),int(search[1])),"Explored map still scouts genuinely unobserved ground");
    for(auto id:wave) check(game.moveAnchorForTesting(id)==search,"Wave keeps a coherent discovery destination");

    swgb::Game changedEnemy(assets);
    const auto changedWave=prepareWave(changedEnemy);
    check(changedEnemy.moveObjectForTesting(changedEnemy.commandCenterId(1),3.5f,60.5f),"Relocate unseen enemy base");
    check(changedEnemy.damageObjectForTesting(changedEnemy.commandCenterId(1),100000),"Remove unseen enemy base");
    changedEnemy.updateSkirmishArmyForTesting(2);
    check(changedEnemy.moveAnchorForTesting(changedWave.front())==search,"Scouting choice does not read unseen enemy base position or death");

    // Observe the old destination, then put the wave back: only knowledge changed.
    const auto start=game.objectPosition(wave.front());
    check(game.moveObjectForTesting(wave.front(),search[0],search[1]),"Scout observes chosen region");
    check(game.observedForTesting(2,int(search[0]),int(search[1])),"Scouting records actual observation");
    check(game.moveObjectForTesting(wave.front(),start[0],start[1]),"Scout returns to initial position");
    for(auto id:wave) check(game.stopUnitForTesting(id),"Stop wave fixture");
    game.updateSkirmishArmyForTesting(2);
    check(game.moveAnchorForTesting(wave.front())!=search,"Discovery history advances search instead of repeating old waypoint");
    wave=prepareWave(game);
    game.setDiplomacyForTesting(2,1,1);
    const auto surveyor=game.spawnObjectForTesting(1,83,2,1.5f,1.5f);
    for(int y=2;y<64;y+=4) for(int x=2;x<64;x+=4) {
        check(game.moveObjectForTesting(surveyor,x+.5f,y+.5f),"Survey revisit fixture");
        check(game.observedForTesting(2,x,y),"Revisit fixture records actual sight");
    }
    check(game.moveObjectForTesting(surveyor,60.5f,60.5f),"Park surveyor");
    game.updateSkirmishArmyForTesting(2);
    const auto revisit=game.moveAnchorForTesting(wave.front());
    check(game.observedForTesting(2,int(revisit[0]),int(revisit[1])),"Completed exploration enters revisit scouting");
    check(game.stopUnitForTesting(wave.front()) && game.moveObjectForTesting(wave.front(),4.5f,4.5f),
          "Relocate idle reinforcement away from travelling wave");
    game.updateSkirmishArmyForTesting(2);
    check(game.moveAnchorForTesting(wave.front())==revisit,
          "Reinforcement keeps shared revisit destination until renewed sight, not historical observation");
    check(game.moveObjectForTesting(wave.front(),revisit[0],revisit[1]) && game.stopUnitForTesting(wave.front()),
          "Complete shared revisit");
    game.updateSkirmishArmyForTesting(2);
    check(game.moveAnchorForTesting(wave.front())!=revisit,"Completed revisit advances search");
    std::printf("PASS: AI fog acquisition, last-seen pursuit and observation-based scouting\n");
}
