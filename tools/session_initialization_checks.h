// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

static void sessionInitializationChecks(swgb::Assets &assets) {
    swgb::Game game(assets);std::string error;
    swgb::Scenario scenario;
    scenario.map.width=scenario.map.height=32;
    scenario.map.tiles.resize(32*32);
    scenario.civilizations.fill(1);
    scenario.players[0].active=scenario.players[0].human=true;
    scenario.players[0].civilization=1;
    scenario.players[0].populationLimit=200;
    swgb::ScenarioTrigger timer;
    timer.enabled=true;
    swgb::ScenarioCondition elapsed;
    elapsed.type=10;elapsed.fields.assign(8,-1);elapsed.fields[7]=5;
    timer.conditions.push_back(elapsed);
    scenario.triggers.push_back(timer);
    check(game.initScenario(scenario,&error),error.c_str());
    game.update(3,{});
    check(!game.triggerFired(0),"First scenario timer is partially elapsed");
    check(game.initScenario(scenario,&error),error.c_str());
    game.update(2,{});
    check(!game.triggerFired(0),"Same-sized scenario reload discards previous elapsed trigger time");
    game.update(3,{});
    check(game.triggerFired(0),"Reloaded trigger fires after its own complete interval");
    check(game.initScenario(scenario,&error),error.c_str());
    check(!game.triggerFired(0) && game.triggerEnabled(0),
          "Same-sized scenario reload discards the previous fired flag");

    const auto named=[&](const char *name) {
        for(const auto &unit:assets.dat().civs[1].units)
            if(unit.exists && unit.name2==name)return int(unit.id);
        return -1;
    };
    const int shield=named("BLDG-SHLDGEN"),power=named("BLDG-POWERCORE");
    check(shield>=0 && power>=0,"Original shield generator and power core fixture assets");
    scenario.triggers.clear();
    const auto place=[&](int unit,uint32_t id,float x,float y) {
        swgb::ScenarioUnit object;
        object.unitId=uint16_t(unit);object.spawnId=id;object.player=1;object.state=2;
        object.x=x;object.y=y;scenario.units.push_back(object);
    };
    place(shield,1,15,15);place(power,2,18,15);place(293,3,15,18);
    scenario.nextUnitId=4;
    check(game.initScenario(scenario,&error),error.c_str());
    check(game.objectShielded(3),"First session populates shield generator index cache");
    std::swap(scenario.units[0],scenario.units[2]);
    check(game.initScenario(scenario,&error),error.c_str());
    swgb::Game fresh(assets);
    check(fresh.initScenario(scenario,&error),error.c_str());
    check(fresh.objectShielded(3) && game.objectShielded(3),
          "Equal-sized scenario reload rebuilds shield indices after object order changes");
    std::puts("PASS: scenario trigger elapsed/fired state and same-size shield-cache session resets");
}
