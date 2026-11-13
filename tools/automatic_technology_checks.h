// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <limits>

// Original 5bfec9/5c0d45 and effect102 dispatch fixtures are retained in
// Research/windows-reference/rms-start109-tech-*-oracle and rms-effect102-oracle.
static void automaticTechnologyChecks(swgb::Assets &assets) {
    auto &data=const_cast<swgb::dat::DatFile &>(assets.dat());
    const auto savedTechs=data.techs;
    const auto savedEffects=data.effects;
    struct Restore {
        swgb::dat::DatFile &data;
        const std::vector<swgb::dat::Tech> &techs;
        const std::vector<swgb::dat::Effect> &effects;
        ~Restore() { data.techs=techs; data.effects=effects; }
    } restore{data,savedTechs,savedEffects};
    const int first=int(data.techs.size());
    const int trigger=first, delayed=first+1, modifier=first+2;
    const int disabled=first+3, positiveTime=first+4, completed=first+5;
    const int unconditional=first+6, queued=first+7;
    for(int i=0;i<8;++i) {
        swgb::dat::Tech tech;
        for(auto &required:tech.requiredTechs) required=-1;
        tech.requiredTechCount=1;
        tech.requiredTechs[0]=int16_t(trigger);
        tech.name=tech.name2="UNNAMED-AUTOMATIC-FIXTURE";
        tech.locationId=109;
        tech.costs[0]={0,300,1};
        data.techs.push_back(tech);
    }
    data.techs[size_t(trigger)].requiredTechs[0]=-1;
    data.techs[size_t(trigger)].researchTime=10;
    data.techs[size_t(delayed)].researchTime=10;
    data.techs[size_t(positiveTime)].researchTime=10;
    data.techs[size_t(positiveTime)].name2="AVAIL-POSITIVE-TIME-FIXTURE";
    data.techs[size_t(unconditional)].requiredTechCount=0;
    swgb::dat::Effect changes;
    changes.name="Automatic completion ordering fixture";
    changes.commands={
        {1,86,0,0,2.f},
        {103,int16_t(delayed),-1,1,-10.f},
        {102,-1,-1,-1,float(disabled)+.9f},
        {102,-1,-1,-1,float(completed)}};
    data.techs[size_t(modifier)].effectId=int16_t(data.effects.size());
    data.effects.push_back(changes);
    std::string error;
    {
        swgb::Game game(assets);
        check(game.init(0x5A17,64,&error,1,1),error.c_str());
        check(game.technologyResearched(1,unconditional),
              "Zero-time requiredCount0 technology completes without a name whitelist");
        check(!game.technologyResearched(1,delayed) &&
              !game.technologyResearched(1,modifier),
              "Incomplete prerequisite blocks zero-time and positive-time technologies");
        const auto researcher=game.spawnObjectForTesting(1,109,1,12.f,12.f);
        check(researcher && game.queueTechnologyForTesting(researcher,queued),
              "Queue pending zero-time technology fixture");
        const auto disabledResearcher=game.spawnObjectForTesting(1,109,1,18.f,18.f);
        check(disabledResearcher && game.queueTechnologyForTesting(disabledResearcher,disabled),
              "Queue technology disabled by the completion effect");
        check(game.researchTechnologyForTesting(1,completed),"Complete preservation fixture");
        check(game.researchTechnologyForTesting(1,trigger),"Complete automatic research trigger");
        check(game.technologyResearched(1,modifier) &&
              game.technologyResearched(1,delayed),
              "New103 effect restarts lower-ID eligibility using base time without resource86");
        check(!game.technologyResearched(1,disabled),
              "New102 effect truncates fractional target before later zero-time completion");
        check(game.technologyResearched(1,completed),
              "Effect102 preserves already completed technology");
        check(!game.technologyResearched(1,queued),
              "Automatic refresh skips already queued technology after prerequisite completion");
        check(!game.technologyResearched(1,positiveTime),
              "AVAIL name does not auto-complete positive research time");
        game.update(0.f,{});
        check(!game.technologyResearched(1,disabled) && game.productionQueueSizeForTesting(disabledResearcher)==0,
              "Disabled research retires on its next tick without completing");
    }
    const int tree=data.civs[1].techTreeId;
    check(tree>=0 && size_t(tree)<data.effects.size(),"Civilization tree fixture exists");
    data.effects[size_t(tree)].commands.push_back({102,-1,-1,-1,65535.f});
    {
        swgb::Game game(assets);
        check(game.init(0x5A17,64,&error,1,1),error.c_str());
        check(!game.technologyResearched(1,unconditional),
              "Startup102 target65535 wraps signed16 and disables all pending technologies");
    }
    // Isolate technology0 so invalid x87 integer conversions are observable.
    data.techs[0]=data.techs[size_t(unconditional)];
    for(float target:{-1.f,std::numeric_limits<float>::quiet_NaN(),
                      std::numeric_limits<float>::infinity(),4294967296.f}) {
        data.effects[size_t(tree)].commands=savedEffects[size_t(tree)].commands;
        data.effects[size_t(tree)].commands.push_back({102,-1,-1,-1,target});
        swgb::Game game(assets);
        check(game.init(0x5A17,64,&error,1,1),error.c_str());
        check(game.technologyResearched(1,0)==(target==-1.f),
              "Literal102 minus1 skips while NaN, infinity and2^32 target technology0");
    }
    std::puts("PASS: original automatic technology gates, effect order, time modifiers and disable targets");
}
