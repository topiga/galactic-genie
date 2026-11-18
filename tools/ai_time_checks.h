// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

static void aiTimeChecks(swgb::Assets &assets) {
    swgb::Game game(assets); std::string error;
    // PER runs on owner callbacks, independently of elapsed simulation time.
    const auto advanceAndEvaluate=[&](float dt) {
        game.update(dt,{});
        for(int i=0;i<20;++i)game.update(0,{});
    };
    // Packaged gamedata_x1.drs resource60001 gates age advancement with game-time >=600.
    const std::string source=
        "(defrule (game-time >= 2) => (set-goal 701 1) (disable-self))\n"
        "(defrule (game-time == 1) => (set-goal 702 1) (disable-self))\n";
    auto load=[&] {
        check(game.loadAiSourceForTesting(2,"game-time-check",source,{},&error),error.c_str());
    };
    check(game.init(0x5A17,64,&error,1,1),error.c_str());
    load();
    advanceAndEvaluate(.75f);
    check(game.aiGoalForTesting(2,701)==0 && game.aiGoalForTesting(2,702)==0,
          "AI game-time excludes fractional seconds before the threshold");
    advanceAndEvaluate(.5f);
    check(game.aiGoalForTesting(2,702)==1 && game.aiGoalForTesting(2,701)==0,
          "AI game-time reports elapsed whole simulation seconds");
    game.updatePaused(10,{});
    load(); advanceAndEvaluate(0);
    check(game.aiGoalForTesting(2,702)==1 && game.aiGoalForTesting(2,701)==0,
          "Pause does not advance game-time and reloading AI does not reset it");
    advanceAndEvaluate(.75f);
    check(game.aiGoalForTesting(2,701)==1,"AI game-time reaches its threshold after resumed simulation");
    load(); advanceAndEvaluate(0);
    check(game.aiGoalForTesting(2,701)==1,"Late-loaded AI observes time elapsed before loading");
    check(game.init(0x5A17,64,&error,1,1),error.c_str());
    load(); advanceAndEvaluate(0);
    check(game.aiGoalForTesting(2,701)==0 && game.aiGoalForTesting(2,702)==0,
          "Starting a new game resets the AI game clock");
    advanceAndEvaluate(3);
    swgb::Scenario scenario;
    scenario.map.width=scenario.map.height=32;
    scenario.map.tiles.resize(32*32);
    scenario.civilizations.fill(1);
    for(int i=0;i<2;++i) {
        scenario.players[size_t(i)].active=true;
        scenario.players[size_t(i)].civilization=1;
    }
    check(game.initScenario(scenario,&error),error.c_str());
    load(); advanceAndEvaluate(0);
    check(game.aiGoalForTesting(2,701)==0 && game.aiGoalForTesting(2,702)==0,
          "Scenario initialization resets the AI game clock");
    advanceAndEvaluate(2);
    check(game.aiGoalForTesting(2,701)==1 && game.matchTime()==0,
          "Scenario AI clock advances independently of the custom skirmish clock");
    check(game.initSkirmish(0x5A17,&error),error.c_str());
    check(game.loadAiSourceForTesting(2,"match-clock", "(defrule (true) => (disable-self))",{},&error),error.c_str());
    constexpr float step=1.f/30;
    for(int frame=0;frame<3000;++frame)game.update(step,{});
    check(game.matchResult()==-1 && game.matchTime()==float(3000*double(step)),
          "Skirmish clock follows session time without cumulative float drift");
    const float matchTime=game.matchTime();
    game.updatePaused(10,{});
    check(game.matchTime()==matchTime,"Pause preserves the session-based skirmish clock");
    game.update(step,{});
    check(game.matchTime()==float(3001*double(step)),"Skirmish clock resumes from session time");
    check(game.initSkirmish(0x5A17,&error),error.c_str());
    check(game.loadAiSourceForTesting(2,"local-match-clock","(defrule (true) => (disable-self))",{},&error),error.c_str());
    game.updateLocal(1000,0,{});
    for(uint32_t frame=1;frame<=1500;++frame)game.updateLocal(1000+frame*50,.05f,{});
    check(game.matchTime()==112.5f,"Local skirmish clock follows admitted integer milliseconds at original speed");
    game.updatePaused(10,{});game.updateLocal(1000000,0,{});
    check(game.matchTime()==112.5f,"Local clock rebases after pause without advancing match time");
    game.updateLocal(1000050,.05f,{});
    check(game.matchTime()==112.575f,"Local match clock resumes after host-time rebasing");
    std::puts("PASS: AI game-time, whole seconds, pause, late loading and game/scenario resets");
}
