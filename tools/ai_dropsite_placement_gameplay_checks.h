// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
[[gnu::noinline]] static void aiDropsitePlacementGameplayChecks(swgb::Assets &assets) {
    swgb::Scenario scenario;scenario.map.width=scenario.map.height=64;scenario.map.tiles.resize(4096);
    scenario.nextUnitId=1;scenario.playerSlotCount=2;scenario.civilizations.fill(1);
    for(int i=0;i<2;++i) {scenario.players[i].active=true;scenario.players[i].human=true;
        scenario.players[i].civilization=1;scenario.players[i].diplomacy.fill(3);
        scenario.players[i].diplomacy[size_t(i+1)]=0;}
    swgb::Game game(assets);std::string error;
    for(bool upgraded:{false,true})for(const auto pair:std::array<std::array<int,2>,4>{{{68,59},{562,348},{323,66},{584,102}}}) {
        check(game.initScenario(scenario,&error),error.c_str());game.setLocalPlayerForTesting(2);
        check(game.spawnObjectForTesting(1,109,2,8,8) && game.spawnObjectForTesting(1,83,2,11,8),
              "Dropsite placement has an admitted owner, home and builder");
        const auto observer=game.spawnObjectForTesting(1,83,2,44,44);
        const auto remembered=game.spawnObjectForTesting(0,pair[1],0,46,44);
        const auto unseen=game.spawnObjectForTesting(0,pair[1],0,24,8);
        check(observer && remembered && unseen,"Known distant resource and closer unobserved decoy");
        // Keep the hidden-mutation contract on the explicitly native fallback:
        // original resource grids read known actors' live floats.
        check(game.spawnObjectForTesting(1,72,2,8,20),"A real wall-plan context keeps this fixture on the bounded fallback");
        if(upgraded)check(game.researchTechnologyForTesting(2,1),"Apply original age-up to real dropsite masters");
        const auto *master=game.unitPrototypeForTesting(2,pair[0]);
        check(master && (!upgraded || master->copyId!=master->id),"Fixture exercises actual upgraded dropsite identities");
        {
            float px=0,py=0;
            check(game.chooseAiPrefabPositionForTesting(2,*master,px,py,&error)==-1,
                  "Unrepresented wall stage explicitly selects native fallback, not original grid");
        }
        for(int type=0;type<4;++type)game.setResourceForTesting(2,type,1000);
        const std::string source="(defrule (true) => (set-strategic-number 117 100) (disable-self))\n"
            "(defrule (game-time > 1) (can-build "+std::to_string(pair[0])+
            ") => (build "+std::to_string(pair[0])+") (disable-self))";
        check(game.loadAiSourceForTesting(2,"remembered-dropsite",source,{},&error),error.c_str());
        game.update(0,{});game.updateAiGatherersForTesting(2);
        const auto &knowledge=game.economyForTesting(2);
        check(knowledge.resourceSlots.count(remembered)==1 && !knowledge.resourceSlots.count(unseen),
              "Only the observed resource enters placement knowledge");
        check(game.moveObjectForTesting(observer,4,4),"Remove resource observer");game.updatePaused(0,{});
        check(!game.objectVisibleForTesting(remembered) && !game.objectVisibleForTesting(unseen),
              "Both resource locations are hidden before requesting construction");
        check(game.moveObjectForTesting(remembered,55,55),"Mutate hidden live resource independently of remembered coordinates");
        game.setObjectResourceForTesting(remembered,0);
        uint32_t foundation=0;
        for(int step=0;step<100 && !foundation;++step) {
            game.update(.05f,{});
            for(auto id:game.underConstructionObjectIds())if(game.objectUnitId(id)==pair[0])foundation=id;
        }
        check(foundation!=0,"Production PER dispatches the requested dropsite through the normal build queue");
        const auto position=game.objectPosition(foundation);
        check(std::hypot(position[0]-46.f,position[1]-44.f)<=5.f,
              "Dropsite uses remembered compatible resource coordinates across upgrades, without hidden-state leakage");
        check(!game.economyForTesting(2).resourceSlots.count(unseen),"Placement never admits the hidden decoy");
    }
    std::puts("PASS: original dropsite identities, upgraded names and remembered-resource native placement");
}
