// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
static void scenarioImportChecks(swgb::Assets &assets) {
    swgb::Scenario scenario;
    scenario.map.width=scenario.map.height=32;scenario.map.tiles.resize(32*32);
    scenario.playerSlotCount=2;scenario.civilizations.fill(1);
    for(int i=0;i<2;++i){auto &p=scenario.players[i];p.active=true;p.human=i==0;p.civilization=1;}
    swgb::ScenarioUnit center;center.unitId=109;center.player=1;center.state=2;center.x=16;center.y=16;
    swgb::ScenarioUnit worker=center;worker.spawnId=1;worker.unitId=83;worker.garrisonedInId=0;
    scenario.units={center,worker};scenario.nextUnitId=2;
    swgb::ScenarioTrigger remove;remove.enabled=true;
    swgb::ScenarioEffect effect;effect.type=15;effect.selectedUnitIds={0};remove.effects.push_back(effect);
    swgb::ScenarioTrigger victory;victory.enabled=true;
    swgb::ScenarioCondition destroyed;destroyed.type=6;destroyed.fields.resize(4,-1);destroyed.fields[2]=0;destroyed.fields[3]=0;victory.conditions.push_back(destroyed);
    effect={};effect.type=13;effect.fields.resize(8,-1);effect.fields[7]=1;victory.effects.push_back(effect);
    swgb::ScenarioTrigger delayedDefeat=victory;
    swgb::ScenarioCondition timer;timer.type=10;timer.fields.resize(8,-1);timer.fields[7]=1;
    delayedDefeat.conditions={timer};delayedDefeat.effects.front().fields[7]=2;
    scenario.triggers={remove,victory,delayedDefeat};scenario.triggerOrder={0,1,2};
    swgb::Game game(assets);std::string error;
    check(game.initScenario(scenario,&error),error.c_str());
    check(game.objectRegisteredForTesting(2)&&game.objectUnitId(2)==109&&game.objectRegisteredForTesting(1),
          "Imported ID0 gets an addressable unused ID without changing nonzero IDs");
    check(game.garrisonContainerForTesting(1)==2,"Garrison reference to original ID0 follows the remap");
    check(scenario.units[0].spawnId==0&&scenario.units[1].garrisonedInId==0&&scenario.triggers[0].effects[0].selectedUnitIds[0]==0&&scenario.triggers[1].conditions[0].fields[3]==0,
          "Import normalization leaves source scenario untouched");
    game.update(.2f,{});
    check(!game.objectActive(2)&&game.matchResult()==1,
          "Selected-object effect and destroyed-object condition both address remapped ID0");
    game.update(1.f,{});
    check(game.matchResult()==1,"Later scenario triggers cannot reverse a finished match");
    check(game.spawnObjectForTesting(1,83,1,5,5)==3,"Future object IDs cannot collide with the imported replacement");
    scenario.units.push_back(center);
    check(!game.initScenario(scenario,&error),"Ambiguous duplicate zero IDs are rejected");
    scenario.units.pop_back();scenario.units[1].spawnId=uint32_t(std::numeric_limits<int32_t>::max());
    check(!game.initScenario(scenario,&error)&&game.matchResult()==1,
          "Unrepresentable signed-reference replacement rejects before resetting the match");
    std::puts("PASS: scenario ID0 registration, garrison and trigger references, future IDs and source immutability");
}
