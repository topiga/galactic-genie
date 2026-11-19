// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

[[gnu::noinline]] static void dropsiteFactGameplayChecks(swgb::Assets &assets) {
    swgb::Scenario scenario;scenario.map.width=scenario.map.height=64;
    scenario.map.tiles.resize(64*64);scenario.nextUnitId=1;
    scenario.playerSlotCount=2;scenario.civilizations.fill(1);
    for(int i=0;i<2;++i) {
        scenario.players[i].active=true;scenario.players[i].human=i==0;
        scenario.players[i].civilization=1;scenario.players[i].diplomacy.fill(3);
        scenario.players[i].diplomacy[size_t(i+1)]=0;
    }
    swgb::Game game(assets);std::string error;
    check(game.initScenario(scenario,&error),error.c_str());
    const auto center=game.spawnObjectForTesting(1,109,2,20,20);
    const auto observer=game.spawnObjectForTesting(1,83,2,24,24);
    check(center && observer,"Dropsite comparison has a command center and sight source");
    check(game.loadAiSourceForTesting(2,"dropsite-fact-runtime",
        "(defrule (true) => (set-strategic-number sn-food-gatherer-percentage 100) (disable-self))"
        "(defrule (game-time < 1) (dropsite-min-distance food == -1)"
        " (dropsite-min-distance carbon == -1) (dropsite-min-distance metal == -1)"
        " (dropsite-min-distance nova == -1) => (set-goal 960 1))"
        "(defrule (dropsite-min-distance 9 == 0) (dropsite-min-distance -1 == 0)"
        " => (set-goal 961 1))"
        "(defrule (not (dropsite-min-distance unresolved-resource == 0)) => (set-goal 962 1))"
        "(defrule (game-time >= 2) (dropsite-min-distance food == 4)"
        " (dropsite-min-distance carbon == 3) (dropsite-min-distance metal == 5)"
        " (dropsite-min-distance nova == 6) => (set-goal 963 1))"
        "(defrule (game-time >= 5) (dropsite-min-distance food == 4)"
        " (dropsite-min-distance carbon == 3) (dropsite-min-distance metal == 5)"
        " (dropsite-min-distance nova == 6) => (set-goal 964 1))",{},&error),error.c_str());
    const auto advance=[&](int ticks){for(int i=0;i<ticks;++i)game.update(.1f,{});};
    advance(10);
    check(game.aiGoalForTesting(2,960)==1,"Empty resource registries retain original minimum -1");
    check(game.aiGoalForTesting(2,961)==1 && game.aiGoalForTesting(2,962)==0,
          "Invalid numeric resource compares as zero; unknown symbol remains unknown under not");
    const auto prototype=[&](int cls) {
        for(const auto &unit:assets.dat().civs[0].units)
            if(unit.id>=0 && unit.cls==cls && unit.standingGraphic[0]>=0)return int(unit.id);
        return -1;
    };
    const int food=prototype(27),carbon=prototype(31),metal=prototype(30),nova=prototype(29);
    check(food>=0 && carbon>=0 && metal>=0 && nova>=0,"Four original resource classes available");
    const auto laterRow=game.spawnObjectForTesting(0,carbon,0,23,20);
    const auto laterColumn=game.spawnObjectForTesting(0,carbon,0,20,17);
    const auto firstTile=game.spawnObjectForTesting(0,carbon,0,20,23);
    check(laterRow && laterColumn && firstTile && game.spawnObjectForTesting(0,food,0,24,20) &&
          game.spawnObjectForTesting(0,metal,0,20,25) &&
          game.spawnObjectForTesting(0,nova,0,26,20),"Observe four distinct depot distances");
    // The second PER evaluation precedes that tick's economy refresh. Wait for
    // the following evaluation to observe the newly admitted resource cache.
    advance(35);
    const auto &trees=game.economyForTesting(2).resources[1];
    check(trees.records.size()==3 && trees.records[0].id==int32_t(firstTile) &&
          trees.records[1].id==int32_t(laterColumn) && trees.records[2].id==int32_t(laterRow) &&
          trees.cachedBestId==int32_t(firstTile),"Equal-distance trees retain original revealed tile order");
    check(game.aiGoalForTesting(2,963)==1,
          "PER reads each resource's registered center-to-depot distance with original ID mapping");
    check(game.moveObjectForTesting(center,3,3) && game.moveObjectForTesting(observer,4,4),
          "Remove sight and relocate depot without admitting a new depot");
    advance(25);
    check(game.aiGoalForTesting(2,964)==1,
          "Fact reads retained metric cache instead of scanning hidden resources or recomputing geometry");
    std::puts("PASS: production dropsite-min-distance empty, mapped, invalid and remembered cache facts");
}
