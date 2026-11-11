// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

static void unitCostChecks(swgb::Assets &assets) {
    swgb::Game game(assets); std::string error;
    check(game.initSkirmish(0x5A17,&error,false,1,1),error.c_str());
    check(game.researchTechnologyForTesting(1,501),"Cost effect fixture research");
    auto &data=const_cast<swgb::dat::DatFile &>(assets.dat());
    auto &commands=data.effects[size_t(data.techs[501].effectId)].commands;
    const auto original=commands;
    struct Restore {
        std::vector<swgb::dat::EffectCommand> &target;
        std::vector<swgb::dat::EffectCommand> saved;
        ~Restore() { target=std::move(saved); }
    } restore{commands,original};
    auto unit=data.civs[1].units[83];
    unit.costs={{{0,10,1},{1,20,1},{3,30,256}}}; unit.trainTime=10;
    auto effect=[&](int type,int attribute,float value) {
        swgb::dat::EffectCommand command{};
        command.type=type; command.a=83; command.b=-1; command.c=attribute; command.d=value;
        return command;
    };
    commands={effect(5,100,1.05f),effect(5,100,1.05f),effect(5,101,1.05f),effect(5,101,1.05f)};
    auto costs=game.unitCostsForTesting(1,unit);
    check(costs[0].amount==10 && costs[1].amount==22 && costs[2].amount==30,
          "Cost multipliers round each signed-short command using the stored float factor and low flag byte");
    check(game.unitTrainingTimeForTesting(1,unit)==10,"Training-time effects use the same per-command rounding");
    commands={effect(0,103,8.9f),effect(4,103,-1.9f),effect(4,104,2.9f),effect(0,105,99)};
    costs=game.unitCostsForTesting(1,unit);
    check(costs[0].amount==7 && costs[1].amount==22 && costs[2].amount==30,
          "Specific resource effects truncate set/add values and leave disabled costs unchanged");
    unit.costs={{{3,10,1},{2,20,1},{0,30,1}}};
    commands={effect(0,105,7),effect(0,106,9)};
    costs=game.unitCostsForTesting(1,unit);
    check(costs[0].amount==7 && costs[1].amount==9 && costs[2].amount==30,
          "Resource105 modifies nova and106 modifies ore");
    commands={effect(5,100,.5f)};
    const float food=game.resource(1,0);
    check(game.loadAiSourceForTesting(1,"discount-purchase",
          "(defrule (true) => (train UNIT-WORKER) (disable-self))",{},&error),error.c_str());
    game.update(0,{});
    check(game.productionQueueSizeForTesting(game.commandCenterId(1))==1 &&
          game.resource(1,0)==food-25,"AI purchase pays modified worker cost through the real queue");
    commands={effect(5,100,.8f)};
    check(game.cancelProductionForTesting(game.commandCenterId(1)),"Cancel discounted purchase");
    check(game.resource(1,0)==food-25+40,"Original cancellation refunds current modified costs");
    std::puts("PASS: original cost/time arithmetic, resource mapping, AI payment and current-cost refunds");
}
