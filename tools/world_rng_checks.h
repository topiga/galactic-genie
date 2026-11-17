// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "../src/core/genie_random.h"

static void worldRngChecks(swgb::Assets &assets) {
    // Original world61f736..61f9c7, seed1, empty object/update bodies:
    // Research/windows-reference/world-rng-order-v2-oracle-2026-11-16.json.
    // These checks cover empty-world RNG wrapping, not gameplay update ordering.
    struct Case { uint32_t slots, finalState; };
    const Case cases[]={{1,39341247u},{2,130938811u},{8,3458626948u}};
    for(const auto &c:cases) for(int bodyDraws:{0,4}) {
        swgb::Scenario scenario;
        scenario.map.width=scenario.map.height=32;
        scenario.map.tiles.resize(32*32);
        scenario.playerSlotCount=c.slots;
        scenario.enabledPlayerCount=1;
        scenario.civilizations.fill(1);
        for(uint32_t i=0;i<c.slots;++i) {
            auto &player=scenario.players[i];
            player.active=i==0;
            player.human=i==0;
            player.civilization=1;
            player.populationLimit=200;
        }
        swgb::Game game(assets);
        std::string error;
        check(game.initScenario(scenario,&error),error.c_str());
        game.seedLegacyRandomForTesting(1); // The oracle starts at world update, after startup.
        std::string body="(defrule (true) => ";
        for(int i=0;i<bodyDraws;++i)body+="(generate-random-number 32768) ";
        body+="(set-goal 994 1) (disable-self))";
        check(game.loadAiSourceForTesting(1,"world-rng-body",body,{},&error),error.c_str());
        game.update(0,{});
        check(game.aiGoalForTesting(1,994)==1,"Empty-world controlled PER body executes once");

        // Reloading the controller preserves the per-player random result and
        // saved world seed. Observe two PER draws after player one's saved draw.
        check(game.legacyRandomForTesting()==c.finalState,"Original first-step full CRT matches");
        uint32_t next=(c.finalState>>16)&32767u;
        swgb::genieRandomDraw(next);
        const int first=int(swgb::genieRandomDraw(next))+1;
        const int second=int(swgb::genieRandomDraw(next))+1;
        uint32_t bodyState=1;
        swgb::genieRandomDraw(bodyState);
        int retained=-1;
        for(int i=0;i<bodyDraws;++i)retained=int(swgb::genieRandomDraw(bodyState))+1;
        const std::string observe=
            "(defrule (random-number == "+std::to_string(retained)+") => (set-goal 995 1) (disable-self))\n"
            "(defrule (true) => (generate-random-number 32768) (disable-self))\n"
            "(defrule (random-number == "+std::to_string(first)+") => (set-goal 996 1) (generate-random-number 32768) (disable-self))\n"
            "(defrule (random-number == "+std::to_string(second)+") => (set-goal 997 1) (disable-self))\n";
        check(game.loadAiSourceForTesting(1,"world-rng-observe",observe,{},&error),error.c_str());
        game.setAmbientRandomForTesting(0xdeadbeefu);
        game.update(0,{});
        const std::string label="Original empty-world RNG: "+std::to_string(c.slots)+
            " instantiated slots, only human one active, "+std::to_string(bodyDraws)+" discarded AI draws";
        check(game.aiGoalForTesting(1,995)==1 && game.aiGoalForTesting(1,996)==1 &&
              game.aiGoalForTesting(1,997)==1,label.c_str());
        scenario.playerSlotCount=17;
        check(!game.initScenario(scenario,&error) && game.aiGoalForTesting(1,996)==1,
              "Invalid serialized player count leaves the existing world and controller intact");
    }
}
