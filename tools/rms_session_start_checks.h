// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "../src/engine/rms_map.h"
#include "rms_finalization_checks.h"
#include <filesystem>
#include <fstream>

static void rmsSessionStartChecks(swgb::Assets &assets) {
    rmsFinalizationChecks();
    const auto fixtures=std::filesystem::path(__FILE__).parent_path()/"fixtures";
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
    std::puts("PASS: original starting-resource tiers and atomic multi-player AI activation");
}
