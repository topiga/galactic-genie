// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
[[gnu::noinline]] static void conversionRetaliationChecks(swgb::Assets &assets) {
    for (bool human:{true,false}) for (bool moving:{false,true}) {
        swgb::Scenario scenario;scenario.map.width=scenario.map.height=64;
        scenario.map.tiles.resize(64*64);scenario.playerSlotCount=2;scenario.nextUnitId=1;
        scenario.civilizations.fill(1);
        for(int p=0;p<2;++p) {
            scenario.players[p].active=true;scenario.players[p].human=p==0?human:true;
            scenario.players[p].civilization=1;scenario.players[p].diplomacy.fill(3);
            scenario.players[p].diplomacy[size_t(p+1)]=0;
        }
        auto game=std::make_unique<swgb::Game>(assets);auto &g=*game;std::string error;
        check(g.initScenario(scenario,&error),error.c_str());
        const auto target=g.spawnObjectForTesting(1,329,1,25,20);
        const auto source=g.spawnObjectForTesting(1,115,2,20,20);
        check(target && source,"Create real conversion retaliation actors");
        g.setAttackModeForTesting(target,3);g.setAttackModeForTesting(source,3);
        check(g.loadAiSourceForTesting(1,"conversion-retaliation",
            "(defrule (true) => (disable-self))",{},&error),error.c_str());
        g.update(0,{});
        check(g.issueConversionForTesting(source,target),"Emit real conversion threat");
        for(int i=0;i<120 && !g.pendingConversionThreatsForTesting(target);++i)g.update(1.f/30,{});
        check(g.pendingConversionThreatsForTesting(target)==1,"Conversion500 is queued before response");
        check(g.stopUnitForTesting(source),"Stop further emissions before isolated response");
        if(moving)check(g.issuePointMoveForTesting(target,25,35),"Install higher movement order");
        const auto before=g.combatStats();const float hp=g.objectHitPoints(target);
        g.setAttackModeForTesting(target,0);
        g.update(0,{});
        const bool retaliates=!human || !moving;
        check((g.attackTargetForTesting(target)==source)==retaliates,
              "Original generic500 responds idle/computer, preserves human movement");
        check(g.combatStats().retaliationOrders==before.retaliationOrders+(retaliates?1:0),
              "Deferred response dispatches exactly one actual retaliation order");
        check(g.objectHitPoints(target)==hp && g.combatStats().attacksLanded==before.attacksLanded,
              "Retaliation notification does not fake damage or landed hits");
    }
    std::puts("PASS: conversion threat retaliation and human movement preservation");
}
