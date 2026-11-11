// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

static void aiTimeChecks(swgb::Assets &assets) {
    swgb::Game game(assets); std::string error;
    // Packaged gamedata_x1.drs resource60001 gates age advancement with game-time >=600.
    const std::string source=
        "(defrule (game-time >= 2) => (set-goal 701 1) (disable-self))\n"
        "(defrule (game-time == 1) => (set-goal 702 1) (disable-self))\n";
    auto load=[&] {
        check(game.loadAiSourceForTesting(2,"game-time-check",source,{},&error),error.c_str());
    };
    check(game.init(0x5A17,64,&error,1,1),error.c_str());
    load();
    game.update(.75f,{});
    check(game.aiGoalForTesting(2,701)==0 && game.aiGoalForTesting(2,702)==0,
          "AI game-time excludes fractional seconds before the threshold");
    game.update(.5f,{});
    check(game.aiGoalForTesting(2,702)==1 && game.aiGoalForTesting(2,701)==0,
          "AI game-time reports elapsed whole simulation seconds");
    game.updatePaused(10,{});
    load(); game.update(0,{});
    check(game.aiGoalForTesting(2,702)==1 && game.aiGoalForTesting(2,701)==0,
          "Pause does not advance game-time and reloading AI does not reset it");
    game.update(.75f,{});
    check(game.aiGoalForTesting(2,701)==1,"AI game-time reaches its threshold after resumed simulation");
    load(); game.update(0,{});
    check(game.aiGoalForTesting(2,701)==1,"Late-loaded AI observes time elapsed before loading");
    check(game.init(0x5A17,64,&error,1,1),error.c_str());
    load(); game.update(0,{});
    check(game.aiGoalForTesting(2,701)==0 && game.aiGoalForTesting(2,702)==0,
          "Starting a new game resets the AI game clock");
    game.update(3,{});
    swgb::Scenario scenario;
    scenario.map.width=scenario.map.height=32;
    scenario.map.tiles.resize(32*32);
    scenario.civilizations.fill(1);
    for(int i=0;i<2;++i) {
        scenario.players[size_t(i)].active=true;
        scenario.players[size_t(i)].civilization=1;
    }
    check(game.initScenario(scenario,&error),error.c_str());
    load(); game.update(0,{});
    check(game.aiGoalForTesting(2,701)==0 && game.aiGoalForTesting(2,702)==0,
          "Scenario initialization resets the AI game clock");
    game.update(2,{});
    check(game.aiGoalForTesting(2,701)==1 && game.matchTime()==0,
          "Scenario AI clock advances independently of the custom skirmish clock");
    std::puts("PASS: AI game-time, whole seconds, pause, late loading and game/scenario resets");
}
