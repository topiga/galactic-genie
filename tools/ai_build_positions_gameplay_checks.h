// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
[[gnu::noinline]] static void aiBuildPositionsGameplayChecks(swgb::Assets &assets) {
    swgb::Scenario scenario;scenario.map.width=scenario.map.height=64;scenario.map.tiles.resize(4096);
    scenario.nextUnitId=4;scenario.playerSlotCount=2;scenario.civilizations.fill(1);
    for(int i=0;i<2;++i) {scenario.players[i].active=scenario.players[i].human=true;
        scenario.players[i].civilization=1;scenario.players[i].diplomacy.fill(3);scenario.players[i].diplomacy[size_t(i+1)]=0;}
    for(const auto spec:std::array<std::array<int,4>,3>{{{1,109,32,32},{2,83,36,32},{3,307,36,34}}}) {
        swgb::ScenarioUnit u;u.spawnId=spec[0];u.unitId=uint16_t(spec[1]);u.x=spec[2];u.y=spec[3];u.player=2;u.state=2;scenario.units.push_back(u);
    }
    swgb::Game game(assets);std::string error;
    const auto count=[&](int player) {const auto &slots=game.aiBuildPositionsForTesting(player).slots();
        return std::count_if(slots.begin(),slots.end(),[](const auto &p){return p.prototype!=-1;});};
    const auto status=[&](int player,int prototype,float x,float y) {
        for(const auto &p:game.aiBuildPositionsForTesting(player).slots())
            if(p.prototype==prototype && p.x==uint8_t(int(64-y)) && p.y==uint8_t(int(x)))return int(p.status);
        return -1;
    };
    const auto reset=[&] {
        check(game.initScenario(scenario,&error),error.c_str());game.setLocalPlayerForTesting(2);
        check(game.loadAiSourceForTesting(2,"placement-registry","",{},&error),error.c_str());
        for(int type=0;type<4;++type)game.setResourceForTesting(2,type,1000);
    };
    reset();const auto home=game.objectPosition(1);
    check(count(2)==1 && status(2,109,home[0],home[1])==1,
          "Scenario admission registers the owned building class, not worker or troop classes");
    const auto foundation=game.spawnFoundationForTesting(1,70,2,42,42,{2});
    check(foundation && status(2,70,42,42)==1,"Foundation factory registers status1 before completion");
    check(game.completeFoundationForTesting(foundation),"Finish foundation work");
    game.update(0,{});
    const auto remainingFoundations=game.underConstructionObjectIds();
    check(std::find(remainingFoundations.begin(),remainingFoundations.end(),foundation)==remainingFoundations.end() &&
          game.objectActive(foundation) && status(2,70,42,42)==1,
          "Actual completion callback retains the original planned-position record");
    check(game.damageObjectForTesting(foundation,100000) && status(2,70,42,42)==1,
          "Ordinary death does not delete planned-position history");
    check(game.transferObjectForTesting(1,1) && status(1,109,home[0],home[1])==1 && status(2,109,home[0],home[1])==1,
          "New ownership admits a position while old-owner history persists");
    game.setAiBuildPositionForTesting(2,109,int(64-home[1]),int(home[0]),2);
    const auto before=count(2);
    check(game.loadAiSourceForTesting(2,"placement-registry-reload","",{},&error),error.c_str());
    check(count(2)==before && status(2,109,home[0],home[1])==2,
          "PER reload preserves status and order without live-object re-registration");
    reset();check(count(2)==1 && status(2,70,42,42)==-1,"Fresh world clears obsolete planned positions");
    const auto &prefab=assets.dat().civs[1].units[70];
    check(game.admitAiBuildForTesting(2,prefab),"Admit a real request for feedback checks");
    game.failAiBuildRequestForTesting(2,0,1,52,52);
    check(status(2,70,52,52)==-1,"Execution feedback never inserts a missing position");
    game.setAiBuildPositionForTesting(2,70,12,52,1);
    game.failAiBuildRequestForTesting(2,0,6,52,52);
    check(status(2,70,52,52)==2,"Cancellation feedback updates an existing key to status2");
    game.failAiBuildRequestForTesting(2,0,-1,52,52);
    check(status(2,70,52,52)==0,"Original minus-one feedback restores a preset status0");
    float x=-1,y=-1;game.seedLegacyRandomForTesting(12345);
    check(game.chooseAiPrefabPositionForTesting(2,prefab,x,y,&error)==1 && x==52 && y==52 &&
          game.legacyRandomForTesting()==12345 && status(2,70,52,52)==1,
          "First status0 preset is consumed before fresh-search RNG and uses normal final-center snapping");
    reset();check(game.admitAiBuildForTesting(2,prefab),"Admit unreachable construction request");
    game.setAiBuildPositionForTesting(2,70,12,52,0);
    // Enclose the selected worker without occupying the requested foundation.
    const auto trapped=game.objectPosition(2);
    for(int dy=-2;dy<=2;++dy)for(int dx=-2;dx<=2;++dx)
        if(std::abs(dx)==2 || std::abs(dy)==2)
            check(game.setTerrainForTesting(int(trapped[0])+dx,int(trapped[1])+dy,1),"Enclose builder with impassable water");
    game.updateAiBuildRequestsForTesting(2);
    check(status(2,70,52,52)==2 && game.underConstructionObjectIds().empty(),
          "Selected builder failure inserts tactical rejection without dispatching a partial route");
    reset();check(game.admitAiBuildForTesting(2,prefab),"Admit normal construction for cancellation lifecycle");
    game.updateAiBuildRequestsForTesting(2);game.update(0,{});
    uint32_t built=0;for(auto id:game.underConstructionObjectIds())if(game.objectUnitId(id)==70)built=id;
    check(built!=0,"World command admits a registered foundation");const auto at=game.objectPosition(built);
    check(status(2,70,at[0],at[1])==1 && game.selectObjectForTesting(built) && game.destroyLastSelectedForTesting(),
          "Cancel the actual queued foundation through the ordinary UI command");
    check(status(2,70,at[0],at[1])==2,"Actual cancellation updates the registered position and retains history");
    check(game.initAridSkirmish(1,&error),error.c_str());
    for(int player:{1,2}) {const auto p=game.objectPosition(game.commandCenterId(player));
        bool found=false;for(const auto &record:game.aiBuildPositionsForTesting(player).slots())
            if(record.prototype==109 && record.x==uint8_t(int(120-p[1])) && record.y==uint8_t(int(p[0])) && record.status==1)found=true;
        check(found,"RMS startup, AI installation and playable activation preserve owned admissions");}
    std::puts("PASS: planned-position admission, completion, death, ownership, feedback, presets and session lifetime");
}

[[gnu::noinline]] static void aiResourceGridGameplayChecks(swgb::Assets &assets) {
    swgb::Scenario scenario;scenario.map.width=scenario.map.height=64;scenario.map.tiles.resize(4096);
    scenario.nextUnitId=1;scenario.playerSlotCount=2;scenario.civilizations.fill(1);
    for(int i=0;i<2;++i) {scenario.players[i].active=scenario.players[i].human=true;scenario.players[i].civilization=1;
        scenario.players[i].diplomacy.fill(3);scenario.players[i].diplomacy[size_t(i+1)]=0;}
    swgb::Game game(assets);std::string error;
    for(bool upgraded:{false,true})for(const auto pair:std::array<std::array<int,2>,2>{{{562,348},{584,102}}}) {
        check(game.initScenario(scenario,&error),error.c_str());game.setLocalPlayerForTesting(2);
        check(game.spawnObjectForTesting(1,109,2,32,32) && game.spawnObjectForTesting(1,83,2,35,32),"Original grid home and builder");
        const auto observer=game.spawnObjectForTesting(1,83,2,42,32);
        const auto known=game.spawnObjectForTesting(0,pair[1],0,44.5f,32.5f);
        const auto unknown=game.spawnObjectForTesting(0,pair[1],0,15.5f,10.5f);
        check(observer && known && unknown,"Grid resource knowledge fixture");
        if(upgraded)check(game.researchTechnologyForTesting(2,1),"Real age-up keeps original depot category");
        for(int type=0;type<4;++type)game.setResourceForTesting(2,type,1000);
        const auto source="(defrule (true) => (set-strategic-number 117 100) (disable-self))\n"
            "(defrule (game-time > 1) (can-build "+std::to_string(pair[0])+") => (build "+std::to_string(pair[0])+") (disable-self))";
        check(game.loadAiSourceForTesting(2,"original-resource-grid",source,{},&error),error.c_str());
        game.update(0,{});game.updateAiGatherersForTesting(2);
        check(game.economyForTesting(2).resourceSlots.count(known) && !game.economyForTesting(2).resourceSlots.count(unknown),
              "Original resource grid only receives admitted known IDs");
        check(game.moveObjectForTesting(observer,4,4),"Hide stationary known resource");game.updatePaused(0,{});
        float x=-1,y=-1;const auto *unit=game.unitPrototypeForTesting(2,pair[0]);
        check(game.chooseAiPrefabPositionForTesting(2,*unit,x,y,&error)==1,error.c_str());
        check(std::hypot(x-44.5f,y-32.5f)<8,"Original scoring attracts the depot to a hidden but previously observed compatible resource");
        uint32_t foundation=0;
        for(int tick=0;tick<100 && !foundation;++tick) {game.update(.05f,{});
            for(auto id:game.underConstructionObjectIds())if(game.objectUnitId(id)==pair[0])foundation=id;}
        check(foundation!=0,"Normal PER construction uses the original supported category grid");
        const auto placed=game.objectPosition(foundation);
        check(std::hypot(placed[0]-44.5f,placed[1]-32.5f)<8 && !game.economyForTesting(2).resourceSlots.count(unknown),
              "Actual foundation follows original known-resource scoring without discovering hidden decoy");
    }
    std::puts("PASS: original carbon/ore grid in normal gameplay, upgrades and hidden stationary resources");
}
