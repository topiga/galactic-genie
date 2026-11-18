// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "../src/engine/rms_map.h"
#include "rms_finalization_checks.h"
#include "empty_owner_gameplay_checks.h"
#include <filesystem>
#include <fstream>
#include <chrono>

static void rmsStartCameraChecks(swgb::Assets &assets) {
    // Three original5d1110/4af610 controls, positions in original world axes.
    const std::vector<std::vector<std::array<int,3>>> cases={
        {{83,5,5},{109,80,80},{109,20,30}},{{109,20,30},{109,30,20}},{{83,70,60},{293,5,5}}};
    const uint32_t selected[]={3,1,1};size_t index=0;
    for(const auto &entries:cases) {
        swgb::Scenario scenario;scenario.map.width=scenario.map.height=128;scenario.map.tiles.resize(128*128);
        scenario.playerSlotCount=1;scenario.players[0].active=scenario.players[0].human=true;
        scenario.players[0].civilization=1;scenario.civilizations.fill(1);scenario.nextUnitId=1;
        for(const auto &entry:entries){swgb::ScenarioUnit unit;unit.unitId=entry[0];unit.x=entry[1];unit.y=entry[2];
            unit.player=1;unit.state=2;unit.spawnId=scenario.nextUnitId++;scenario.units.push_back(unit);}
        swgb::Game game(assets);std::string error;check(game.initScenario(scenario,&error),error.c_str());
        const auto random=game.legacyRandomForTesting();game.focusRmsStartForTesting();const auto camera=game.cameraForTesting();
        check(game.lookAtObject(selected[index++]) && game.cameraForTesting()==camera && game.legacyRandomForTesting()==random,
              "Original starting camera prefers nearest109, preserves ties, otherwise uses first primary object without RNG");
    }
}

static void rmsSessionStartChecks(swgb::Assets &assets) {
    rmsFinalizationChecks();
    emptyOwnerGameplayChecks(assets);
    rmsStartCameraChecks(assets);
    const auto fixtures=std::filesystem::absolute(std::filesystem::path(__FILE__).parent_path()/"fixtures");
    swgb::RmsScript script;std::string error;
    std::ifstream input(fixtures/"owned-scouts.rms");
    check(bool(input),"Read session RMS fixture");
    const std::string source((std::istreambuf_iterator<char>(input)),std::istreambuf_iterator<char>());
    check(script.load(60000,[&](int32_t id,std::vector<uint8_t> &bytes) {
        if(id!=60000)return false;bytes.assign(source.begin(),source.end());return true;
    },{},&error),error.c_str());
    auto &data=const_cast<swgb::dat::DatFile &>(assets.dat());
    struct Restore {
        swgb::dat::DatFile &data;std::vector<float> one,six;
        ~Restore(){data.civs[1].resources=std::move(one);data.civs[6].resources=std::move(six);}
    } restore{data,data.civs[1].resources,data.civs[6].resources};
    const float offsets[]={1,-25,3,4};
    for(int civ:{1,6})for(int type=0;type<4;++type)data.civs[civ].resources[91+type]=offsets[type];
    const float expected[][4]={{201,175,253,104},{201,175,253,104},
                              {501,475,453,304},{1001,975,853,704}};
    swgb::Game game(assets);
    swgb::RmsSessionOptions options;
    for(uint32_t tier=0;tier<4;++tier)for(uint32_t map:{0u,37u}) {
        options.resourceTier=tier;options.mapId=map;uint32_t rng=1;
        check(game.initRmsSession(script,{},32,rng,{6,1,6},options,&error),error.c_str());
        for(int player=1;player<=3;++player) {
            for(int type=0;type<4;++type) {
                const float wanted=map==37 && type==1?std::max(expected[tier][type],475.f):
                    expected[tier][type]+(map==37 && type==2?100.f:0.f);
                check(game.resource(player,type)==wanted,"Original starting-resource offsets, tier overwrite and map37 order");
            }
            check(game.resource(player,98)==575 && game.resource(player,164)==200 && game.resource(player,165)==375,
                  "Session funds preserve RMS starting-object cost counters");
        }
    }
    options={};options.aiScripts={"",(fixtures/"session-startup.per").string(),(fixtures/"session-startup.per").string()};
    uint32_t rng=1;
    check(game.initRmsSession(script,{},32,rng,{6,1,6},options,&error),error.c_str());
    check(game.worldCheckpointForTesting().has_value() && rng==game.legacyRandomForTesting(),
          "RMS session retains a distinct finalized world seed and exposes its ambient CRT");
    check(game.aiRuleCountForTesting(1)==0,"Session never activates AI for the human slot");
    for(int player=2;player<=3;++player)
        check(game.aiRuleCountForTesting(player)==3 && game.aiGoalForTesting(player,901)==0 &&
              game.resource(player,0)==201,"AI is loaded after initialization without evaluating during construction");
    game.update(0,{});
    for(int player=2;player<=3;++player)
        check(game.aiGoalForTesting(player,901)==1 && game.aiGoalForTesting(player,902)==1 &&
              game.resource(player,0)==151,"First AI evaluation observes initialized age/funds and pays for one worker");
    const auto objectCount=game.activeObjectCount();
    const auto valid=options;
    for(int invalid=0;invalid<5;++invalid) {
        options=valid;
        if(invalid==0)options.resourceTier=4;
        if(invalid==1)options.startingAge=1;
        if(invalid==2)options.aiScripts[0]=options.aiScripts[1];
        if(invalid==3)options.aiScripts.pop_back();
        if(invalid==4)options.aiScripts[2]=(fixtures/"missing-session-ai.per").string();
        rng=12345;
        check(!game.initRmsSession(script,{},32,rng,{6,1,6},options,&error) && rng==12345 &&
              game.activeObjectCount()==objectCount && game.resource(2,0)==151 && game.aiGoalForTesting(2,901)==1,
              "Invalid session settings or later AI file preserve the running world and earlier AI state");
    }
    options=valid;options.startingAge=3;rng=1;
    check(game.initRmsSession(script,{},32,rng,{6,1,6},options,&error),error.c_str());
    check(game.aiGoalForTesting(2,903)==0 && game.resource(2,0)==201,
          "Higher-age research completes before AI activation without charging starting food");
    game.update(0,{});
    check(game.aiGoalForTesting(2,901)==0 && game.aiGoalForTesting(2,903)==1 && game.resource(2,0)==151,
          "First AI pass observes the selected starting age and initialized funds");
    options=valid;options.aiScripts[1]="RandomGame.per";rng=1;
    check(game.initRmsSession(script,{},32,rng,{6,1,6},options,&error),error.c_str());
    check(game.aiRuleCountForTesting(2)>3 && game.aiRuleCountForTesting(3)==3 && game.aiGoalForTesting(3,901)==0,
          "RMS session loads packaged RandomGame includes alongside a filesystem personality without evaluation");
    check(game.loadAiScript(3,"RandomGame.per",options.aiDefines,&error),error.c_str());
    const auto packagedCount=game.aiRuleCountForTesting(3);
    check(packagedCount>3,"Public AI loading resolves the packaged original personality");
    check(!game.loadAiScript(3,(fixtures/"missing"/"RandomGame.per").string(),options.aiDefines,&error) &&
          game.aiRuleCountForTesting(3)==packagedCount,"Explicit missing paths do not fall back to a same-named package or replace running AI");
    {
        const auto directory=std::filesystem::temp_directory_path()/
            ("swgb-session-ai-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        const bool created=std::filesystem::create_directory(directory);
        check(created,"Create isolated local personality fixture");
        if(!created)return;
        struct RestoreDirectory {
            std::filesystem::path previous,temporary;
            ~RestoreDirectory(){std::error_code ignored;std::filesystem::current_path(previous,ignored);std::filesystem::remove_all(temporary,ignored);}
        } restoreDirectory{std::filesystem::current_path(),directory};
        {std::ofstream file(directory/"RandomGame.per");file<<"(load \"local-child\")";}
        {std::ofstream file(directory/"local-child.per");file<<"(defrule (true) => (set-goal 920 1))";}
        std::filesystem::current_path(directory);
        check(game.loadAiScript(3,"RandomGame.per",options.aiDefines,&error) && game.aiRuleCountForTesting(3)==1,
              "Existing local entry wins over packaged name and keeps relative filesystem includes");
        {std::ofstream file("RandomGame.per");file<<"(defrule";}
        check(!game.loadAiScript(3,"RandomGame.per",options.aiDefines,&error) &&
              error.find("unterminated list")!=std::string::npos && game.aiRuleCountForTesting(3)==1,
              "Malformed local personality is reported without package fallback or AI replacement");
        const auto objects=game.activeObjectCount();const auto money=game.resource(2,0);rng=12345;
        check(!game.initRmsSession(script,{},32,rng,{6,1,6},options,&error) && rng==12345 &&
              error.find("unterminated list")!=std::string::npos && game.activeObjectCount()==objects &&
              game.resource(2,0)==money && game.aiRuleCountForTesting(3)==1,
              "Malformed local RMS personality preserves the existing world, RNG and other AI");
    }
    std::puts("PASS: original starting-resource tiers and atomic multi-player AI activation");
}
