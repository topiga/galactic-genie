// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

static void civilizationBonusChecks(swgb::Assets &assets) {
    const int units[]={1212,1212,933,579,1208,933,104,933};
    const int attributes[]={101,100,1,13,12,1,100,2};
    const float expected[]={90,50,101,110,103,105,90,105};
    std::string error;
    swgb::Game game(assets);
    for(int civ=1;civ<=8;++civ) {
        check(game.init(0x5A17,64,&error,civ,9-civ),error.c_str());
        const auto id=game.spawnObjectForTesting(civ,units[civ-1],1,6.5f,6.5f);
        check(id && std::abs(game.unitAttributeForTesting(id,attributes[civ-1],100)-expected[civ-1])<.001f,
              "Each original civilization applies its own unit-stat bonus");
    }
    check(game.initSkirmish(0x5A17,&error,false,5,1,75),error.c_str());
    check(game.populationCapacityForTesting(1)==75,
          "Trade Federation population bonus reaches setup cap without extra housing");
    check(game.init(0x5A17,64,&error,1,7),error.c_str());
    const auto fortress=game.spawnFoundationForTesting(1,1212,1,6.5f,6.5f,{});
    check(fortress && std::abs(game.constructionDurationForTesting(fortress)-
          assets.dat().civs[1].units[1212].trainTime*.9f)<.001f,
          "Empire construction-time bonus reaches the actual foundation timer");

    const auto master=game.spawnObjectForTesting(1,115,1,8.5f,8.5f);
    check(game.playerAttributeForTesting(1,89)==1 && game.unitAttributeForTesting(master,5,1)==1,
          "Enemy Republic grants no healing or Jedi speed bonus");
    game.setDiplomacyForTesting(1,2,0);
    game.initializeCivilizationBonusesForTesting();
    check(game.playerAttributeForTesting(1,89)==2 &&
          std::abs(game.unitAttributeForTesting(master,5,1)-1.1f)<.0001f,
          "Startup team bonuses follow recipient alliance without requiring reciprocity");
    game.setDiplomacyForTesting(1,2,3);
    check(game.playerAttributeForTesting(1,89)==2,
          "Startup effects are not recomputed by querying stats after diplomacy changes");
    check(game.researchTechnologyForTesting(1,578),"Research healing multiplier fixture");
    check(std::abs(game.playerAttributeForTesting(1,89)-2.4f)<.0001f,
          "Research effects apply after civilization and team effects");
    check(game.init(0x5A17,64,&error,7,7),error.c_str());
    game.setDiplomacyForTesting(1,2,0); game.initializeCivilizationBonusesForTesting();
    const auto duplicate=game.spawnObjectForTesting(7,643,1,8.5f,8.5f);
    check(std::abs(game.unitAttributeForTesting(duplicate,5,1)-1.1f)<.0001f,
          "Allies of the same civilization do not multiply the team bonus twice");
    check(game.init(0x5A17,64,&error,1,1),error.c_str());
    check(game.playerAttributeForTesting(1,89)==1,"Restart clears prior match bonuses and research");
    std::puts("PASS: original civilization effects, directional startup alliances, duplicate teams and research order");
}

static void civilizationChecks(swgb::Assets &assets,swgb::SoftRenderer &renderer) {
    civilizationBonusChecks(assets);
    swgb::Game game(assets);std::string error;
    for(int civ=1;civ<=8;++civ) {
        check(game.initSkirmish(0x5A17,&error,false,civ,9-civ),error.c_str());
        game.setClassicHud(true);
        const int troopId=game.objectUnitId(11);
        const int enemyTroop=game.objectUnitId(game.commandCenterId(2)+10);
        check(troopId>=0 && enemyTroop>=0,"Both opening troopers exist");
        const auto &troop=assets.dat().civs[size_t(civ)].units[size_t(troopId)];
        const auto &opponent=assets.dat().civs[size_t(9-civ)].units[size_t(enemyTroop)];
        std::printf("Checking civilization %d versus %d\n",civ,9-civ);
        check(game.aiObjectCountForTesting(1,"UNIT-WORKER")==5 &&
              game.aiObjectCountForTesting(1,troop.name)==4,"Each civilization starts with five workers and four matching troopers");
        check(game.aiObjectCountForTesting(2,"UNIT-WORKER")==5 &&
              game.aiObjectCountForTesting(2,opponent.name)==4,"Each opposing civilization has a complete opening army");
        check(troopId>=0 && assets.dat().civs[size_t(civ)].units[size_t(troopId)].civilization==civ,
              "Starting troopers belong to the selected civilization");
        const int iconSet=assets.dat().civs[size_t(civ)].iconSet;
        check(assets.interfaceSheet(51100+iconSet)!=nullptr,"Original civilization HUD artwork exists, including expansion factions");
        check(game.selectObjectForTesting(game.commandCenterId(1)),"Select civilization command center");
        const auto workers=game.productionOptionIds(game.commandCenterId(1));
        check(!workers.empty(),"Selected civilization offers worker recruitment");
        const int slot=assets.dat().civs[size_t(civ)].units[size_t(workers.front())].buttonId-1;
        check(slot>=0 && slot<12,"Worker occupies a visible original command slot");
        const float before=game.resource(1,0)+game.resource(1,1)+game.resource(1,2)+game.resource(1,3);
        game.render(renderer,960,544);
        swgb::InputState recruit;recruit.pointerX=recruit.boxStartX=(20+(slot%4)*42)*1.2f;
        recruit.pointerY=recruit.boxStartY=340+(20+(slot/4)*42)*1.2f;
        recruit.pointerPressed=recruit.pointerHeld=true;game.update(0,recruit);
        recruit.pointerPressed=recruit.pointerHeld=false;recruit.selectPressed=true;game.update(0,recruit);
        check(game.productionQueueSizeForTesting(game.commandCenterId(1))==1 &&
              game.resource(1,0)+game.resource(1,1)+game.resource(1,2)+game.resource(1,3)<before,"Original HUD recruits a worker using actual civilization costs");
        game.render(renderer,960,544);
        if(civ==7 || civ==8) check(renderer.savePng(civ==7?"republic-hud.png":"confederacy-hud.png"),"Save expansion civilization HUD comparison");
        for(int frame=0;frame<1200;++frame) game.update(1.f/30,{});
        check(game.matchResult()<0 && game.aiObjectCountForTesting(1,"UNIT-WORKER")==6,
              "Each civilization completes its paid worker recruitment");
    }
    const auto center=game.commandCenterId(1);const auto time=game.matchTime();
    check(!game.initSkirmish(0x5A17,&error,false,0,3) && !error.empty(),"Reject Gaia as human civilization");
    check(!game.initSkirmish(0x5A17,&error,false,1,9),"Reject unsupported opposing civilization");
    check(game.commandCenterId(1)==center && game.matchTime()==time,"Invalid setup does not reset an existing match");
    check(game.initSkirmish(0x5A17,&error,false,1,3,25),error.c_str());
    uint32_t extraWorker=0;
    for(int player=1;player<=2;++player) {
        const int civ=player==1?1:3;
        for(int i=0;i<5;++i) check(game.spawnObjectForTesting(civ,70,player,5.f+i*3,5.f+player*10),"Population-cap housing fixture");
        check(game.populationCapacityForTesting(player)==25,"Housing is capped by setup population limit for both players");
        while(game.populationUsedForTesting(player)<25) {
            const auto worker=game.spawnObjectForTesting(civ,83,player,10.f,10.f+player*10);
            check(worker!=0,"Population-cap worker fixture");if(player==1) extraWorker=worker;
        }
    }
    const auto houses=game.aiObjectCountForTesting(2,"BLDG-DWELLING1",true);
    check(game.queueUnitForTesting(game.commandCenterId(1),83),"Queue completion fixture at population limit");
    game.update(.2f,{});
    check(game.productionQueueSizeForTesting(game.commandCenterId(1))==1 && game.populationUsedForTesting(1)==25,
          "Production waits at the selected population cap");
    check(game.aiObjectCountForTesting(2,"BLDG-DWELLING1",true)==houses,"AI does not build useless housing at maximum population capacity");
    check(game.damageObjectForTesting(extraWorker,100000),"Free one population slot");game.update(.2f,{});
    check(game.productionQueueSizeForTesting(game.commandCenterId(1))==0 && game.populationUsedForTesting(1)==25,
          "Queued production resumes when population space becomes available");
    check(!game.initSkirmish(0x5A17,&error,false,1,3,0) && !game.initSkirmish(0x5A17,&error,false,1,3,201),
          "Population settings reject unsupported values");
    std::puts("PASS: eight civilization openings, original HUDs, paid recruitment, population limits and setup validation");
}

static void civilizationMatches(swgb::Assets &assets,bool conquest=false) {
    swgb::Game game(assets);std::string error;
    for(int opponent=1;opponent<=8;++opponent) {
        check(game.initSkirmish(0x5A17,&error,false,9-opponent,opponent,75,conquest),error.c_str());
        for(int frame=0;frame<30*(conquest?1800:1200) && game.matchResult()<0;++frame) {
            game.update(1.f/30,{});
            if(frame%(30*120)==0) {
                std::printf("CIV MATCH: AI=%d human=%d time=%.1f center=%.0f/%.0f population=%.0f/%.0f\n",
                    opponent,9-opponent,game.matchTime(),game.objectHitPoints(game.commandCenterId(1)),
                    game.objectHitPoints(game.commandCenterId(2)),game.populationUsedForTesting(1),game.populationUsedForTesting(2));
                std::fflush(stdout);
            }
        }
        check(game.matchResult()==0 && game.combatStats().attacksLanded>0,"Each civilization AI defeats an idle opposing faction through combat");
        std::printf("PASS: civilization %d AI victory at %.1fs\n",opponent,game.matchTime());std::fflush(stdout);
    }
}
