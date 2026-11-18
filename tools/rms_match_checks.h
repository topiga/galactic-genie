// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "../src/engine/rms_script.h"

static void rmsMatchChecks(swgb::Assets &assets) {
    std::string error;std::vector<uint8_t> bytes{1};
    check(!assets.readRmsResource(-1,bytes,&error) && bytes.empty(),"Invalid packaged RMS read clears output");
    check(assets.readRmsResource(54201,bytes,&error) && !bytes.empty(),"Read original Arid from installed game data");
    swgb::RmsScript script;
    check(script.load(54201,[&](int32_t id,std::vector<uint8_t> &out) {
        return assets.readRmsResource(id,out,&error);
    },{},&error),error.c_str());
    const std::unordered_set<std::string> defines{
        "TINY_MAP","FIXED_POSITIONS","DEFAULT_RESOURCES","RANDOM_MAP","2_PLAYERS"};
    swgb::Game reference(assets),game(assets);uint32_t rng=1;
    check(reference.initRmsSession(script,defines,120,rng,{1,1},{},&error),error.c_str());
    check(game.initAridSkirmish(1,&error,false,75,true),error.c_str());
    check(game.aiRuleCountForTesting(2)>0,"Arid Conquest loads the original personality during normal startup");
    check(game.skirmishActive() && game.matchResult()==-1 && game.matchTime()==0,
          "Packaged Arid enters a real playable match lifecycle");
    check(game.worldCheckpointForTesting()==reference.worldCheckpointForTesting() && game.legacyRandomForTesting()==rng,
          "Match activation preserves original RMS session checkpoint and ambient RNG");
    check(game.activeObjectCount()==reference.activeObjectCount(),"Arid match adds no custom opening units or resource nodes");
    for(int player=1;player<=2;++player) {
        check(game.commandCenterId(player)!=0 && game.objectUnitId(game.commandCenterId(player))==109,
              "Match victory centers resolve original owned Command Centers");
        check(game.objectCountForTesting(player,109)==1 &&
              game.objectCountForTesting(player,83)+game.objectCountForTesting(player,293)==3 &&
              game.objectCountForTesting(player,550)==1,"Original Arid opening has a CC, three workers and scout per player");
        const float normal[]={200,200,250,100};
        for(int type=0;type<4;++type)
            check(game.resource(player,type)==normal[type] && game.resource(player,type)==reference.resource(player,type),
                  "Arid uses normal original starting resources, without custom skirmish grants");
    }
    const auto checkpoint=game.worldCheckpointForTesting();const auto ambient=game.legacyRandomForTesting();
    const auto center=game.objectPosition(game.commandCenterId(1));
    game.updatePaused(.5f,{});
    check(game.matchTime()==0 && game.worldCheckpointForTesting()==checkpoint && game.legacyRandomForTesting()==ambient &&
          game.objectPosition(game.commandCenterId(1))==center,"Paused RMS match does not advance simulation or AI");
    check(game.tileVisibilityForTesting(0,0)==0,"Standard RMS reveal leaves distant terrain unexplored");
    check(!game.initAridSkirmish(1,&error,false,74,true) && game.worldCheckpointForTesting()==checkpoint &&
          game.objectPosition(game.commandCenterId(1))==center,"Invalid match settings fail before replacing the running world");
    game.resignSkirmish();check(game.matchResult()==0,"RMS match resignation uses the shared defeat lifecycle");
    check(game.initAridSkirmish(1,&error,true,100,false),error.c_str());
    check(game.matchResult()==-1 && game.matchTime()==0 && game.objectPosition(game.commandCenterId(1))==center &&
          game.worldCheckpointForTesting()==checkpoint,"Restarting the same RMS seed preserves its opening and resets the result");
    check(game.aiRuleCountForTesting(2)==0,"Arid Command Center objective retains the native fallback opponent");
    check(game.tileVisibilityForTesting(0,0)>=1,"Explored reveal survives RMS match initialization");
    // Probe the normal runtime facts after opening assertions, without adding units/resources.
    const int headroom=100-int(game.populationCapacityForTesting(1));
    const std::string probe="(defrule (population-headroom == "+std::to_string(headroom)+
        ") (difficulty == moderate) (map-type desert) (not (map-type forest)) => (set-goal 925 1) (disable-self))";
    check(game.loadAiSourceForTesting(1,"rms-match-settings",probe,{},&error),error.c_str());
    game.update(0,{});
    check(game.aiGoalForTesting(1,925)==1,"RMS match runtime observes selected population limit and moderate difficulty");
    const auto opening=game.populationCapacityForTesting(1);
    const auto used=game.populationUsedForTesting(1);
    for(int i=0;i<21;++i)
        check(game.spawnObjectForTesting(1,70,1,20.f+2*(i%7),20.f+2*(i/7))!=0,"Create extra prefabs");
    const int built=int(opening+21*5);
    const std::string overbuilt="(defrule (housing-headroom == "+std::to_string(int(built-used))+
        ") (population-headroom == "+std::to_string(100-built)+") => (set-goal 925 2))";
    check(game.loadAiSourceForTesting(1,"extra-housing",overbuilt,{},&error),error.c_str());
    game.update(0,{});
    check(game.aiGoalForTesting(1,925)==2 && game.populationCapacityForTesting(1)==100,
          "Surplus housing remains visible to PER while the playable population cap stays at 100");
    std::puts("PASS: packaged Arid match lifecycle, original opening, settings, checkpoint, pause and restart");
}
