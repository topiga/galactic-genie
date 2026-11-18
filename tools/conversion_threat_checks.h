// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
[[gnu::noinline]] static void conversionThreatChecks(swgb::Assets &assets) {
    swgb::Scenario scenario;scenario.map.width=scenario.map.height=64;
    scenario.map.tiles.resize(64*64);scenario.playerSlotCount=2;scenario.nextUnitId=1;
    scenario.civilizations.fill(1);
    for(int p=0;p<2;++p) {
        scenario.players[p].active=true;scenario.players[p].human=true;
        scenario.players[p].civilization=1;scenario.players[p].diplomacy.fill(3);
        scenario.players[p].diplomacy[size_t(p+1)]=0;
    }
    auto game=std::make_unique<swgb::Game>(assets);auto &g=*game;std::string error;
    check(g.initScenario(scenario,&error),error.c_str());
    // Target owner1 updates before converter owner2: emitted500 must remain
    // pending until the following owner1 pass, rather than deliver immediately.
    const auto target=g.spawnObjectForTesting(1,329,1,25,20);
    const auto source=g.spawnObjectForTesting(1,115,2,20,20);
    check(target && source,"Create deferred conversion threat actors");
    g.setAttackModeForTesting(target,3);g.setAttackModeForTesting(source,3);
    check(g.loadAiSourceForTesting(1,"conversion-threat",
        "(defrule (true) => (disable-self))",{},&error),error.c_str());
    g.update(0,{});
    const float hp=g.objectHitPoints(target);
    const auto hits=g.combatStats().attacksLanded;
    check(g.issueConversionForTesting(source,target),"Start production conversion threat source");
    for(int i=0;i<120 && !g.pendingConversionThreatsForTesting(target);++i)g.update(1.f/30,{});
    check(g.pendingConversionThreatsForTesting(target)==1 &&
          g.attackMemoryQuarterForTesting(1,10).received==0,
          "First conversion500 waits for target-owner update");
    g.updatePaused(1,{});
    check(g.pendingConversionThreatsForTesting(target)==1 &&
          g.attackMemoryQuarterForTesting(1,10).received==0,
          "Pause does not drain queued conversion threat");
    g.update(1.f/30,{});
    check(g.attackMemoryQuarterForTesting(1,10).received==1,
          "Target update dispatches deferred owner513 through shared attack memory");
    check(g.objectHitPoints(target)==hp && g.combatStats().attacksLanded==hits,
          "Conversion threat inflicts no HP damage and adds no landed-hit statistics");
    check(g.stopUnitForTesting(source),"Stop subsequent conversion emissions");
    g.update(1.f/30,{});
    check(g.pendingConversionThreatsForTesting(target)==0,
          "Already emitted threat drains once even after source cancels");
    std::puts("PASS: deferred conversion threat, paused delivery, shared memory and no damage");
}
