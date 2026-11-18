// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "../src/core/genie_ai_constructor_metadata.h"
#include "../src/engine/rms_script.h"
#include <memory>
[[gnu::noinline]] static void rmsConstructorMetadataGameplayChecks(swgb::Assets &assets) {
    struct Golden {uint32_t seed,known,revisit,food;};
    const Golden goldens[]={{1,4294943043u,5612,4995},{2,9398,18739,5752},{12345,3199,35981,4685}};
    const std::string source="<LAND_GENERATION> base_terrain 0 create_player_lands { "
        "terrain_type 0 base_size 3 number_of_tiles 60 } <OBJECTS_GENERATION> "
        "create_object 109 { number_of_objects 1 set_place_for_every_player "
        "min_distance_to_players 0 max_distance_to_players 0 }";
    swgb::RmsScript script;std::string error;
    check(script.load(60000,[&](int32_t id,std::vector<uint8_t> &bytes) {
        if(id!=60000)return false;bytes.assign(source.begin(),source.end());return true;
    },{},&error),error.c_str());
    auto baseline=std::make_unique<swgb::Game>(assets);
    uint32_t baselineRng=17;
    check(baseline->initRmsSession(script,{},32,baselineRng,{1,1},{},&error),error.c_str());
    check(!baseline->explorationRevisitTimeForTesting(2) && !baseline->economyForTesting(2).foodRefreshTime &&
          !baseline->aiKnownRefreshTimeForTesting(2),"Absent session seed leaves all original constructor epochs unknown");
    auto game=std::make_unique<swgb::Game>(assets);
    const auto checkTimers=[&](const Golden &g) {
        check(game->explorationRevisitTimeForTesting(2)==g.revisit &&
              game->economyForTesting(2).foodRefreshTime==g.food && game->aiKnownRefreshTimeForTesting(2)==g.known,
              "Actual normal-session constructor goldens survive RMS initialization");
    };
    for(const auto &g:goldens) {
        uint32_t constructorState=swgb::genieNormalSessionComputerConstructorState(g.seed);
        const auto pure=swgb::geniePlayerConstructorWithMetadata(constructorState);
        check(pure.infoKnownRefreshTime==g.known && pure.infoRevisitTime==g.revisit && pure.infoFoodRefreshTime==g.food,
              "Pure separate-session replay matches original61e007 constructor timer controls");
        swgb::RmsSessionOptions options;options.sessionSeed=g.seed;uint32_t rng=17;
        check(game->initRmsSession(script,{},32,rng,{1,1},options,&error),error.c_str());
        checkTimers(g);
        check(rng==baselineRng && game->legacyRandomForTesting()==baseline->legacyRandomForTesting() &&
              game->worldCheckpointForTesting()==baseline->worldCheckpointForTesting(),
              "Session constructor replay leaves independent RMS random stream and checkpoint unchanged");
        check(!game->explorationRevisitTimeForTesting(1) && !game->aiKnownRefreshTimeForTesting(1) &&
              !game->economyForTesting(1).foodRefreshTime,"Human owner has no manufactured InfoAI constructor epoch");
        check(game->loadAiSourceForTesting(2,"retain-constructor-epoch","(defrule (true) => (disable-self))",{},&error),error.c_str());
        checkTimers(g);
        const auto count=game->activeObjectCount();const auto retainedRng=game->legacyRandomForTesting();
        auto randomSentinel=options;randomSentinel.sessionSeed=UINT32_MAX;
        uint32_t rejectedSeed=789;
        check(!game->initRmsSession(script,{},32,rejectedSeed,{1,1},randomSentinel,&error) &&
              rejectedSeed==789 && game->legacyRandomForTesting()==retainedRng &&
              game->activeObjectCount()==count && game->aiRuleCountForTesting(2)==1,
              "Original random-session sentinel rejects before changing an existing session");
        checkTimers(g);
        for(const auto &unsupported:std::vector<std::vector<uint16_t>>{{1},{1,0},{1,9},{1,1,1,1,1,1,1,1,1}}) {
            uint32_t rejectedRng=789;
            check(!game->initRmsSession(script,{},32,rejectedRng,unsupported,options,&error) &&
                  rejectedRng==789 && game->legacyRandomForTesting()==retainedRng && game->activeObjectCount()==count &&
                  game->aiRuleCountForTesting(2)==1,"Unproven session-constructor context rejects atomically");
            checkTimers(g);
        }
    }
    check(game->initAridSkirmish(2,&error),error.c_str());
    checkTimers(goldens[1]);
    check(game->loadAiScript(2,"RandomGame.per",{},&error),error.c_str());
    checkTimers(goldens[1]);

    // Actual uninterrupted61e007..61e119, complete594f00 return and outer reseed.
    // Columns are known-category, revisit, food; rows are computer owners2..8.
    const uint32_t ownerGoldens[3][7][3]={
        {{4294943043u,5612,4995},{4294941020u,52378,169},{52603,24101,2077},
         {65411,567,7033},{4294910518u,48110,799},{4294958118u,40917,5594},{4294939651u,17124,494}},
        {{9398,18739,5752},{43859,19466,5481},{4294908897u,25086,4846},
         {52992,25291,4418},{4600,21960,1778},{45081,55471,809},{32873,40896,4877}},
        {{3199,35981,4685},{4294921102u,23321,1101},{4294950912u,52025,4333},
         {4294924228u,51357,2471},{4294955380u,30412,6243},{4294964906u,37427,22},{4445,52807,30}}
    };
    for(size_t seedIndex=0;seedIndex<3;++seedIndex) {
        uint32_t state=swgb::genieNormalSessionComputerConstructorState(goldens[seedIndex].seed);
        for(size_t owner=2;owner<=8;++owner) {
            const auto metadata=swgb::geniePlayerConstructorWithMetadata(state);
            const auto &expected=ownerGoldens[seedIndex][owner-2];
            check(metadata.infoKnownRefreshTime==expected[0] && metadata.infoRevisitTime==expected[1] &&
                  metadata.infoFoodRefreshTime==expected[2],"Every original computer constructor preserves its owner-index RNG epoch");
        }
    }
    for(size_t count=3;count<=8;++count) {
        const size_t seedIndex=(count-3)%3;
        std::vector<uint16_t> civs;
        for(size_t i=0;i<count;++i)civs.push_back(uint16_t(count%2 ? 8-i : i+1));
        uint32_t unseededRng=17;
        check(baseline->initRmsSession(script,{},64,unseededRng,civs,{},&error),error.c_str());
        swgb::RmsSessionOptions options;options.sessionSeed=goldens[seedIndex].seed;
        uint32_t seededRng=17;
        check(game->initRmsSession(script,{},64,seededRng,civs,options,&error),error.c_str());
        check(seededRng==unseededRng && game->legacyRandomForTesting()==baseline->legacyRandomForTesting() &&
              game->worldCheckpointForTesting()==baseline->worldCheckpointForTesting(),
              "Mixed-civilization constructor metadata does not consume the map random stream");
        const auto checkOwner=[&](int owner) {
            const auto &expected=ownerGoldens[seedIndex][owner-2];
            check(game->aiKnownRefreshTimeForTesting(owner)==expected[0] &&
                  game->explorationRevisitTimeForTesting(owner)==expected[1] &&
                  game->economyForTesting(owner).foodRefreshTime==expected[2],
                  "Mixed-civilization RMS installs each computer's original constructor epochs");
        };
        for(size_t owner=2;owner<=count;++owner) {
            checkOwner(int(owner));
            check(!baseline->aiKnownRefreshTimeForTesting(int(owner)) &&
                  !baseline->explorationRevisitTimeForTesting(int(owner)) &&
                  !baseline->economyForTesting(int(owner)).foodRefreshTime,
                  "Omitted session context leaves every computer constructor epoch unknown");
        }
        check(!game->aiKnownRefreshTimeForTesting(1) && !game->explorationRevisitTimeForTesting(1) &&
              !game->economyForTesting(1).foodRefreshTime,"Non-Empire humans also have no InfoAI constructor epoch");
        check(game->loadAiSourceForTesting(int(count),"retain-last-owner-epoch",
              "(defrule (true) => (disable-self))",{},&error),error.c_str());
        checkOwner(int(count));
    }
}
