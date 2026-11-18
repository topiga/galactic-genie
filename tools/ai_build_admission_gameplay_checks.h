// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

static void aiBuildAdmissionGameplayChecks(swgb::Assets &assets) {
    swgb::Game game(assets);std::string error;
    auto reset=[&] {
        check(game.initSkirmish(0x5A17,&error,false,1,1),error.c_str());
        for(int player:{1,2}) {
            for(int type=0;type<4;++type)game.setResourceForTesting(player,type,0);
            for(const auto &cost:game.unitCostsForTesting(player,assets.dat().civs[1].units[70]))
                if(cost.flag && cost.type>=0 && cost.amount>0)game.addResourceForTesting(player,cost.type,cost.amount);
        }
    };
    auto load=[&](int player,const std::string &source) {
        check(game.loadAiSourceForTesting(player,"build-admission-runtime",source,{},&error),error.c_str());
    };
    auto followingPass=[&] {for(int n=0;n<25;++n)game.update(0,{});};
    reset();
    const int before=game.objectCountForTesting(1,70);
    load(1,"(defrule (can-build 70) (not (can-build 109)) => (set-goal 970 1) (disable-self))"
           "(defrule (true) => (build 109) (disable-self))"
           "(defrule (can-build 70) => (set-goal 971 1) (build 70) (disable-self))");
    load(2,"(defrule (can-build 70) => (set-goal 972 1) (disable-self))");
    game.update(0,{});
    check(game.aiGoalForTesting(1,970)==1,"Prefab is affordable and Command Center unaffordable before build attempt");
    check(game.aiGoalForTesting(1,971)==0 && game.objectCountForTesting(1,70)==before,
          "Unaffordable first build consumes the same-pass attempt and blocks later affordable build");
    check(game.aiGoalForTesting(2,972)==1,"Another owner does not inherit the previous owner's build attempt");
    followingPass();
    check(game.aiGoalForTesting(1,971)==1,
          "Next admitted PER evaluation resets the attempt and re-enables the affordable build fact");

    reset();
    load(1,"(defrule (can-build 70) => (set-goal 973 1) (disable-self))"
           "(defrule (true) => (build 72) (disable-self))"
           "(defrule (can-build-with-escrow 70) => (set-goal 974 1) (disable-self))");
    load(2,"(defrule (true) => (disable-self))");game.update(0,{});
    check(game.aiGoalForTesting(1,973)==1 && game.aiGoalForTesting(1,974)==0,
          "Original excluded prototype72 action consumes latch and blocks escrow-aware build facts too");
    followingPass();
    check(game.aiGoalForTesting(1,974)==1,"Excluded action's disabled rule does not poison subsequent evaluations");
    std::puts("PASS: PER build-attempt failure consumption, excluded prototype, next-pass reset and owner isolation");
}
