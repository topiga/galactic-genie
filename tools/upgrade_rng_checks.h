// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "../src/core/genie_random.h"

static void upgradeRngChecks(swgb::Assets &assets) {
    // 21 original4c2a70 controls with observable native completed/idle sprites.
    // Sources: retained rms-upgrade-rng-controls-summary and rms-upgrade-rng-mobile-v1-oracle (2026-11-16).
    // The 12 original construction/snow sprite-state controls need a separate visual-state fixture.
    struct Case {const char *name;uint32_t seed,after;int source,target;bool spawnMobile,randomGraphic;};
    const Case cases[]={
        {"normal",1u,41u,109,71,false,false},
        {"normal",12345u,7584u,109,71,false,false},
        {"normal",4294967295u,35u,109,71,false,false},
        {"animated-random",1u,1469331435u,109,71,false,true},
        {"animated-random",12345u,4088502554u,109,71,false,true},
        {"animated-random",4294967295u,1537853365u,109,71,false,true},
        {"mobile-self",1u,1u,558,558,true,false},
        {"mobile-self",12345u,12345u,558,558,true,false},
        {"mobile-self",4294967295u,4294967295u,558,558,true,false},
        {"building-self",1u,41u,109,109,false,false},
        {"building-self",12345u,7584u,109,109,false,false},
        {"building-self",4294967295u,35u,109,109,false,false},
        {"no-matching-object",1u,1u,71,141,false,false},
        {"no-matching-object",12345u,12345u,71,141,false,false},
        {"no-matching-object",4294967295u,4294967295u,71,141,false,false},
        {"mobile-changed-plain",1u,1u,558,293,true,false},
        {"mobile-changed-plain",12345u,12345u,558,293,true,false},
        {"mobile-changed-plain",4294967295u,4294967295u,558,293,true,false},
        {"mobile-changed-random",1u,3357800067u,558,293,true,true},
        {"mobile-changed-random",12345u,1255958651u,558,293,true,true},
        {"mobile-changed-random",4294967295u,1948984945u,558,293,true,true},
    };
    auto &data=const_cast<swgb::dat::DatFile &>(assets.dat());
    auto &commands=data.effects[size_t(data.techs[501].effectId)].commands;
    struct RestoreCommands {
        std::vector<swgb::dat::EffectCommand> &commands,original;
        ~RestoreCommands(){commands=std::move(original);}
    } restore{commands,commands};
    swgb::Game game(assets);std::string error;
    const auto observe=[&](uint32_t expectedState) {
        // Observe two consecutive draws through the real PER action/fact interface, no RNG getter.
        uint32_t next=expectedState;
        const int first=int(swgb::genieRandomDraw(next))+1;
        const int second=int(swgb::genieRandomDraw(next))+1;
        const std::string source=
            "(defrule (random-number == -1) => (set-goal 950 1) (disable-self))\n"
            "(defrule (true) => (generate-random-number 32768) (disable-self))\n"
            "(defrule (random-number == "+std::to_string(first)+") => (set-goal 951 1) (generate-random-number 32768) (disable-self))\n"
            "(defrule (random-number == "+std::to_string(second)+") => (set-goal 952 1) (disable-self))\n";
        check(game.loadAiSourceForTesting(1,"original-upgrade-rng",source,{},&error),error.c_str());
        game.update(0,{});
        check(game.aiGoalForTesting(1,950)==1 && game.aiGoalForTesting(1,951)==1 && game.aiGoalForTesting(1,952)==1,
              "Original upgrade RNG feeds two consecutive AI draws in the same ordered PER pass");
    };
    for(const auto &c:cases) {
        commands=restore.original;
        check(game.init(c.seed,64,&error,6,1),error.c_str());
        if(c.spawnMobile) {
            check(game.objectCountForTesting(1,c.source)==0,"Upgrade mobile source is initially absent");
            check(game.spawnObjectForTesting(6,c.source,1,8.5f,8.5f)!=0,"Create one original mobile upgrade source");
        }
        check(game.objectCountForTesting(1,c.source)==(c.source==71?0u:1u),
              "Exactly one matching source, or original no-matching-object control");
        auto &graphic=data.graphics[size_t(data.civs[6].units[size_t(c.target)].standingGraphic[0])];
        const auto oldFlags=graphic.sequenceType;
        struct RestoreFlags {uint8_t &flags,old;~RestoreFlags(){flags=old;}} restoreFlags{graphic.sequenceType,oldFlags};
        if(c.randomGraphic)graphic.sequenceType|=4;
        else if(c.source==558 && c.target==293)graphic.sequenceType&=uint8_t(~4u);
        commands={{3,int16_t(c.source),int16_t(c.target),-1,0}};
        game.invalidateUnitPrototypesForTesting();
        check(game.researchTechnologyForTesting(1,501),"Apply actual upgrade RNG control");
        observe(c.after);
    }
    // Static callback branches are mutually exclusive: standing A→B must not then use dying B→C.
    {
        commands=restore.original;
        check(game.init(1,64,&error,6,1),error.c_str());
        check(game.spawnObjectForTesting(6,558,1,8.5f,8.5f)!=0,"Standing/dying branch control");
        auto &source=data.civs[6].units[558];auto &target=data.civs[6].units[293];
        const int16_t oldSourceDying=source.dyingGraphic,oldTargetDying=target.dyingGraphic;
        struct RestoreDying {
            int16_t &source,&target,oldSource,oldTarget;
            ~RestoreDying(){source=oldSource;target=oldTarget;}
        } restoreDying{source.dyingGraphic,target.dyingGraphic,oldSourceDying,oldTargetDying};
        source.dyingGraphic=target.standingGraphic[0];
        target.dyingGraphic=source.standingGraphic[0];
        auto &middle=data.graphics[size_t(target.standingGraphic[0])];
        auto &last=data.graphics[size_t(source.standingGraphic[0])];
        struct RestoreSequence {
            uint8_t &middle,&last,oldMiddle,oldLast;
            ~RestoreSequence(){middle=oldMiddle;last=oldLast;}
        } restoreSequence{middle.sequenceType,last.sequenceType,middle.sequenceType,last.sequenceType};
        middle.sequenceType=last.sequenceType=5;
        commands={{3,558,293,-1,0}};
        game.invalidateUnitPrototypesForTesting();
        check(game.researchTechnologyForTesting(1,501),"Apply mutually exclusive graphic refresh control");
        observe(3357800067u); // Two draws; a mistaken standing→dying cascade would consume four.
    }
    std::puts("PASS: 21 original building/mobile upgrade RNG controls and shared AI stream continuation");
}
