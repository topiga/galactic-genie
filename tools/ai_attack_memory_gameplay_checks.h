// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

static void aiAttackMemoryGameplayChecks(swgb::Assets &assets) {
    swgb::Scenario scenario;scenario.map.width=scenario.map.height=64;scenario.map.tiles.resize(64*64);
    scenario.nextUnitId=1;scenario.playerSlotCount=2;scenario.civilizations.fill(1);
    for(int i=0;i<2;++i) {
        scenario.players[i].active=true;scenario.players[i].human=i==0;scenario.players[i].civilization=1;
        scenario.players[i].diplomacy.fill(3);scenario.players[i].diplomacy[size_t(i+1)]=0;
    }
    swgb::Game game(assets);std::string error;
    check(game.initScenario(scenario,&error),error.c_str());
    const auto source=game.spawnObjectForTesting(1,307,1,21.f,20.f);
    const auto victim=game.spawnObjectForTesting(1,307,2,20.f,20.f);
    check(source && victim,"Attack-memory native damage fixtures");
    check(game.loadAiSourceForTesting(2,"attack-memory", "(defrule (true) => (disable-self))",{},&error),error.c_str());
    game.setAmbientRandomForTesting(1);
    check(game.damageObjectForTesting(victim,1,source),"Admit nonfatal damage notification adapter");
    check(game.attackMemoryQuarterForTesting(2,10).received==1 &&
          game.attackMemoryVictimCountForTesting(2)==1 && game.legacyRandomForTesting()==41,
          "Original X44 quarter10 and normal-owner15-bit RNG reseed reach runtime");
    game.damageObjectForTesting(victim,1,source);
    check(game.attackMemoryQuarterForTesting(2,10).received==1 &&
          game.attackMemoryVictimCountForTesting(2)==1 && game.legacyRandomForTesting()==172,
          "Repeated victim is deduplicated while owner notification still wraps RNG");
    game.update(0,{}); // First loaded-AI evaluation admits its military tick.
    check(game.attackMemoryVictimCountForTesting(2)==0,
          "Admitted native military tick clears processed victims even with no military groups");
    game.damageObjectForTesting(victim,1,source);
    check(game.attackMemoryQuarterForTesting(2,10).received==2,
          "Next admitted notification interval can record the same victim again");
    game.setAmbientRandomForTesting(1);
    game.damageObjectForTesting(source,1,victim);
    check(game.legacyRandomForTesting()==41 && game.attackMemoryQuarterForTesting(1,10).received==0,
          "Human/no-AI owner preserves wrapper RNG without tactical memory");
    game.setAmbientRandomForTesting(1);
    game.damageObjectForTesting(victim,1);
    check(game.legacyRandomForTesting()==1,"Sourceless damage has no actor500-to-owner513 wrapper");
    const auto doomed=game.spawnObjectForTesting(1,307,2,20.f,20.f);
    check(doomed!=0,"Fatal notification fixture");
    game.damageObjectForTesting(doomed,100000,source);
    check(game.attackMemoryQuarterForTesting(2,10).received==3,
          "Fatal damage records registered victim before native death removal");
    check(game.initScenario(scenario,&error),error.c_str());
    check(game.attackMemoryQuarterForTesting(2,10).received==0 &&
          game.attackMemoryVictimCountForTesting(2)==0,"New session clears attack memory and dedup state");
    std::puts("PASS: native attack-memory damage adapter, dedup lifecycle, fatal event and owner RNG wrapping");
}
