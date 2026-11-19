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
(defrule (wall-completed-percentage 1 == 100)
         (can-afford-complete-wall 1 BLDG-BARRIER-LINE) => (set-goal 909 1))
(defrule (can-build-wall 2 BLDG-BARRIER-LINE) => (build-wall 2 BLDG-BARRIER-LINE) (disable-self))
(defrule (can-build-gate-with-escrow 2) => (build-gate 2) (disable-self))
)",{},&error),error.c_str());
    const auto advance=[&](int steps){for(int i=0;i<steps;++i)game.update(.1f,{});};
    advance(60);
    check(game.underConstructionObjectIds().empty() && game.resource(2,2)==60,
          "Unseen wall route stays unpurchased");
    check(game.aiGoalForTesting(2,909)==1,"Disabled inner plan is complete and costs zero ore");
    game.setResourceForTesting(2,2,29);
    check(game.spawnObjectForTesting(1,109,2,26,37),"Command Center reveals route");
    const auto farm=game.spawnObjectForTesting(1,50,2,26,42);
    const auto farmer=game.spawnObjectForTesting(1,83,2,28,42);
    game.setObjectResourceForTesting(farm,300);
    check(game.issueGatherForTesting(farmer,farm),"Worker assigned to farm");
    advance(60);
    check(game.underConstructionObjectIds().empty() && game.resource(2,2)==29,
          "Segment affordability covers six tiles at five ore each");
    game.setResourceForTesting(2,2,60);advance(100);
    check(game.resource(2,2)==0,"Six wall tiles and one gate debit exactly sixty ore");
    const auto foundations=game.underConstructionObjectIds();
    check(!foundations.empty() && std::all_of(foundations.begin(),foundations.end(),[&](auto id) {
        return game.constructionBuilderId(id)!=farmer;
    }),"AI leaves the farmer working when wall foundations need builders");
    check(game.spawnObjectForTesting(1,83,2,28,37),"Add available builder");
    advance(1500);
    check(game.aiObjectCountForTesting(2,"117",false)>0 && game.aiObjectCountForTesting(2,"64",false)>0,
          "AI builders finish wall pieces and the oriented gate");
    game.setAiWallSegmentForTesting(2,2,{2,24,33,29,33,6,true});
    check(game.loadAiSourceForTesting(2,"direct-wall-order",
          "(defrule (true) => (build-wall 2 BLDG-BARRIER-LINE))",{},&error),error.c_str());
    game.setResourceForTesting(2,2,5);advance(100);
    const auto sites=game.underConstructionObjectIds();
    check(sites.size()==1 && game.objectUnitId(sites.front())==117 && game.resource(2,2)==0,
          "Direct wall order places only the tiles the player can pay for");
    // Expected master and center from the original gate selector (58a4b0) on these same segments.
    struct GateCase { int length, dx, dy, master; float x,y; };
    const GateCase cases[]={
        {3,0,1,490,10.5f,21},{3,1,0,487,11,20.5f},{3,1,1,665,11,21},{3,1,-1,673,12,19},
        {4,0,1,490,10.5f,22},{4,1,0,487,12,20.5f},{4,1,1,665,12,22},{4,1,-1,673,12,19}
    };
    for(const auto &c:cases) {
        check(game.initScenario(scenario,&error),error.c_str());
        check(game.researchTechnologyForTesting(2,1),"Enable gate");
        game.setResourceForTesting(2,2,60);
        check(game.spawnObjectForTesting(1,109,2,21,48),"Reveal gate site");
        const int last=c.length-1;
        game.setAiWallSegmentForTesting(2,2,{2,10,20,10+last*c.dx,20+last*c.dy,c.length,true});
        check(game.loadAiSourceForTesting(2,"oriented-gate",R"(
(defrule (true) => (enable-wall-placement 2) (disable-self))
(defrule (can-build-wall 2 117) => (build-wall 2 117) (disable-self))
(defrule (can-build-gate 2) => (build-gate 2) (disable-self))
)",{},&error),error.c_str());
        advance(150);
        const auto sites=game.underConstructionObjectIds();
        check(std::any_of(sites.begin(),sites.end(),[&](auto id) {
            const auto position=game.objectPosition(id);
            return game.objectUnitId(id)==c.master && position[0]==c.y && 64-position[1]==c.x;
        }),"Queued gate master and center match Clone Campaigns");
    }
    std::puts("PASS: AI wall exploration, whole segment cost, gate replacement and worker completion");
}
