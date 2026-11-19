// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Keep large Game fixtures out of the combined sanitizer driver's stack frame.
[[gnu::noinline]] static void unitLineGameplayChecks(swgb::Assets &assets) {
    const swgb::dat::UnitLine *laser=nullptr;
    for(const auto &line:assets.dat().unitLines)
        if(swgb::normalizeAiSymbol(line.name)=="unit-laser-line")laser=&line;
    check(laser && laser->unitIds.size()>=2,"Original DAT supplies the laser upgrade line");
    swgb::Game game(assets);std::string error;
    check(game.initSkirmish(0x5A17,&error,false,1,1),error.c_str());
    check(game.loadAiSourceForTesting(2,"unit-line-baseline","",{},&error),error.c_str());
    const int baseline=game.aiObjectCountForTesting(2,"UNIT-LASER-LINE",false);
    check(game.loadAiSourceForTesting(2,"original-unit-line",
        "(defconst laser-alias UNIT-LASER-LINE) "
        "(defrule (can-train laser-alias) => (train laser-alias) (set-goal 81 1) (disable-self)) "
        "(defrule (unit-type-count laser-alias >= 2) => (set-goal 82 1) (disable-self)) "
        "(defrule (unit-type-count-total laser-alias >= "+std::to_string(baseline+3)+") => (set-goal 83 1) (disable-self))",{},&error),error.c_str());
    check(game.spawnObjectForTesting(1,109,2,12,12)!=0 && game.spawnObjectForTesting(1,87,2,20,12)!=0,
          "Line training has actual Command Center capacity and Troop Center");
    for(int type=0;type<4;++type)game.setResourceForTesting(2,type,1000);
    for(size_t i=0;i<2;++i)check(game.spawnObjectForTesting(1,laser->unitIds[i],2,30.f+float(i),30)!=0,
        "Different members of the original laser line spawn");
    check(game.aiObjectCountForTesting(2,"laser-alias",false)==baseline+2 &&
          game.aiObjectCountForTesting(2,std::to_string(-laser->id),false)==baseline+2,
          "Symbol aliases and numeric line operands count all upgrades once");
    for(int tick=0;tick<65;++tick)game.update(.1f,{});
    check(game.aiGoalForTesting(2,81)==1 && game.aiGoalForTesting(2,82)==1 && game.aiGoalForTesting(2,83)==1,
          "Production PER can-train, train and mixed-upgrade count use DAT line identity");
    check(game.aiQueuedUnitCountForTesting(2,-laser->id)>0 ||
          game.aiObjectCountForTesting(2,"UNIT-LASER-LINE",false)>baseline+2,
          "Real line training produces a queued or completed member");
    check(game.aiObjectCountForTesting(2,"laser-alias",true)==
          game.aiObjectCountForTesting(2,"laser-alias",false)+int(game.aiQueuedUnitCountForTesting(2,-laser->id)),
          "Original total count includes pending training while current count excludes it");
    // A passive startup isolates garrison count changes from pending production.
    check(game.initSkirmish(0x5A17,&error,false,1,1),error.c_str());
    check(game.loadAiSourceForTesting(2,"unit-line-garrison","",{},&error),error.c_str());
    const auto container=game.spawnObjectForTesting(1,109,2,10,10);
    const auto passenger=game.spawnObjectForTesting(1,307,2,12,10);
    check(container && passenger,"Owned line member and container spawn");
    const int lineBefore=game.aiObjectCountForTesting(2,"UNIT-LASER-LINE",false);
    const int unitBefore=game.aiObjectCountForTesting(2,"307",false);
    check(game.garrisonForTesting(passenger,container),"Order original line member into a Command Center");
    for(int tick=0;tick<80 && game.garrisonContainerForTesting(passenger)<0;++tick)game.update(.1f,{});
    check(game.garrisonContainerForTesting(passenger)==int(container),"Line member actually enters the container");
    check(game.aiObjectCountForTesting(2,"UNIT-LASER-LINE",false)==lineBefore &&
          game.aiObjectCountForTesting(2,"307",false)==unitBefore,
          "Garrison preserves both line and individual prototype current counts");
    check(game.ejectGarrisonedUnitForTesting(container,passenger),"Unload counted line member");
    check(game.aiObjectCountForTesting(2,"UNIT-LASER-LINE",false)==lineBefore &&
          game.aiObjectCountForTesting(2,"307",false)==unitBefore,
          "Unloading does not double-register prototype counts");
    check(game.spawnObjectForTesting(1,87,2,20,12)!=0 &&
          game.researchTechnologyForTesting(2,261),
          "Empire Troop Center and trooper upgrade are available");
    for(int type=0;type<4;++type)game.setResourceForTesting(2,type,1000);
    check(game.loadAiSourceForTesting(2,"upgraded-trooper",
        "(defrule (can-train 448) => (train 448) (set-goal 84 1) (disable-self))",{},&error),error.c_str());
    for(int tick=0;tick<65;++tick)game.update(.1f,{});
    check(game.aiGoalForTesting(2,84)==1 && game.aiQueuedUnitCountForTesting(2,-laser->id)==1,
          "An upgraded numeric troop operand uses its enabled base and queues at its Troop Center");
    check(game.loadAiSourceForTesting(2,"civilization-operands",
        "(defconst medic-alias UNIT-MEDIC) (defconst raw-alias 167) "
        "(defconst upgrade-alias ut-laser-2) "
        "(defrule (true) => (set-goal 85 medic-alias) (set-goal 86 raw-alias) "
        "(set-goal 87 upgrade-alias) (set-goal 88 my-unique-unit-line) (set-goal 89 my-unique-unit) (disable-self))",{},&error),error.c_str());
    for(int tick=0;tick<65;++tick)game.update(.1f,{});
    check(game.aiGoalForTesting(2,85)==939 && game.aiGoalForTesting(2,86)==167 &&
          game.aiGoalForTesting(2,87)==261 && game.aiGoalForTesting(2,88)==26 && game.aiGoalForTesting(2,89)==993,
          "Empire PER maps named medic and laser research operands and preserves numeric aliases");
    std::puts("PASS production DAT unit-line constants, aliases, counts and training");
}
