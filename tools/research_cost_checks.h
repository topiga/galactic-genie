// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

static void researchCostChecks(swgb::Assets &assets) {
    swgb::Game game(assets); std::string error;
    check(game.initSkirmish(0x5A17,&error,false,1,1),error.c_str());
    check(game.researchTechnologyForTesting(1,501),"Research modifier fixture");
    auto &data=const_cast<swgb::dat::DatFile &>(assets.dat());
    auto &commands=data.effects[size_t(data.techs[501].effectId)].commands;
    auto &tech=data.techs[174];
    struct Restore {
        std::vector<swgb::dat::EffectCommand> &commands;
        std::vector<swgb::dat::EffectCommand> saved;
        swgb::dat::Tech &tech, &age;
        swgb::dat::Tech savedTech,savedAge;
        ~Restore() { commands=std::move(saved); tech=std::move(savedTech); age=std::move(savedAge); }
    } restore{commands,commands,tech,data.techs[1],tech,data.techs[1]};
    tech.costs[0]={0,100,1}; tech.costs[1]={0,70,1}; tech.costs[2]={3,60,0};
    tech.researchTime=30;
    auto effect=[](int type,int target,int resource,int mode,float value) {
        swgb::dat::EffectCommand c{};
        c.type=type;c.a=target;c.b=resource;c.c=mode;c.d=value;return c;
    };
    commands={effect(101,174,0,1,-20.9f),effect(101,174,0,1,-10.9f)};
    auto costs=game.researchCostsForTesting(1,tech);
    check(costs[0].amount==90 && costs[1].amount==70 && costs[2].amount==60,
          "Research deltas overwrite, truncate and affect only first enabled matching slot");
    commands={effect(101,174,0,256,33.9f)};
    check(game.researchCostsForTesting(1,tech)[0].amount==33,"Research set mode uses low byte");
    commands.push_back(effect(1,85,0,0,.5f));
    check(game.researchCostsForTesting(1,tech)[0].amount==16,"Research scaling truncates without rounding");
    data.techs[1].costs[0]={0,100,1};
    check(game.researchCostsForTesting(1,data.techs[1])[0].amount==100,"Age research bypasses resource85");
    commands={effect(101,174,0,1,32767),effect(1,85,0,0,0)};
    check(game.researchCostsForTesting(1,tech)[0].amount==32867,"Unscaled base plus delta remains wider than short");
    commands.push_back(effect(1,85,0,0,1));
    check(game.researchCostsForTesting(1,tech)[0].amount==-32669,"Positive multiplier narrows to signed short");
    commands={effect(103,174,0,1,-10.9f),effect(103,174,0,1,-5.9f),effect(1,86,0,0,.5f)};
    check(game.researchDurationForTesting(1,tech)==10,"Time delta replaces earlier delta and follows base scaling");
    commands[1]=effect(103,174,0,256,10.9f);
    check(game.researchDurationForTesting(1,tech)==0,"Research time set mode subtracts unscaled base");
    commands={effect(103,174,0,1,2.9f),effect(1,86,0,0,.25f)};
    check(game.researchDurationForTesting(1,tech)==9.5f,"Research duration preserves fractional threshold");
    commands={effect(1,86,0,0,-1)};
    check(game.researchDurationForTesting(1,tech)==30,"Nonpositive research multiplier is neutral");
    commands={effect(101,174,0,0,25),effect(1,85,0,0,.5f)};
    const auto center=game.commandCenterId(1);
    check(game.queueTechnologyForTesting(center,174),"Queue research refund fixture");
    const float food=game.resource(1,0);
    commands[0]=effect(101,174,0,0,35);
    check(game.cancelProductionForTesting(center),"Cancel research refund fixture");
    check(game.resource(1,0)==food+17+35,
          "Research cancellation recalculates current costs including duplicate resource slot");
    std::puts("PASS: original research cost deltas, integer boundaries, age exemption and time thresholds");
}
