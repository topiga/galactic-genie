// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <filesystem>
#include <fstream>
#include <chrono>

static void aiContextGameplayChecks(swgb::Assets &assets) {
    swgb::Game game(assets);std::string error;
    check(game.initAridSkirmish(1,&error,false,75,true),error.c_str());
    const std::string source=
        "(defconst own-player my-player-number)\n(defconst own-civ my-civ)\n"
        "#load-if-defined EMPIRE\n#load-if-not-defined FEDERATION\n"
        "#load-if-defined TINY-MAP\n#load-if-defined DESERT-MAP\n"
        "#load-if-defined POPULATION-CAP-75\n#load-if-defined DIFFICULTY-MODERATE\n"
        "#load-if-defined TECH-LEVEL-1-START\n#load-if-defined LOW-RESOURCES-START\n"
        "(defrule (civ-selected own-civ) (civ-selected EMPIRE) (not (civ-selected FEDERATION)) => (set-goal 950 own-player))\n"
        "#end-if\n#end-if\n#end-if\n#end-if\n#end-if\n#end-if\n#end-if\n#end-if\n";
    check(game.loadAiSourceForTesting(2,"context-runtime",source,{},&error),error.c_str());game.update(0,{});
    check(game.aiGoalForTesting(2,950)==2,
          "Production Arid loader seeds own-player/civ aliases, DAT civilization enums and session defines before parsing");
    const auto rules=game.aiRuleCountForTesting(2);
    check(!game.loadAiSourceForTesting(2,"bad-context","(defconst fail not-a-context-constant)",{},&error),
          "Invalid replacement script fails");
    check(game.aiRuleCountForTesting(2)==rules && game.aiGoalForTesting(2,950)==2,
          "Failed production AI replacement preserves the running program and goal state");
    const std::string overrideSource=
        "#load-if-defined DIFFICULTY-HARD\n"
        "(defrule (difficulty == moderate) => (set-goal 951 1))\n#end-if\n"
        "#load-if-defined DIFFICULTY-MODERATE\n"
        "(defrule (true) => (set-goal 952 1))\n#end-if\n";
    check(game.loadAiSourceForTesting(2,"context-override",overrideSource,{"DIFFICULTY-HARD"},&error),error.c_str());
    game.update(0,{});
    check(game.aiGoalForTesting(2,951)==1 && game.aiGoalForTesting(2,952)==0,
          "Explicit hard compiler define replaces moderate define while runtime difficulty remains moderate");

    // Public filesystem entrypoint exercises context-before-include parsing too.
    const auto fixture=std::filesystem::temp_directory_path()/("galactic-genie-ai-context-checks-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::filesystem::create_directories(fixture);
    struct Cleanup {std::filesystem::path path;~Cleanup(){std::error_code ec;std::filesystem::remove_all(path,ec);}} cleanup{fixture};
    {
        std::ofstream root(fixture/"entry.per");root<<"(defconst root-owner my-player-number)\n(load \"child\")\n";
        std::ofstream child(fixture/"child.per");child<<
            "(defconst child-civ my-civ)\n#load-if-defined POPULATION-CAP-75\n"
            "(defrule (civ-selected child-civ) => (set-goal 953 root-owner))\n#end-if\n";
    }
    check(game.loadAiScript(2,(fixture/"entry.per").string(),{},&error),error.c_str());game.update(0,{});
    check(game.aiGoalForTesting(2,953)==2,"Production filesystem includes share preinstalled session constants and defines");
    check(game.initSkirmish(0x5A17,&error,false,1,2),error.c_str());
    check(game.loadAiSourceForTesting(2,"custom-map-context",
        "#load-if-defined DESERT-MAP\n(defrule (true) => (set-goal 954 1))\n#else\n"
        "(defrule (true) => (set-goal 955 1))\n#end-if\n",{},&error),error.c_str());game.update(0,{});
    check(game.aiGoalForTesting(2,954)==0 && game.aiGoalForTesting(2,955)==1,
          "Custom generated match does not inherit or invent the packaged desert map define");
    swgb::Scenario scenario;
    scenario.map.width=scenario.map.height=32;
    scenario.map.tiles.resize(32*32);
    auto &computer=scenario.players[1];
    computer.active=true;computer.human=false;computer.aiType=1;computer.civilization=1;
    computer.aiSource="#load-if-defined DESERT-MAP\n"
        "(defrule (map-type desert) (not (map-type raiders)) => (set-goal 956 1))\n"
        "#else\n(defrule (map-type 2) => (set-goal 956 2))\n#end-if\n";
    check(game.initScenario(scenario,&error,2,1,9),error.c_str());game.update(0,{});
    check(game.aiGoalForTesting(2,956)==1,"Scenario AI reads the selected lobby map during compilation and play");
    check(game.initScenario(scenario,&error),error.c_str());game.update(0,{});
    check(game.aiGoalForTesting(2,956)==2,"A fresh scenario uses the original initial lobby map");
    std::puts("PASS: production AI context, root/include aliases, explicit defines, reload atomicity and unknown map");
}
