// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cmath>
#include <cstdio>
#include <limits>
#include <memory>
#include <vector>

static void renderVisibilityPointChecks(swgb::Assets &assets);

static void visibilityRefreshChecks(swgb::Assets &assets) {
    auto scenario=[] {
        swgb::Scenario s;s.map.width=s.map.height=32;s.map.tiles.resize(32*32);
        s.nextUnitId=1;s.playerSlotCount=3;s.civilizations.fill(1);
        for(int i=0;i<3;++i){s.players[i].active=true;s.players[i].human=true;s.players[i].civilization=1;
            s.players[i].diplomacy.fill(3);s.players[i].diplomacy[size_t(i+1)]=0;}
        return s;
    };
    // Memory insertion and selection consume the optimized refresh path;
    // objectVisibleForTesting remains the independent live source scan.
    auto freshMemory=[](swgb::Game &g,uint32_t target) {
        const bool expected=g.objectVisibleForTesting(target);
        g.setInitiallyExploredForTesting(false);
        check(g.rememberedForTesting(target)==expected,
              "Fresh stationary enemy memory agrees with live sight-source scan");
        check(g.renderObjectVisibleForTesting(target)==expected,
              "Fresh render object visibility agrees with live scan");
        if(!expected)check(!g.objectSelected(target),"Refresh clears selection outside live sight");
        // Visible annexes and edge fixtures need not be inspectable. Memory
        // remains compared for every fixture; selection applies when admitted.
        if(expected && g.selectObjectForTesting(target)) {
            g.tileVisibilityForTesting(0,0);
            check(g.objectSelected(target),"Refresh retains a selected visible building");
        }
    };
    std::string error;swgb::Game game(assets);
    check(game.initScenario(scenario(),&error),error.c_str());
    const auto observer=game.spawnObjectForTesting(1,83,1,16.f,16.f);
    const auto target=game.spawnObjectForTesting(1,70,3,17.f,16.f);
    check(observer && target,"Visibility refresh fixtures");
    check(game.selectObjectForTesting(target),"Normal visible building is inspectable");
    freshMemory(game,target);
    check(game.moveObjectForTesting(observer,1.f,1.f),"Move sight source away");
    check(!game.objectVisibleForTesting(target),"Live scan loses distant building");
    check(game.rememberedForTesting(target) && !game.objectSelected(target),
          "Lost sight retains building memory but clears live selection");
    freshMemory(game,target);
    const auto ally=game.spawnObjectForTesting(1,83,2,17.f,16.f);
    check(ally!=0,"Allied observer fixture");
    // Sight sharing is a civilization/research attribute, not the mutable
    // resource inventory. Original technology61 installs REVEAL-ALLY.
    check(game.researchTechnology(1,61),"Research original allied-sight effect");
    game.setDiplomacyForTesting(1,2,0);game.setDiplomacyForTesting(2,1,3);
    check(!game.objectVisibleForTesting(target),"One-way alliance grants no shared sight");
    freshMemory(game,target);
    game.setDiplomacyForTesting(2,1,0);
    check(game.objectVisibleForTesting(target),"Mutual alliance grants shared sight");
    freshMemory(game,target);
    check(game.damageObjectForTesting(ally,100000),"Remove allied sight source");
    check(!game.objectVisibleForTesting(target),"Dead observer contributes no sight");
    freshMemory(game,target);

    // Annex target-manager geometry intentionally differs from center-based
    // objectVisible/memory visibility; the optimization must not conflate them.
    const auto parent=game.spawnObjectForTesting(1,70,3,1.f,1.f);
    check(parent && game.linkAnnexForTesting(target,parent),"Annex fixture");
    freshMemory(game,target);
    check(game.setBuildingFrozenForTesting(target,true),"Freeze annex fixture");
    freshMemory(game,target);
    check(game.setBuildingFrozenForTesting(target,false),"Thaw annex fixture");

    // Exercise integer and adjacent-float coordinate conversion without
    // substituting raster/ceil rounding for the original live query.
    check(game.moveObjectForTesting(observer,16.f,16.f),"Restore observer");
    for(float x:{0.f,1.f,15.5f,16.f,std::nextafter(16.f,0.f),std::nextafter(16.f,32.f),31.5f,32.f})
        for(float y:{0.f,1.f,15.5f,16.f,std::nextafter(16.f,0.f),std::nextafter(16.f,32.f),31.5f,32.f}) {
            check(game.moveObjectForTesting(target,x,y),"Place coordinate comparison target");
            freshMemory(game,target);
        }

    // Construction sight uses footprint rules, including the unit112 exception.
    for(int unit:{70,112}) {
        swgb::Game construction(assets);
        check(construction.initScenario(scenario(),&error),error.c_str());
        const auto site=construction.spawnFoundationForTesting(1,unit,1,16.f,16.f,{});
        const auto building=construction.spawnObjectForTesting(1,70,3,16.f,16.f);
        check(site && building,"Construction visibility fixtures");
        for(float x:{12.f,15.f,16.f,17.f,20.f}) {
            construction.moveObjectForTesting(building,x,16.f);
            freshMemory(construction,building);
        }
        check(construction.completeFoundationForTesting(site),"Complete sight-source foundation");
        construction.update(0,{});
        freshMemory(construction,building);
    }

    // Imported garrison excludes the worker from sight even when its stored
    // coordinates lie far from its container. No movement/timing dependency.
    auto imported=scenario();
    swgb::ScenarioUnit center;center.spawnId=1;center.unitId=109;center.player=1;center.x=center.y=2;
    swgb::ScenarioUnit passenger;passenger.spawnId=2;passenger.unitId=83;passenger.player=1;
    passenger.x=passenger.y=25;passenger.garrisonedInId=1;
    imported.units={center,passenger};imported.nextUnitId=3;
    swgb::Game garrison(assets);check(garrison.initScenario(imported,&error),error.c_str());
    check(garrison.garrisonContainerForTesting(2)==1,"Imported passenger remains garrisoned");
    const auto distant=garrison.spawnObjectForTesting(1,70,3,25.f,25.f);
    check(distant && !garrison.objectVisibleForTesting(distant),"Garrisoned passenger grants no distant sight");
    freshMemory(garrison,distant);
    renderVisibilityPointChecks(assets);
    std::puts("PASS: refresh memory/selection agrees with live sight across alliance, death, construction, annex, garrison and coordinates");
}

static void renderVisibilityPointChecks(swgb::Assets &assets) {
    const auto scenario=[] {
        swgb::Scenario s;s.map.width=s.map.height=32;s.map.tiles.resize(32*32);
        s.nextUnitId=1;s.playerSlotCount=3;s.civilizations.fill(1);
        for(int i=0;i<3;++i) {
            s.players[i].active=true;s.players[i].human=true;s.players[i].civilization=1;
            s.players[i].diplomacy.fill(3);s.players[i].diplomacy[size_t(i+1)]=0;
        }
        return s;
    };
    const float infinity=std::numeric_limits<float>::infinity();
    const float tiny=std::nextafter(0.f,infinity);
    std::vector<float> coordinates{-infinity,infinity,std::numeric_limits<float>::quiet_NaN(),
                                   -1.f,-tiny,tiny};
    for(int i=0;i<=32;++i) {
        const float value=float(i);
        coordinates.push_back(value);
        coordinates.push_back(std::nextafter(value,-infinity));
        coordinates.push_back(std::nextafter(value,infinity));
        if(i<32)coordinates.push_back(value+.5f);
    }
    size_t comparisons=0;
    const auto compare=[&](swgb::Game &game,const char *stage) {
        // Same refresh boundary as render, without advancing simulation or
        // making the reference query accidentally use the raster cache.
        game.updatePaused(0,{});
        for(float x:coordinates)for(float y:coordinates) {
            const bool expected=game.worldSoundVisibleForTesting(x,y);
            const bool actual=game.renderPointVisibleForTesting(x,y);
            if(expected!=actual)std::fprintf(stderr,
                "Render visibility mismatch [%s] (%a,%a): live=%d cache=%d\n",
                stage,double(x),double(y),expected,actual);
            check(expected==actual,"Fresh render sight agrees with independent live source scan");
            ++comparisons;
        }
    };
    std::string error;
    auto game=std::make_unique<swgb::Game>(assets);
    check(game->initScenario(scenario(),&error),error.c_str());
    compare(*game,"no sources");
    game->setInitiallyExploredForTesting(true);
    compare(*game,"explored terrain is not live sight");
    check(!game->renderPointVisibleForTesting(16,16),"Explored terrain does not reveal live objects");
    game->setInitiallyExploredForTesting(false);
    const auto observer=game->spawnObjectForTesting(1,83,1,16,16);
    check(observer!=0,"Render visibility observer fixture");
    compare(*game,"ordinary source");
    check(game->renderPointVisibleForTesting(16,16),"Observer reveals its own location");
    check(game->moveObjectForTesting(observer,8,.125f),"Move observer next to lower map edge");
    compare(*game,"rounded original coordinate outside raster");
    check(game->worldSoundVisibleForTesting(8,tiny) && game->renderPointVisibleForTesting(8,tiny),
          "Positive subnormal Y retains live sight despite rounded original tile outside raster");
    check(game->moveObjectForTesting(observer,2,2),"Move observer away from ally fixture");
    compare(*game,"moved source");
    const auto ally=game->spawnObjectForTesting(1,83,2,25,25);
    check(ally!=0 && game->researchTechnology(1,61),"Research original allied sight");
    game->setDiplomacyForTesting(1,2,0);game->setDiplomacyForTesting(2,1,3);
    compare(*game,"one-way alliance");
    check(!game->renderPointVisibleForTesting(25,25),"One-way alliance grants no cached sight");
    game->setDiplomacyForTesting(2,1,0);
    compare(*game,"mutual allied sight");
    check(game->renderPointVisibleForTesting(25,25),"Mutual alliance shares cached sight");
    check(game->damageObjectForTesting(ally,100000),"Remove allied observer");
    compare(*game,"dead allied source");
    check(!game->renderPointVisibleForTesting(25,25),"Dead ally leaves no live sight in new frame");
    for(int prototype:{70,112}) {
        auto construction=std::make_unique<swgb::Game>(assets);
        check(construction->initScenario(scenario(),&error),error.c_str());
        const auto foundation=construction->spawnFoundationForTesting(1,prototype,1,16,16,{});
        check(foundation!=0,"Render construction sight fixture");
        compare(*construction,prototype==112?"unit112 foundation exception":"foundation footprint sight");
        check(construction->completeFoundationForTesting(foundation),"Complete sight-source foundation");
        compare(*construction,"completed building sight");
    }
    std::printf("PASS: %zu render sight comparisons across coordinates and source lifecycle\n",comparisons);
}
