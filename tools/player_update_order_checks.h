// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "../src/core/genie_random.h"

static void playerUpdateOrderChecks(swgb::Assets &assets) {
    // Synthetic scheduling controls composed from the original completed-building
    // self-upgrade RNG callback and normal-player PER save/restore wrapper.
    // These prove owner barriers, not original awake/sleep/auxiliary list ordering.
    auto &data=const_cast<swgb::dat::DatFile &>(assets.dat());
    auto &commands=data.effects[size_t(data.techs[501].effectId)].commands;
    struct Restore {
        std::vector<swgb::dat::EffectCommand> &commands,original;
        ~Restore(){commands=std::move(original);}
    } restore{commands,commands};
    // Bacta Tanks501 is disabled for this civilization. Give the synthetic
    // effect its own non-disabled technology ID instead of bypassing that gate.
    const int technologyId=int(data.techs.size());
    auto synthetic=data.techs[501];
    synthetic.civ=-1;
    synthetic.requiredTechCount=1;
    std::fill(std::begin(synthetic.requiredTechs),std::end(synthetic.requiredTechs),int16_t(-1));
    data.techs.push_back(synthetic);
    struct RestoreTechCount {
        std::vector<swgb::dat::Tech> &techs;size_t count;
        ~RestoreTechCount(){techs.resize(count);}
    } restoreTechCount{data.techs,size_t(technologyId)};
    swgb::Scenario scenario;
    scenario.map.width=scenario.map.height=32;
    scenario.map.tiles.resize(32*32);
    scenario.playerSlotCount=2;
    scenario.civilizations.fill(6);
    for(int player=1;player<=2;++player) {
        auto &slot=scenario.players[size_t(player-1)];
        slot.active=true;slot.human=player==1;slot.civilization=6;slot.populationLimit=200;
        slot.diplomacy[1]=player==1?0:3;slot.diplomacy[2]=player==2?0:3;
        swgb::ScenarioUnit center;
        center.unitId=109;center.spawnId=uint32_t(player);center.player=player;center.state=2;
        center.x=player==1?8.f:24.f;center.y=16.f;
        scenario.units.push_back(center);
    }
    scenario.nextUnitId=3;
    std::string error;
    for(unsigned mask=0;mask<4;++mask) {
        commands=restore.original;
        swgb::Game game(assets);
        check(game.initScenario(scenario,&error),error.c_str());
        commands={{3,109,109,-1,0}};
        game.invalidateUnitPrototypesForTesting();
        uint32_t expectedState=1;
        for(int player=1;player<=2;++player) {
            const bool queued=(mask&(1u<<unsigned(player-1)))!=0;
            check(!game.technologyResearched(player,technologyId),"Owner-order control starts without synthetic research");
            if(queued) {
                check(game.queueTechnologyForTesting(uint32_t(player),technologyId),"Queue owner-local upgrade completion");
                // Original556730 saves rand() and reseeds with that returned 15-bit value.
                expectedState=swgb::genieRandomDraw(expectedState);
            }
            const uint32_t saved=swgb::genieRandomDraw(expectedState);
            const int first=int(swgb::genieRandomDraw(expectedState))+1;
            const int second=int(swgb::genieRandomDraw(expectedState))+1;
            expectedState=saved; // PER draws are private to the normal-player wrapper.
            const std::string condition=queued?"(research-completed order-tech)":"(not (research-completed order-tech))";
            const std::string source=
                "(defconst order-tech "+std::to_string(technologyId)+")\n"
                "(defrule "+condition+" => (set-goal 970 1) (disable-self))\n"
                "(defrule (true) => (generate-random-number 32768) (disable-self))\n"
                "(defrule (random-number == "+std::to_string(first)+") => (set-goal 971 1) (generate-random-number 32768) (disable-self))\n"
                "(defrule (random-number == "+std::to_string(second)+") => (set-goal 972 1) (disable-self))\n";
            check(game.loadAiSourceForTesting(player,"owner-phase-barrier",source,{},&error),error.c_str());
        }
        game.update(1.f,{}); // Completes the 0.1-second queue even without power.
        for(int player=1;player<=2;++player) {
            const bool queued=(mask&(1u<<unsigned(player-1)))!=0;
            if(game.technologyResearched(player,technologyId)!=queued || game.productionQueueSizeForTesting(uint32_t(player))!=0)
                std::fprintf(stderr,"Owner barrier mask=%u player=%d queued=%d researched=%d queue=%zu remaining=%g\n",
                             mask,player,int(queued),int(game.technologyResearched(player,technologyId)),
                             game.productionQueueSizeForTesting(uint32_t(player)),double(game.productionRemainingForTesting(uint32_t(player))));
            check(game.technologyResearched(player,technologyId)==queued && game.productionQueueSizeForTesting(uint32_t(player))==0,
                  "Each queued owner completes exactly its own research during the admitted step");
            check(game.aiGoalForTesting(player,970)==1,
                  "Each player's AI observes its own completed production phase");
            check(game.aiGoalForTesting(player,971)==1 && game.aiGoalForTesting(player,972)==1,
                  "AI sees earlier and own upgrade RNG but never later-owner work from the same step");
        }
    }
    std::puts("PASS: four owner production/AI barriers with real research completion and shared RNG observations");
}
