// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

static void aiFogChecks(swgb::Assets &assets) {
    std::string error;
    swgb::Game game(assets);
    check(game.initSkirmish(0x5A17,&error),error.c_str());
    const int troop=game.objectUnitId(11);
    const auto hunter=game.spawnObjectForTesting(1,troop,1,20.5f,20.5f);
    const auto target=game.spawnObjectForTesting(1,troop,2,22.5f,20.5f);
    check(hunter && target,"AI fog pursuit fixture");
    game.setDiplomacyForTesting(1,2,3);game.setDiplomacyForTesting(2,1,3);
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
    // Original419396 handles registered non-building death before lost-sight pursuit.
    check(game.attackTargetForTesting(hunter)==0 && game.objectTaskReferencesForTesting(target)==0,
          "Registered unit death releases the target before the lost-sight branch");

    const auto prepareScout=[&](swgb::Game &match) {
        check(match.initSkirmish(0x5A17,&error,true,1,3,200,true),error.c_str());
        const auto home=match.objectPosition(match.commandCenterId(2));
        const auto scout=match.spawnObjectForTesting(3,550,2,home[0]+3,home[1]+3);
        check(scout!=0,"Original singleton scouting fixture");
        check(match.loadAiSourceForTesting(2,"fog-exploration",R"(
(defrule (true) => (set-strategic-number sn-number-explore-groups 1)
 (set-strategic-number sn-minimum-explore-group-size 1) (disable-self))
)",{},&error),error.c_str());
        return scout;
    };
    const auto dispatch=[&](swgb::Game &match,uint32_t scout) {
        for(int i=0;i<8 && match.explorationStateForTesting(scout)!=11;++i) {
            match.update(0,{});match.updateAiMilitaryForTesting(2,UINT32_MAX);
        }
        check(match.explorationStateForTesting(scout)==11,"Exploration group dispatches its singleton scout");
    };
    const auto scout=prepareScout(game);dispatch(game,scout);
    check(game.attackTargetForTesting(scout)==0,"Initial scout does not target unseen Command Center");
    const auto search=game.explorationDestinationForTesting(scout);
    check(!game.observedForTesting(2,int(search[0]),int(search[1])),
          "Explored map still chooses genuinely unobserved exploration information");
    swgb::Game changedEnemy(assets);const auto changedScout=prepareScout(changedEnemy);
    check(changedEnemy.moveObjectForTesting(changedEnemy.commandCenterId(1),3.5f,60.5f),"Relocate unseen enemy base");
    check(changedEnemy.damageObjectForTesting(changedEnemy.commandCenterId(1),100000),"Remove unseen enemy base");
    dispatch(changedEnemy,changedScout);
    check(changedEnemy.explorationDestinationForTesting(changedScout)==search,
          "Initial scouting destination ignores unseen enemy position and destruction");
    const auto start=game.objectPosition(scout);
    check(game.moveObjectForTesting(scout,search[0],search[1]) && game.observedForTesting(2,int(search[0]),int(search[1])),
          "Scout records actual observation at its destination");
    check(game.moveObjectForTesting(scout,start[0],start[1]) && game.reissueExploreForTesting(scout),
          "Observation notification requests the next original exploration destination");
    game.update(0,{});game.update(0,{});
    check(game.explorationDestinationForTesting(scout)!=search,
          "Persistent information reservations prevent repeating the same waypoint");
    const auto surveyor=game.spawnObjectForTesting(1,83,2,1.5f,1.5f);
    for(int y=2;y<64;y+=4)for(int x=2;x<64;x+=4) {
        check(game.moveObjectForTesting(surveyor,x+.5f,y+.5f),"Survey revisit fixture");
        check(game.observedForTesting(2,x,y),"Survey records first-ever sight");
    }
    check(game.moveObjectForTesting(surveyor,60.5f,60.5f),"Park surveyor");
    check(!game.reissueExploreForTesting(scout),"Fully observed information exhausts the first spiral");
    check(game.reissueExploreForTesting(scout),"Exhaustion reopens known hidden same-zone cells for a subsequent search");
    game.update(0,{});game.update(0,{});
    const auto revisit=game.explorationDestinationForTesting(scout);
    check(game.observedForTesting(2,int(revisit[0]),int(revisit[1])),"Revisit uses genuinely known terrain");
    std::printf("PASS: AI fog acquisition, last-seen pursuit and original exploration information\n");
}
