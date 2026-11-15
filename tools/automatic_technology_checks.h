// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <limits>

// Original 5bfec9/5c0d45 and effect102 dispatch fixtures are retained in
// Research/windows-reference/rms-start109-tech-*-oracle and rms-effect102-oracle.
static void automaticTechnologyChecks(swgb::Assets &assets) {
    // Full installed original graph: fresh player, then only Command Center marker22.
    const std::vector<std::vector<int>> originalCompleted={
        {0,22,451,486},{0,22,452,486,493},{0,22,454,486},{0,22,453,486},
        {0,22,486},{0,22,455,486},{0,22,387,486},{0,22,388,486}};
    for (int civ=1;civ<=8;++civ) {
        swgb::Scenario scenario;
        scenario.map.width=scenario.map.height=24;
        scenario.map.tiles.resize(24*24);
        scenario.nextUnitId=2;
        scenario.players[0].active=scenario.players[0].human=true;
        scenario.players[0].civilization=scenario.civilizations[0]=civ;
        scenario.players[0].populationLimit=500;
        swgb::Game game(assets); std::string error;
        for (bool withCenter:{false,true}) {
            if (withCenter) {
                swgb::ScenarioUnit center;
                center.unitId=109; center.player=1; center.spawnId=1;
                center.x=center.y=8; center.state=2;
                scenario.units.push_back(center);
            }
            check(game.initScenario(scenario,&error),error.c_str());
            std::vector<int> actual;
            for (size_t id=0;id<assets.dat().techs.size();++id)
                if (game.technologyResearched(1,int(id))) actual.push_back(int(id));
            const std::vector<int> initial=civ==2?std::vector<int>{0,486,493}:std::vector<int>{0,486};
            check(actual==(withCenter?originalCompleted[size_t(civ-1)]:initial),
                  "Installed civilization startup and Command Center technology cascade match original graph");
            auto expectedOrder=initial;
            if (withCenter) {
                expectedOrder.push_back(22);
                const int unlock[]={451,452,454,453,-1,455,387,388};
                if (unlock[civ-1]>=0) expectedOrder.push_back(unlock[civ-1]);
            }
            check(game.technologyOrderForTesting(1)==expectedOrder,
                  "Original player startup research precedes Command Center completion cascade");
            check(game.resource(1,21)==(civ==2?2.f:1.f)+(withCenter && civ!=5?1.f:0.f),
                  "Original visible automatic research counter excludes hidden technologies");
            // Trade Federation tree effect8 sets250 AFTER buildings; do not add the starting5 again.
            check(game.populationCapacityForTesting(1)==(civ==5?250.f:withCenter?5.f:0.f) &&
                  game.populationUsedForTesting(1)==0,
                  "Civilization capacity and Command Center storage are counted exactly once");
            check(game.unitAvailableForTesting(1,70)==(civ!=5),
                  "Ordinary TF tree disables housing after initial technology0 enables it");
            if (withCenter) {
                const float capacity=game.populationCapacityForTesting(1);
                check(game.spawnObjectForTesting(civ,109,1,16,16)!=0 &&
                      game.populationCapacityForTesting(1)==capacity+5,
                      "A later Command Center grants capacity after the startup bonus");
            }
        }
    }
    auto &data=const_cast<swgb::dat::DatFile &>(assets.dat());
    const auto savedTechs=data.techs;
    const auto savedEffects=data.effects;
    struct Restore {
        swgb::dat::DatFile &data;
        const std::vector<swgb::dat::Tech> &techs;
        const std::vector<swgb::dat::Effect> &effects;
        ~Restore() { data.techs=techs; data.effects=effects; }
    } restore{data,savedTechs,savedEffects};
    {
        const int first=int(data.techs.size());
        for(int location:{-1,109}) for(int button:{0,1}) {
            swgb::dat::Tech technology;
            for(auto &required:technology.requiredTechs) required=-1;
            technology.researchTime=10;technology.locationId=int16_t(location);technology.buttonId=uint8_t(button);
            data.techs.push_back(technology);
        }
        swgb::Scenario scenario;scenario.map.width=scenario.map.height=24;scenario.map.tiles.resize(24*24);
        scenario.players[0].active=scenario.players[0].human=true;
        scenario.players[0].civilization=scenario.civilizations[0]=1;
        swgb::Game game(assets);std::string error;
        check(game.initScenario(scenario,&error),error.c_str());
        for(int offset=0;offset<4;++offset) {
            const auto before=game.resource(1,21);
            check(game.researchTechnology(1,first+offset) && game.resource(1,21)==before+(offset==3?1.f:0.f),
                  "Original research counter requires both location and button, not an icon");
            const auto completed=game.resource(1,21);
            check(game.researchTechnology(1,first+offset) && game.resource(1,21)==completed,
                  "Repeated research completion does not count twice");
        }
        data.techs=savedTechs;
    }
    {
        struct CounterCase { swgb::dat::Tech::Cost costs[3]; float expected; bool overflow=false; };
        const CounterCase cases[]={
            {{{0,100,1},{1,50,1},{3,25,1}},175},
            {{{0,100,1},{1,50,0},{3,25,1}},125},
            {{{-1,100,1},{1,-5,1},{3,25,1}},20},
            {{{0,100,1},{1,50,1},{3,25,1}},-32594,true},
        };
        for(const auto &c:cases) {
            data.techs=savedTechs;data.effects=savedEffects;
            for(size_t i=0;i<3;++i) data.techs[486].costs[i]=c.costs[i];
            const int manual=int(data.techs.size());
            auto technology=data.techs[486];technology.researchTime=10;technology.effectId=-1;
            for(auto &required:technology.requiredTechs) required=-1;
            technology.requiredTechCount=0;data.techs.push_back(technology);
            if(c.overflow) {
                auto &commands=data.effects[size_t(data.civs[1].techTreeId)].commands;
                commands.push_back({101,486,0,1,32767});
                commands.push_back({101,int16_t(manual),0,1,32767});
                commands.push_back({1,85,0,0,0});
            }
            swgb::Scenario scenario;scenario.map.width=scenario.map.height=24;scenario.map.tiles.resize(24*24);
            scenario.players[0].active=scenario.players[0].human=true;
            scenario.players[0].civilization=scenario.civilizations[0]=1;
            swgb::Game game(assets);std::string error;
            check(game.initScenario(scenario,&error),error.c_str());
            check(game.resource(1,21)==1 && game.resource(1,99)==c.expected,
                  "Original automatic research statistic ignores disabled/invalid slots and narrows signed cost words");
            check(game.researchTechnology(1,manual) && game.resource(1,21)==2 && game.resource(1,99)==2*c.expected,
                  "Explicit research uses the same original completion accounting");
            check(game.researchTechnology(1,manual) && game.resource(1,99)==2*c.expected,
                  "Repeated completion does not add research cost twice");
        }
        data.techs=savedTechs;data.effects=savedEffects;
    }
    {
        const int dependent=int(data.techs.size());
        swgb::dat::Tech afterBuilding,initial;
        for(auto &required:afterBuilding.requiredTechs) required=-1;
        afterBuilding.requiredTechCount=1;
        afterBuilding.requiredTechs[0]=data.civs[1].units[109].techId;
        initial=afterBuilding;
        initial.requiredTechCount=0;
        initial.effectId=int16_t(data.effects.size());
        swgb::dat::Effect restriction;
        restriction.commands={{102,-1,-1,-1,float(dependent)}};
        data.effects.push_back(restriction);
        data.techs.push_back(afterBuilding);
        data.techs.push_back(initial);
        swgb::Game game(assets); std::string error;
        swgb::Scenario scenario;
        scenario.map.width=scenario.map.height=24;
        scenario.map.tiles.resize(24*24);
        scenario.players[0].active=scenario.players[0].human=true;
        scenario.players[0].civilization=scenario.civilizations[0]=1;
        swgb::ScenarioUnit center;
        center.unitId=109;center.player=1;center.spawnId=1;center.x=center.y=8;center.state=2;
        scenario.units.push_back(center);scenario.nextUnitId=2;
        check(game.initScenario(scenario,&error),error.c_str());
        check(game.technologyResearched(1,dependent+1) && !game.technologyResearched(1,dependent),
              "Initial research restriction must precede imported building completion");
        check(game.init(1,64,&error,1,1),error.c_str());
        check(game.technologyResearched(1,dependent+1) && !game.technologyResearched(1,dependent),
              "Initial research restriction must precede fresh starting buildings");
        data.techs=savedTechs;data.effects=savedEffects;
    }
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
    data.techs[size_t(unconditional)].name2="MADE-NAME-DOES-NOT-CONTROL-ELIGIBILITY";
    const int buildingMarker=data.civs[1].units[109].techId;
    check(buildingMarker>=0,"Command Center has a DAT completion technology");
    data.techs[size_t(buildingMarker)].name2="RENAMED-COMMAND-CENTER-MARKER";
    const int impostor=int(data.techs.size());
    auto namedLikeBuilding=data.techs[size_t(trigger)];
    namedLikeBuilding.name2="MADE-"+data.civs[1].units[109].name2;
    data.techs.push_back(namedLikeBuilding);
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
        check(game.technologyResearched(1,buildingMarker),
              "Building initiates its DAT technology even when the marker is renamed");
        check(!game.technologyResearched(1,impostor),
              "Matching a building name does not grant an unrelated technology");
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
              !game.technologyResearched(1,delayed),
              "Ordinary103 effect does not change automatic eligibility");
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
    data.effects[size_t(tree)].commands.push_back({103,int16_t(delayed),-1,1,-10.f});
    {
        swgb::Game game(assets);
        check(game.init(0x5A17,64,&error,1,1),error.c_str());
        check(game.researchTechnologyForTesting(1,trigger),"Complete startup restriction trigger");
        check(game.technologyResearched(1,delayed),
              "Startup103 restriction changes eligibility without resource86 scaling");
    }
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
