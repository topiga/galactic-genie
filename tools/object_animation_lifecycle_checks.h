// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

static void objectAnimationLifecycleChecks(swgb::Assets &assets) {
    // Original factory/update/draw chains in object-factory-animation-findings-2026-11-16.json.
    // This checks sprite clocks, not awake/sleep list membership or destruction timing.
    swgb::Scenario scenario;
    scenario.map.width=scenario.map.height=32;scenario.map.tiles.resize(32*32);
    scenario.playerSlotCount=1;scenario.civilizations.fill(1);
    scenario.players[0].active=scenario.players[0].human=true;
    scenario.players[0].civilization=1;
    // Packaged Gaia tree411 is a base10 object; do not substitute a player-civilization prototype.
    for (const int type:{10,-411,20,70}) {
        const auto &units=assets.dat().civs[type==70?1:0].units;
        const auto found=std::find_if(units.begin(),units.end(),[type](const auto &unit) {
            return unit.exists && (type==-411?unit.id==411:unit.type==type) && unit.standingGraphic[0]>=0;
        });
        if (found==units.end()) throw std::runtime_error("Missing original animation fixture type "+std::to_string(type));
        swgb::ScenarioUnit object;
        object.unitId=found->id;object.spawnId=uint32_t(scenario.units.size()+1);
        object.player=type==70?1:0;object.state=2;object.x=4.f+float(object.spawnId)*5;object.y=12;
        scenario.units.push_back(object);
    }
    scenario.nextUnitId=5;
    swgb::Game game(assets);std::string error;
    check(game.initScenario(scenario,&error),error.c_str());
    std::array<float,4> initial{};
    for (uint32_t id=1;id<=4;++id) initial[id-1]=game.objectAnimationForTesting(id)[0];
    game.update(.25f,{});
    for (uint32_t id=1;id<=4;++id) {
        const float elapsed=game.objectAnimationForTesting(id)[0]-initial[id-1];
        check(std::abs(elapsed-(id<=2?0.f:.25f))<.00001f,
              "Static/tree sprites keep their frame while idle animated/mobile sprites advance");
    }
    const auto beforePause=game.objectAnimationForTesting(3);
    game.updatePaused(1,{});
    check(game.objectAnimationForTesting(3)==beforePause,"Pause freezes animated sprite clocks without pretending idle means sleep");
    std::puts("PASS: original base/static versus animated sprite clocks and pause");
}
