// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
static void exploreRevisitGameplayChecks(swgb::Assets &assets) {
    swgb::Scenario scenario;scenario.map.width=scenario.map.height=32;scenario.map.tiles.resize(32*32);
    scenario.nextUnitId=1;scenario.playerSlotCount=2;scenario.civilizations.fill(1);
    for(int i=0;i<2;++i) {
        scenario.players[i].active=true;scenario.players[i].human=true;scenario.players[i].civilization=1;
        scenario.players[i].diplomacy.fill(3);scenario.players[i].diplomacy[size_t(i+1)]=0;
    }
    swgb::Game game(assets);std::string error;
    check(game.initScenario(scenario,&error),error.c_str());
    const auto timestamp=game.explorationRevisitTimeForTesting(2);
    const auto foodTimestamp=game.economyForTesting(2).foodRefreshTime;
    check(foodTimestamp && *foodTimestamp<=7500,"Scenario retains original third-draw food refresh metadata");
    check(timestamp && *timestamp<=60000,"Scenario retains real nested constructor revisit timestamp");
    check(game.loadAiSourceForTesting(2,"revisit-clock","(defrule (true) => (disable-self))",{},&error),error.c_str());
    check(game.explorationRevisitTimeForTesting(2)==timestamp && game.economyForTesting(2).foodRefreshTime==foodTimestamp,"Loading PER preserves player constructor context");
    for(int n=0;n<20;++n)game.updatePaused(.1f,{});
    check(game.explorationRevisitTimeForTesting(2)==timestamp,"Paused UI does not advance original InfoAI timer");
    for(int n=0;n<20;++n)game.update(.1f,{});
    check(game.explorationRevisitTimeForTesting(2) && *game.explorationRevisitTimeForTesting(2)<2000,
          "Admitted InfoAI callback normalizes future constructor timer through unsigned elapsed check");
    check(game.initAridSkirmish(1,&error),error.c_str());
    check(!game.explorationRevisitTimeForTesting(2) && !game.economyForTesting(2).foodRefreshTime,"RMS entry does not invent missing constructor context");
    std::puts("PASS: original constructor revisit metadata, AI callback timing and pause");
}
