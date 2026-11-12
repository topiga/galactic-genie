// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

static void aiTimerChecks(swgb::Assets &assets) {
    swgb::Game game(assets); std::string error;
    check(game.init(0x5A17,64,&error,1,1),error.c_str());
    const std::string source=
        "(defconst t-test 1)\n"
        "(defrule (true) => (enable-timer t-test 1) (enable-timer 20 2) (enable-timer 3 4) (disable-self))\n"
        "(defrule (timer-triggered t-test) => (set-goal 701 1) (disable-timer t-test) (enable-timer t-test 2) (disable-self))\n"
        "(defrule (goal 701 1) (timer-triggered t-test) => (set-goal 702 1) (disable-self))\n"
        "(defrule (timer-triggered 20) => (set-goal 703 1) (disable-self))\n"
        "(defrule (timer-triggered 20) => (set-goal 704 1) (disable-self))\n"
        "(defrule (game-time >= 1) => (enable-timer 3 1) (disable-self))\n"
        "(defrule (timer-triggered 3) => (set-goal 705 1) (disable-self))\n"
        "(defrule (game-time >= 3) => (disable-timer 20) (disable-self))\n"
        "(defrule (game-time >= 3) (not (timer-triggered 20)) => (set-goal 706 1) (disable-self))\n";
    check(game.loadAiSourceForTesting(1,"original-timer-pattern",source,{},&error),error.c_str());
    check(game.loadAiSourceForTesting(2,"timer-isolation",
          "(defrule (timer-triggered 1) => (set-goal 701 1))",{},&error),error.c_str());
    game.update(0,{});
    game.update(.75f,{});
    game.updatePaused(10,{});
    check(game.aiGoalForTesting(1,701)==0,"AI timers wait for simulation time and freeze on pause");
    game.update(.25f,{});
    check(game.aiGoalForTesting(1,701)==1 && game.aiGoalForTesting(1,702)==0,
          "Timer expires at deadline; disable/re-enable starts a fresh interval");
    game.update(1.f,{});
    check(game.aiGoalForTesting(1,702)==0 && game.aiGoalForTesting(1,703)==1 &&
          game.aiGoalForTesting(1,704)==1 && game.aiGoalForTesting(1,705)==1,
          "Triggered timers stay latched across rules; re-enable overwrites an active deadline");
    game.update(1.f,{});
    check(game.aiGoalForTesting(1,702)==1 && game.aiGoalForTesting(1,706)==1,
          "Restarted timer expires after its new interval and disabling clears triggered state");
    check(game.aiGoalForTesting(2,701)==0,"AI timers belong to individual players");
    const std::string invalid=
        "(defrule (true) => (enable-timer 0 0) (enable-timer 21 0) (enable-timer -1 0) (enable-timer 2 -1) (enable-timer 4 0) (disable-self))\n"
        "(defrule (timer-triggered 4) (not (timer-triggered 0)) (not (timer-triggered 21)) (not (timer-triggered -1)) (not (timer-triggered 2)) => (set-goal 707 1) (disable-self))\n";
    check(game.loadAiSourceForTesting(1,"timer-bounds",invalid,{},&error),error.c_str());
    game.update(0,{});
    check(game.aiGoalForTesting(1,707)==1,"Zero delay is immediate; invalid IDs and negative delays are rejected");
    check(game.init(0x5A17,64,&error,1,1),error.c_str());
    check(game.loadAiSourceForTesting(1,"timer-reset",
          "(defrule (timer-triggered 4) => (set-goal 707 1))",{},&error),error.c_str());
    game.update(10,{});
    check(game.aiGoalForTesting(1,707)==0,"New games clear AI timers");
    check(game.loadAiSourceForTesting(1,"original-nand-pattern",
          "(defrule (nand (true) (true)) => (set-goal 708 1))"
          "(defrule (nand (true) (false)) => (set-goal 709 1))"
          "(defrule (nand (false) (true)) => (set-goal 710 1))"
          "(defrule (nand (false) (false)) => (set-goal 711 1))"
          "(defrule (nand (unimplemented-fact) (true)) => (set-goal 712 1))"
          "(defrule (nand (unimplemented-fact) (false)) => (set-goal 713 1))"
          "(defrule (nand (true) (not (false))) => (set-goal 714 1))",{},&error),error.c_str());
    game.update(0,{});
    check(game.aiGoalForTesting(1,708)==0 && game.aiGoalForTesting(1,709)==1 &&
          game.aiGoalForTesting(1,710)==1 && game.aiGoalForTesting(1,711)==1 &&
          game.aiGoalForTesting(1,712)==0 && game.aiGoalForTesting(1,713)==1 &&
          game.aiGoalForTesting(1,714)==0,
          "Original AI nand truth table, nesting and unsupported-condition propagation");
    std::puts("PASS: original AI timer reset, expiry, latch, disable, bounds, player isolation and pause");
}
