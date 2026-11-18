// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "../src/core/genie_random.h"
#include "../src/engine/ai_prefab_grid.h"

static void aiPrefabPlacementGameplayChecks(swgb::Assets &assets) {
    swgb::Game game(assets);std::string error;
    const auto &prefab=assets.dat().civs[1].units[70];
    const auto &center=assets.dat().civs[1].units[109];
    auto reset=[&] {check(game.initSkirmish(0x5A17,&error,false,1,1),error.c_str());};
    auto outsideProtectedCenter=[&](float nx,float ny) {
        const auto cc=game.objectPosition(game.commandCenterId(1));
        const auto bounds=swgb::AiBuildPlacementGrid::buildingBounds(64.f-cc[1],cc[0],center.collisionSize[0]);
        const auto site=swgb::AiBuildPlacementGrid::buildingBounds(64.f-ny,nx,prefab.collisionSize[0]);
        return site.right<bounds.left-3 || site.left>bounds.right+3 ||
               site.bottom<bounds.top-3 || site.top>bounds.bottom+3;
    };
    reset();float x=-99,y=-99;game.seedLegacyRandomForTesting(12345);
    check(game.chooseAiPrefabPositionForTesting(1,center,x,y,&error)==-1,
          "Non-prefab request is explicitly outside the bounded original provider");
    check(game.legacyRandomForTesting()==12345 && x==-99 && y==-99,
          "Unsupported request does not consume placement RNG or change output coordinates");
    check(game.chooseAiPrefabPositionForTesting(1,prefab,x,y,&error)==1,error.c_str());
    check(outsideProtectedCenter(x,y),"Original prefab footprint excludes the full Command Center access rectangle");
    check(game.placementValidForTesting(1,70,x,y),"Chosen prefab also passes native master-placement adapter");

    // Radius zero leaves only the forbidden Command Center anchor. A supported
    // failed exhaustive search still consumes the original SN168 initial draw.
    check(game.loadAiSourceForTesting(1,"prefab-empty-search",
          "(defrule (true) => (set-strategic-number 74 0) (disable-self))",{},&error),error.c_str());
    game.update(0,{});game.seedLegacyRandomForTesting(12345);
    uint32_t expected=12345;swgb::genieRandomDraw(expected);
    x=-99;y=-99;
    check(game.chooseAiPrefabPositionForTesting(1,prefab,x,y,&error)==0,
          "Supported search with only the forbidden anchor returns no location");
    check(game.legacyRandomForTesting()==expected && x==-99 && y==-99,
          "No-location search preserves output and consumes its initial draw exactly once");

    reset();
    for(int type=0;type<4;++type)game.setResourceForTesting(1,type,1000);
    check(game.loadAiSourceForTesting(1,"prefab-production-build",
          "(defrule (true) => (build 70) (disable-self))",{},&error),error.c_str());
    check(game.loadAiSourceForTesting(2,"prefab-passive-opponent",
          "(defrule (true) => (disable-self))",{},&error),error.c_str());
    game.update(0,{});
    game.update(0,{}); // Deferred world command drains after the PER admission.
    bool found=false;
    for(uint32_t id:game.underConstructionObjectIds())if(game.objectUnitId(id)==70) {
        const auto position=game.objectPosition(id);found=true;
        check(outsideProtectedCenter(position[0],position[1]),
              "Actual PER build dispatch retains original Command Center access clearance");
    }
    check(found,"Actual PER build creates a prefab foundation through the shared placement provider");
    reset();
    const auto &farmMaster=assets.dat().civs[1].units[50];
    check(game.chooseAiPrefabPositionForTesting(1,farmMaster,x,y,&error)==1,error.c_str());
    check(game.placementValidForTesting(1,50,x,y),"Original farm scoring chooses a valid native foundation");
    for(int type=0;type<4;++type)game.setResourceForTesting(1,type,1000);
    check(game.loadAiSourceForTesting(1,"farm-production-build",
          "(defrule (true) => (build BLDG-FARM) (disable-self))",{},&error),error.c_str());
    check(game.loadAiSourceForTesting(2,"farm-passive-opponent","",{},&error),error.c_str());
    game.update(0,{});game.update(0,{});
    uint32_t farm=0;
    for(uint32_t id:game.underConstructionObjectIds())if(game.objectUnitId(id)==50)farm=id;
    check(farm!=0,"Actual PER farm construction uses the shared original scoring provider");
    bool completed=false;
    for(int tick=0;tick<1800 && !completed;++tick) {
        game.update(.1f,{});
        const auto foundations=game.underConstructionObjectIds();
        completed=std::find(foundations.begin(),foundations.end(),farm)==foundations.end();
    }
    check(completed && game.objectResourceAmount(farm)>0,"A builder reaches and completes the scored farm site");
    const auto cc=game.objectPosition(game.commandCenterId(1));
    const auto farmer=game.spawnObjectForTesting(1,83,1,cc[0]+3,cc[1]);
    check(farmer && game.issueGatherForTesting(farmer,farm),"Worker receives gather task for the newly placed farm");
    const float remaining=game.objectResourceAmount(farm);
    for(int tick=0;tick<900 && game.objectResourceAmount(farm)>=remaining;++tick)game.update(.1f,{});
    check(game.objectResourceAmount(farm)<remaining,"The completed original-scored farm supports a real gather path");
    std::puts("PASS: original prefab/farm scoring, placement RNG, PER construction and farm gathering");
}
