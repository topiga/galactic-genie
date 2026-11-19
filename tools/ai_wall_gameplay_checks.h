// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
[[gnu::noinline]] static void aiWallGameplayChecks(swgb::Assets &assets) {
    swgb::Scenario scenario;
    scenario.map.width=scenario.map.height=64;scenario.map.tiles.resize(4096);
    scenario.nextUnitId=1;scenario.playerSlotCount=2;scenario.civilizations.fill(1);
    for(int i=0;i<2;++i) {
        auto &p=scenario.players[i];p.active=p.human=true;p.civilization=1;
        p.diplomacy.fill(3);p.diplomacy[size_t(i+1)]=0;
    }
    swgb::Game game(assets);std::string error;
    check(game.initScenario(scenario,&error),error.c_str());
    check(game.researchTechnologyForTesting(2,1),"Empire wall and gate research");
    game.setAiWallSegmentForTesting(2,2,{2,24,30,29,30,6,true});
    game.setResourceForTesting(2,2,60);
    check(game.loadAiSourceForTesting(2,"wall-and-gate",R"(
(defrule (true) => (enable-wall-placement 2) (set-strategic-number 4 3) (disable-self))
(defrule (can-build-wall 2 BLDG-BARRIER-LINE) => (build-wall 2 BLDG-BARRIER-LINE) (disable-self))
(defrule (can-build-gate-with-escrow 2) => (build-gate 2) (disable-self))
)",{},&error),error.c_str());
    const auto advance=[&](int steps){for(int i=0;i<steps;++i)game.update(.1f,{});};
    advance(60);
    check(game.underConstructionObjectIds().empty() && game.resource(2,2)==60,
          "Unseen wall route stays unpurchased");
    game.setResourceForTesting(2,2,29);
    check(game.spawnObjectForTesting(1,109,2,26,37) && game.spawnObjectForTesting(1,83,2,28,37),
          "Command Center reveals route and worker can reach it");
    advance(60);
    check(game.underConstructionObjectIds().empty() && game.resource(2,2)==29,
          "Segment affordability covers six tiles at five ore each");
    game.setResourceForTesting(2,2,60);advance(100);
    check(game.resource(2,2)==0,"Six wall tiles and one gate debit exactly sixty ore");
    advance(1500);
    check(game.aiObjectCountForTesting(2,"117",false)>0 && game.aiObjectCountForTesting(2,"64",false)>0,
          "AI builders finish wall pieces and the oriented gate");
    std::puts("PASS: AI wall exploration, whole segment cost, gate replacement and worker completion");
}
