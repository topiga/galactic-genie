// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "start_placement_policy_checks.h"
#include "../src/engine/rms_script.h"

static void emptyOwnerGameplayChecks(swgb::Assets &assets) {
    checkStartPlacementPolicy();
    // Actual5d1110 factory vectors; the fresh native map has no unrelated obstructions.
    struct Case {uint32_t seed,after;std::array<std::array<float,2>,4> positions;};
    const Case cases[]={
        {1,1572934298u,{{{65,15},{62.26721954345703f,13.990264892578125f},{62.100040435791016f,17.685537338256836f},{67.63936614990234f,16.183629989624023f}}}},
        {12345,598984645u,{{{67,35},{64.8073959350586f,32.37308883666992f},{65.90069580078125f,37.62483596801758f},{64.48225402832031f,35.47691345214844f}}}},
        {0xffffffffu,2864392669u,{{{96,15},{94.27277374267578f,17.71724510192871f},{97.25702667236328f,17.26618194580078f},{93.51802062988281f,14.716238975524902f}}}}
    };
    for(int civ:{1,6})for(const auto &c:cases) {
        swgb::Scenario scenario;scenario.map.width=scenario.map.height=120;scenario.map.tiles.resize(120*120);
        scenario.playerSlotCount=2;scenario.civilizations.fill(uint8_t(civ));scenario.nextUnitId=1;
        for(int i=0;i<2;++i){scenario.players[i].active=true;scenario.players[i].human=i==0;scenario.players[i].civilization=civ;}
        swgb::Game game(assets);std::string error;check(game.initScenario(scenario,&error),error.c_str());
        const float cost=game.resource(1,98),workerCost=game.resource(1,164),buildingCost=game.resource(1,165),workers=game.resource(1,37);
        game.seedLegacyRandomForTesting(c.seed);
        check(game.startEmptyRmsOwnerForTesting(1,&error),error.c_str());
        check(game.activeObjectCount()==4 && game.legacyRandomForTesting()==c.after,"Native fallback preserves original creation count and final CRT");
        for(uint32_t i=0;i<4;++i)check(game.objectPosition(i+1)==std::array<float,2>{c.positions[i][1],120-c.positions[i][0]},
            "Native fallback preserves exact original accepted positions through coordinate conversion");
        check(game.resource(1,98)==cost+525 && game.resource(1,164)==workerCost+150 &&
              game.resource(1,165)==buildingCost+375 && game.resource(1,37)==workers+3,
              "Fallback records original starting-object costs and worker count");
        check(game.pendingFoundationForTesting(1)==255 && game.terrainAtForTesting(int(c.positions[0][1]),119-int(c.positions[0][0]))==(civ==6?6:27),
              "Local starting foundation is painted before rendering or worker updates");
        const auto state=game.legacyRandomForTesting();
        check(game.startEmptyRmsOwnerForTesting(1,&error) && game.legacyRandomForTesting()==state && game.activeObjectCount()==4,
              "Nonempty owners do not recreate their start or draw RNG");
        const auto worker=c.positions[1];
        check(!game.rmsStartPlacementValidForTesting(2,83,worker[0],worker[1]),"Empty-owner workers block existing workers");
    }
    {
        swgb::Scenario scenario;scenario.map.width=scenario.map.height=120;scenario.map.tiles.resize(120*120);
        scenario.playerSlotCount=1;scenario.civilizations.fill(1);scenario.nextUnitId=1;
        scenario.players[0].active=scenario.players[0].human=true;scenario.players[0].civilization=1;
        swgb::Game game(assets);std::string error;check(game.initScenario(scenario,&error),error.c_str());
        auto &attribute=const_cast<swgb::dat::DatFile&>(assets.dat()).civs[1].resources[84];
        struct Restore {float &value;float saved;~Restore(){value=saved;}} restore{attribute,attribute};
        attribute=-3;game.seedLegacyRandomForTesting(1);
        check(game.startEmptyRmsOwnerForTesting(1,&error) && game.activeObjectCount()==1 && game.legacyRandomForTesting()==11478,
              "Negative original starting-worker count creates the CC without workers");
        attribute=restore.saved;
        check(game.initScenario(scenario,&error),error.c_str());
        const auto scenery=game.spawnObjectForTesting(0,573,0,15,55);
        check(scenery!=0,"Clearable scenery fixture");game.seedLegacyRandomForTesting(1);
        check(game.startEmptyRmsOwnerForTesting(1,&error),error.c_str());
        check(game.objectHitPoints(scenery)==0,"Starting CC clears overlapping buildable scenery");
        for(auto &tile:scenario.map.tiles)tile.terrain=1;
        check(game.initScenario(scenario,&error),error.c_str());game.seedLegacyRandomForTesting(1);
        check(!game.startEmptyRmsOwnerForTesting(1,&error) && game.activeObjectCount()==0 && game.legacyRandomForTesting()==3379307729u,
              "Unplaceable CC returns original exhaustion without creating a partial start");
    }
    swgb::RmsScript script;std::string error;
    const std::string source="<LAND_GENERATION> base_terrain 0 create_player_lands { terrain_type 0 base_size 3 number_of_tiles 1000 }";
    check(script.load(0,[&](int32_t id,std::vector<uint8_t> &bytes){if(id)return false;bytes.assign(source.begin(),source.end());return true;},{},&error),error.c_str());
    swgb::Game game(assets);swgb::RmsSessionOptions options;uint32_t random=1;
    check(game.initRmsSession(script,{},120,random,{6,1},options,&error),error.c_str());
    check(game.activeObjectCount()==8 && game.worldCheckpointForTesting().has_value(),"Real RMS session creates both empty-owner starts and finalizes its checkpoint");
    game.update(.05f,{});
    check(game.activeObjectCount()==8,"Fallback objects survive the first admitted gameplay update");
    std::puts("PASS: native empty-owner creation, original positions/RNG, accounting and RMS session integration");
}
