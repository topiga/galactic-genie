// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "../src/engine/rms_map.h"
#include "../src/core/genie_random.h"

static void startingAgeChecks(swgb::Assets &assets) {
    // Actual original601f90..6026be normal-mode controls, civ6/1, raw ages3..6.
    // Successful5bfec9 entries (priorstatus0..2) include automatic descendants in order.
    // Source: retained rms-starting-age-v6-oracle-2026-11-16.json.
    // Full original completed-state sets equal the four startup IDs plus these sequences.
    struct Case {int age;std::vector<int> first,second;std::array<int,2> researchCount;int centerHp;std::array<int,2> scoutHp;};
    const Case cases[]={
        {3,{27,209,31,235,532,13,436,6,15,437,14,438,207,215,37,28,473,33,1,54,208,400,415,478,482},{27,209,31,85,532,13,436,6,15,437,14,438,207,215,5,28,439,33,1,38,114,396,411,478,482,525},{11,12},2500,{60,60}},
        {4,{27,209,31,235,532,13,436,6,15,437,14,438,207,215,37,28,473,33,1,54,208,400,415,478,482,24,26,7,122,446,30,285,29,214,221,61,63,64,65,66,82,126,128,134,149,160,171,188,197,245,265,2,270,275,395,405,479,536},{27,209,31,85,532,13,436,6,15,437,14,438,207,215,5,28,439,33,1,38,114,396,411,478,482,525,24,26,7,122,446,30,281,29,210,217,61,63,64,65,66,82,126,128,134,149,160,171,188,197,241,261,2,266,271,391,401,479},{37,38},2500,{60,60}},
        {5,{27,209,31,235,532,13,436,6,15,437,14,438,207,215,37,28,473,33,1,54,208,400,415,478,482,24,26,7,122,446,30,285,29,214,221,61,63,64,65,66,82,126,128,134,149,160,171,188,197,245,265,2,270,275,395,405,479,536,19,465,471,25,8,32,320,325,330,105,360,365,531,70,71,74,75,79,80,83,129,130,135,144,150,151,153,161,163,172,173,174,175,176,177,183,190,198,199,226,240,250,255,445,480,483,535,569,570,573,3,280,287,410,460},{27,209,31,85,532,13,436,6,15,437,14,438,207,215,5,28,439,33,1,38,114,396,411,478,482,525,24,26,7,122,446,30,281,29,210,217,61,63,64,65,66,82,126,128,134,149,160,171,188,197,241,261,2,266,271,391,401,479,19,461,466,25,8,32,316,321,326,105,356,361,526,70,71,74,75,79,80,83,129,130,135,144,150,151,153,161,163,172,173,174,175,176,177,183,190,198,199,222,246,251,441,480,483,535,569,570,573,3,131,227,276,406,456},{87,88},2750,{60,60}},
        {6,{27,209,31,235,532,13,436,6,15,437,14,438,207,215,37,28,473,33,1,54,208,400,415,478,482,24,26,7,122,446,30,285,29,214,221,61,63,64,65,66,82,126,128,134,149,160,171,188,197,245,265,2,270,275,395,405,479,536,19,465,471,25,8,32,320,325,330,105,360,365,531,70,71,74,75,79,80,83,129,130,135,144,150,151,153,161,163,172,173,174,175,176,177,183,190,198,199,226,240,250,255,445,480,483,535,569,570,573,3,280,287,410,460,67,68,69,73,76,77,81,127,133,136,139,141,143,146,147,148,155,162,169,178,179,180,181,186,193,196,202,203,205,206,260,310,335,345,350,380,370,385,375,435,447,477,481,484,500,571,572},{27,209,31,85,532,13,436,6,15,437,14,438,207,215,5,28,439,33,1,38,114,396,411,478,482,525,24,26,7,122,446,30,281,29,210,217,61,63,64,65,66,82,126,128,134,149,160,171,188,197,241,261,2,266,271,391,401,479,19,461,466,25,8,32,316,321,326,105,356,361,526,70,71,74,75,79,80,83,129,130,135,144,150,151,153,161,163,172,173,174,175,176,177,183,190,198,199,222,246,251,441,480,483,535,569,570,573,3,131,227,276,406,456,67,68,69,72,81,127,136,139,141,147,152,154,156,157,158,162,164,166,168,170,178,179,180,181,186,193,194,196,203,205,206,256,301,306,311,331,341,346,376,366,381,371,416,426,431,447,472,481,484,500,501,568,571,572},{134,142},3025,{60,138}},
    };
    const std::array<std::vector<int>,2> baseCompleted={std::vector<int>{0,22,455,486},std::vector<int>{0,22,451,486}};
    swgb::RmsScript script;std::string error;
    const std::string source=R"(#const DIRT1 6
#const BLDG-MAIN1 109
#const UNIT-WORKER 83
#const UNIT-BIKE-E 550
<PLAYER_SETUP> random_placement
<LAND_GENERATION>
base_terrain DIRT1
create_player_lands { terrain_type DIRT1 land_percent 100 base_size 7 left_border 10 right_border 20 top_border 15 bottom_border 5 }
<TERRAIN_GENERATION>
<OBJECTS_GENERATION>
create_object BLDG-MAIN1 { set_place_for_every_player group_placement_radius 18 min_distance_to_players 0 max_distance_to_players 0 }
create_object UNIT-WORKER { set_place_for_every_player min_distance_to_players 6 max_distance_to_players 6 }
create_object UNIT-BIKE-E { number_of_objects 1 set_place_for_every_player min_distance_to_players 7 max_distance_to_players 9 match_player_civ }
)";
    check(script.load(60000,[&](int32_t id,std::vector<uint8_t> &bytes) {
        if(id!=60000)return false;bytes.assign(source.begin(),source.end());return true;
    },{},&error),error.c_str());
    const std::unordered_set<std::string> defines{"TINY_MAP","DEFAULT_RESOURCES","FIXED_POSITIONS"};
    swgb::Game baseline(assets),game(assets);uint32_t seed=1;
    check(baseline.initRms(script,defines,120,seed,&error,2,{6,1}),error.c_str());
    std::array<std::vector<int>,2> prefix;
    for(int player=1;player<=2;++player) {
        prefix[size_t(player)-1]=baseline.technologyOrderForTesting(player);
        auto sorted=prefix[size_t(player)-1];std::sort(sorted.begin(),sorted.end());
        check(sorted==baseCompleted[size_t(player)-1],"Native RMS research baseline matches original pre-age completed states");
    }
    check(seed==3895143042u,"Original common-border RNG before starting-age research");
    // Original age controls entered from composed post-finalization CRT9653.
    const uint32_t expectedRng[]={4799,18572,25671,25671};
    size_t caseIndex=0;
    for(const auto &c:cases) {
        swgb::RmsSessionOptions options;options.startingAge=uint32_t(c.age);uint32_t rng=1;
        check(game.initRmsSession(script,defines,120,rng,{6,1},options,&error),error.c_str());
        check(rng==expectedRng[caseIndex++],"Original starting-age building upgrades reseed the shared CRT stream");
        for(int player=1;player<=2;++player) {
            auto expected=prefix[size_t(player)-1];
            const auto &added=player==1?c.first:c.second;
            expected.insert(expected.end(),added.begin(),added.end());
            check(game.technologyOrderForTesting(player)==expected,
                  "Original starting-age prerequisites, disabled technologies, automatic descendants and completion order");
            check(game.resource(player,21)==c.researchCount[size_t(player)-1] && game.resource(player,99)==0,
                  "Original starting-age research counts and cost-statistic reset");
            check(game.objectUnitId(uint32_t(player))==109 && game.objectHitPoints(uint32_t(player))==c.centerHp,
                  "Original starting-age live Command Center HP and retained source identity");
            for(uint32_t worker=player==1?3u:6u;worker<(player==1?6u:9u);++worker)
                check(game.objectUnitId(worker)==293 && game.objectHitPoints(worker)==55,
                      "Original starting-age HP applies to all three existing workers");
            const auto scout=uint32_t(player+8);
            check(game.objectUnitId(scout)==(player==1?558:550) &&
                  game.objectHitPoints(scout)==c.scoutHp[size_t(player)-1],
                  "Original starting-age live worker and scout HP changes");
        }
        check(game.worldCheckpointForTesting()==std::optional<uint32_t>(10762),
              "Starting-age upgrades do not overwrite the original finalized world seed");
        uint32_t aiRng=*game.worldCheckpointForTesting();
        aiRng=swgb::genieRandomDraw(aiRng); // Human player one restores its saved draw.
        swgb::genieRandomDraw(aiRng); // Player two saves its draw, then enters PER.
        const int first=int(swgb::genieRandomDraw(aiRng))+1,second=int(swgb::genieRandomDraw(aiRng))+1;
        const std::string source="(defrule (random-number == -1) => (set-goal 990 1) (disable-self))"
            "(defrule (true) => (generate-random-number 32768) (disable-self))"
            "(defrule (random-number == "+std::to_string(first)+") => (set-goal 991 1) (generate-random-number 32768) (disable-self))"
            "(defrule (random-number == "+std::to_string(second)+") => (set-goal 992 1) (disable-self))";
        check(game.loadAiSourceForTesting(2,"post-age-rng",source,{},&error),error.c_str());
        game.update(0,{});
        check(game.aiGoalForTesting(2,990)==1 && game.aiGoalForTesting(2,991)==1 && game.aiGoalForTesting(2,992)==1,
              "First ordered AI pass restores the original pre-age world checkpoint");
    }
    std::puts("PASS: original normal starting ages3..6 research order, complete technology sets, statistics and live HP");
}
