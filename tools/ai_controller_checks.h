// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

static void aiControllerChecks(swgb::Assets &assets) {
    swgb::Game game(assets); std::string error;
    check(game.initSkirmish(0x5A17,&error,false,1,2),error.c_str());
    auto run=[&](int player,const std::string &source) {
        check(game.loadAiSourceForTesting(player,"controller-facts",source,{},&error),error.c_str());
        game.update(0,{});
    };
    run(1,"(defrule (difficulty == moderate) (difficulty > hard) (difficulty < easy) => (set-goal 910 1))"
          "(defrule (difficulty == hardest) => (set-goal 911 1))"
          "(defrule (strategic-number sn-percent-civilian-explorers == 34)"
          " (strategic-number sn-percent-civilian-gatherers == 66)"
          " (strategic-number sn-cap-civilian-builders == 2)"
          " (strategic-number sn-number-explore-groups == 0)"
          " (strategic-number sn-maximum-town-size == 20)"
          " (strategic-number sn-nova-gatherer-percentage == 0)"
          " (strategic-number sn-carbon-gatherer-percentage == 0)"
          " (strategic-number sn-task-ungrouped-soldiers == 1)"
          " (strategic-number sn-number-forward-builders == 0) => (set-goal 912 1))"
          "(defrule (difficulty < easiest) => (set-goal 913 1))");
    check(game.aiGoalForTesting(1,910)==1 && game.aiGoalForTesting(1,911)==0 &&
          game.aiGoalForTesting(1,912)==1 && game.aiGoalForTesting(1,913)==1,
          "Original difficulty constants and verified profile defaults exist before first rule evaluation");
    run(1,"(defrule (or-list 6 (false) (false) (false) (false) (false) (true)) => (set-goal 914 1))"
          "(defrule (not (or-list 0)) => (set-goal 915 1))"
          "(defrule (not (or-list 2 (false))) => (set-goal 916 1))"
          "(defrule (not (or-list 2 (unknown-fact) (false))) => (set-goal 917 1))"
          "(defrule (or-list 2 (true) (unknown-fact)) => (set-goal 918 1))");
    check(game.aiGoalForTesting(1,914)==1 && game.aiGoalForTesting(1,915)==1 &&
          game.aiGoalForTesting(1,916)==0 && game.aiGoalForTesting(1,917)==0 && game.aiGoalForTesting(1,918)==1,
          "Original counted OR, empty result, malformed count, unknown propagation and short circuit");
    auto populations=[&](int civilians,int military) {
        run(3,"(defrule (civilian-population == "+std::to_string(civilians)+") (military-population == "+
            std::to_string(military)+") => (set-goal 901 1))");
        return game.aiGoalForTesting(3,901)==1;
    };
    check(populations(0,0),"Empty player has zero original AI population counters");
    const auto worker=game.spawnObjectForTesting(1,83,3,10,10);
    check(worker && game.spawnObjectForTesting(1,13,3,12,10) && game.spawnObjectForTesting(1,931,3,14,10),
          "Create worker, fishing boat and trader fixtures");
    check(populations(3,0),"Original civilian count includes fishing and trading classes");
    check(game.spawnObjectForTesting(1,939,3,16,10) && game.spawnObjectForTesting(1,307,3,18,10),
          "Create medic and trooper fixtures");
    check(populations(3,2),"Original military count includes nonfighting medics");
    check(game.spawnObjectForTesting(1,70,3,20,10) && game.spawnObjectForTesting(1,79,3,22,10),
          "Create house and turret fixtures");
    check(populations(3,2),"Houses and defense turrets are not military population");
    auto &corpse=const_cast<swgb::dat::DatFile &>(assets.dat()).civs[1].units[940];
    const auto originalClass=corpse.cls;
    struct Restore { int16_t &field; int16_t value; ~Restore(){field=value;} } restore{corpse.cls,originalClass};
    corpse.cls=54;
    check(game.spawnObjectForTesting(1,940,3,24,10) && populations(3,2),
          "Noncreatable object with military class does not count");
    const auto center=game.spawnObjectForTesting(1,109,3,9,10);
    check(center && game.garrisonForTesting(worker,center),"Garrison civilian fixture");
    for (int i=0;i<80 && game.garrisonContainerForTesting(worker)<0;++i) game.update(.1f,{});
    check(game.garrisonContainerForTesting(worker)==int(center) && populations(3,2),
          "Garrisoned civilians remain in original AI population count");
    check(game.queueUnitForTesting(center,83) && populations(3,2),"Paid queued unit does not yet count");
    check(game.cancelProductionForTesting(center),"Cancel queued-unit counter fixture");

    run(1,"(defconst my-civ 1)(defrule (civ-selected my-civ) => (set-goal 902 1))"
          "(defrule (civ-selected 2) => (set-goal 903 1))"
          "(defrule (not (civ-selected unknown-civ)) => (set-goal 904 1))");
    check(game.aiGoalForTesting(1,902)==1 && game.aiGoalForTesting(1,903)==0 && game.aiGoalForTesting(1,904)==0,
          "Civilization query uses own ID and does not negate unresolved symbols into true");
    run(1,"(defconst explore-number 42)"
          "(defrule (true) => (set-strategic-number explore-number 3) (disable-self))"
          "(defrule (strategic-number sn-number-explore-groups == 3) => (set-goal 905 1))"
          "(defrule (true) => (set-strategic-number sn-number-explore-groups 4) (disable-self))"
          "(defrule (strategic-number 42 == 4) => (set-goal 906 1))"
          "(defrule (not (strategic-number untraced-name == 0)) => (set-goal 907 1))"
          "(defrule (not (strategic-number 241 == 0)) => (set-goal 908 1))"
          "(defrule (true) => (set-strategic-number 242 7) (disable-self))");
    check(game.aiGoalForTesting(1,905)==1 && game.aiGoalForTesting(1,906)==1 &&
          game.aiGoalForTesting(1,907)==0 && game.aiGoalForTesting(1,908)==0 &&
          game.aiStrategicNumberForTesting(1,"42")==4 && game.aiStrategicNumberForTesting(1,"242")==0,
          "Strategic-number names, numeric indices and aliases share state; missing defaults stay unknown");
    run(2,"(defrule (strategic-number 42 == 4) => (set-goal 905 1))");
    check(game.aiGoalForTesting(2,905)==0,"Strategic values are private to each player");
    game.setResourceForTesting(1,0,0); game.addResourceForTesting(1,0,-.5f);
    run(1,"(defrule (food-amount == 0) => (set-goal 909 1))");
    check(game.aiGoalForTesting(1,909)==1,"Original resource facts truncate toward zero");
    check(game.initSkirmish(0x5A17,&error,false,1,2),error.c_str());
    check(game.loadAiSourceForTesting(2,"idle-script","",{},&error),error.c_str());
    std::array<float,4> stock{};
    for(int type=0;type<4;++type) stock[size_t(type)]=game.resource(2,type);
    for(int i=0;i<60;++i) game.update(.1f,{});
    for(int type=0;type<4;++type) check(game.resource(2,type)==stock[size_t(type)],
          "Loading even an empty script suppresses fallback purchases");
    check(game.productionQueueSizeForTesting(game.commandCenterId(2))==0,
          "Script and fallback skirmish controller never queue competing orders");
    check(game.initSkirmish(0x5A17,&error,false,1,2),error.c_str()); game.update(0,{});
    check(game.productionQueueSizeForTesting(game.commandCenterId(2))>0,
          "New skirmish restores its normal fallback controller");
    std::puts("PASS: original AI population classes, civilian garrison/queues, civilization and strategic-number queries");
}
