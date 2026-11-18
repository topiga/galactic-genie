// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
static void economyGameplayChecks(swgb::Assets &assets) {
    swgb::Scenario scenario;scenario.map.width=scenario.map.height=64;scenario.map.tiles.resize(64*64);
    scenario.nextUnitId=1;scenario.playerSlotCount=2;scenario.civilizations.fill(1);
    for(int i=0;i<2;++i) {
        scenario.players[i].active=true;scenario.players[i].human=i==0;scenario.players[i].civilization=1;
        scenario.players[i].diplomacy.fill(3);scenario.players[i].diplomacy[size_t(i+1)]=0;
    }
    int berry=-1;for(const auto &u:assets.dat().civs[0].units)
        if(u.cls==27 && u.id>=0 && u.standingGraphic[0]>=0) {berry=u.id;break;}
    check(berry>=0,"Economic fixture has original forage asset");
    swgb::Game game(assets);std::string error;
    auto initialize=[&](bool explorers) {
        check(game.initScenario(scenario,&error),error.c_str());
        check(game.loadAiSourceForTesting(2,"original-economy-runtime",
            std::string("(defrule (true) => (set-strategic-number sn-food-gatherer-percentage 100) ")+
            "(set-strategic-number sn-carbon-gatherer-percentage 0) (set-strategic-number sn-metal-gatherer-percentage 0) "
            "(set-strategic-number sn-nova-gatherer-percentage 0) (set-strategic-number sn-percent-civilian-explorers "+
            (explorers?"34":"0")+") (set-strategic-number sn-cap-civilian-explorers "+(explorers?"1":"0")+
            ") (disable-self)) "
            "(defrule (sheep-and-forage-too-far) => (set-goal 90 1)) "
            "(defrule (not (sheep-and-forage-too-far)) => (set-goal 90 0))",{},&error),error.c_str());
    };
    auto advance=[&](int ticks) {for(int n=0;n<ticks;++n)game.update(.1f,{});};
    initialize(false);
    std::vector<int32_t> workers;
    for(int n=0;n<3;++n)workers.push_back(int32_t(game.spawnObjectForTesting(1,83,2,20.f+n,20.f)));
    const auto food=game.spawnObjectForTesting(0,berry,0,22,22);
    const auto unseen=game.spawnObjectForTesting(0,berry,0,55,55);
    check(food && unseen && std::all_of(workers.begin(),workers.end(),[](int id){return id>0;}),"Economic production fixtures spawn");
    advance(65);
    const auto &economy=game.economyForTesting(2);
    check(economy.civilianIds==workers,"Production economy retains civilian admission order");
    check(economy.gathererQuota==3 && economy.explorerQuota==0 && economy.desired[0]==3,
          "Production PER reaches original quota and rounded allocation");
    check(economy.resourceSlots.count(food)==1 && economy.resourceSlots.count(unseen)==0,
          "Production resource admission uses actual sight, not unknown world objects");
    check(game.aiGathererCountForTesting(2,0)>0,"Production PER assigns workers through the recovered resource selector");
    uint32_t loadedWorker=0;
    for(int tick=0;tick<300 && !loadedWorker;++tick) {
        advance(1);
        for(int32_t id:workers)if(game.objectCarriedAmount(uint32_t(id))>0) {loadedWorker=uint32_t(id);break;}
    }
    check(loadedWorker!=0,"Economic worker gathers a real fractional cargo");
    const float cargo=game.objectCarriedAmount(loadedWorker);
    // Other active gather actions also retain movement-child references. Free the
    // deposit before testing retry; original class27 admission rejects more than3 references.
    for(int32_t id:workers)check(game.stopUnitForTesting(uint32_t(id)),"Interrupt the economic gathering group");
    game.updateAiGatherersForTesting(2);
    check(game.aiGatherTargetForTesting(loadedWorker)==food && game.objectCarriedAmount(loadedWorker)==cargo,
          "Original gather controller resumes the registered resource without discarding cargo");
    const auto metricSlot=game.economyForTesting(2).resourceSlots.at(food);
    const auto depotMetric=[&] {return game.economyForTesting(2).resources[size_t(metricSlot.first)].records[metricSlot.second].depotId;};
    const auto evaluateFoodFact=[&] {for(int tick=0;tick<25;++tick)game.update(0,{});return game.aiGoalForTesting(2,90);};
    const auto center=game.spawnObjectForTesting(1,109,2,10,10);
    check(center!=0,"Economic command center fixture spawns");
    game.updateAiGatherersForTesting(2);
    check(depotMetric()==int32_t(center),"Command center admission refreshes already known resource metrics");
    check(evaluateFoodFact()==1,"Original food fact reports only a distant remembered forage depot");
    const auto processor=game.spawnObjectForTesting(1,68,2,25,25);
    check(processor!=0,"Economic food processing center fixture spawns");
    game.updateAiGatherersForTesting(2);
    check(depotMetric()==int32_t(processor),"Closer processing center refreshes retained food metric");
    check(evaluateFoodFact()==0,"Original food fact recognizes a close cached forage depot");
    check(game.damageObjectForTesting(processor,100000),"Remove the cached processing center");
    game.updateAiGatherersForTesting(2);
    check(depotMetric()==int32_t(center),"Depot removal recomputes the retained resource metric");
    check(evaluateFoodFact()==1,"Original food fact reflects the removed processing center without reloading AI");
    for(size_t i=0;i<workers.size();++i) {
        check(game.stopUnitForTesting(uint32_t(workers[i])) && game.moveObjectForTesting(uint32_t(workers[i]),3.f+float(i),3.f),
              "Remove resource sight and stop old assignments");
    }
    check(!game.targetVisibleForTesting(2,food),"Known forage is now hidden");
    const auto newcomer=game.spawnObjectForTesting(1,83,2,3,5);
    check(newcomer!=0,"Fresh idle worker can use the inherited resource registry");
    game.updateAiGatherersForTesting(2);
    check(game.aiGatherTargetForTesting(newcomer)==food,
          "Remembered forage remains eligible after sight loss");
    check(game.economyForTesting(2).resourceSlots.count(unseen)==0,"Unobserved forage is still excluded");
    const auto resourceSlot=game.economyForTesting(2).resourceSlots.at(food);
    const auto beforeSelections=game.economyForTesting(2).resources[size_t(resourceSlot.first)].records[resourceSlot.second].selections;
    game.updateAiGatherersForTesting(2);
    check(game.economyForTesting(2).resources[size_t(resourceSlot.first)].records[resourceSlot.second].selections==beforeSelections,
          "Satisfied active gathering is not reassigned each economy tick");
    for(int n=0;n<20;++n)game.updatePaused(.1f,{});
    check(game.economyForTesting(2).resources[size_t(resourceSlot.first)].records[resourceSlot.second].selections==beforeSelections,
          "Pause does not run economic allocation or resource selection");
    check(game.damageObjectForTesting(uint32_t(workers.back()),100000),"Remove a registered worker");
    game.updateAiGatherersForTesting(2);
    check(std::none_of(game.economyForTesting(2).economicWorkers.begin(),game.economyForTesting(2).economicWorkers.end(),
        [&](const auto &record){return record.id==workers.back();}),"Dead worker is removed before subsequent economic assignment");

    initialize(true);
    workers.clear();for(int n=0;n<3;++n)workers.push_back(int32_t(game.spawnObjectForTesting(1,83,2,20.f+n,20.f)));
    check(game.spawnObjectForTesting(0,berry,0,22,22)!=0,"Mixed economic exploration forage fixture");
    advance(65);
    const auto &mixed=game.economyForTesting(2);
    check(mixed.explorerQuota==1 && mixed.gathererQuota==2 && mixed.civilianExplorers.size()==1,
          "Original quota shares civilians between exploration and gathering");
    const int32_t explorer=mixed.civilianExplorers.front();
    check(explorer==workers.front() && game.explorationStateForTesting(uint32_t(explorer))>=0,
          "First admitted idle civilian receives a real exploration action");
    check(std::none_of(mixed.economicWorkers.begin(),mixed.economicWorkers.end(),
        [&](const auto &record){return record.id==explorer;}),"Exploration and gathering registries do not own the same worker");
    check(game.aiGathererCountForTesting(2,0)>0,"Civilian exploration coexists with actual food gathering");
    initialize(false);
    check(game.spawnObjectForTesting(1,109,2,16,16)!=0,"Delivery fixture command center");
    for(int n=0;n<3;++n)check(game.spawnObjectForTesting(1,83,2,20.f+n,20.f)!=0,"Delivery fixture workers");
    check(game.spawnObjectForTesting(0,berry,0,22,22)!=0,"Delivery fixture forage");
    const float foodBeforeDelivery=game.resource(2,0);
    bool delivered=false;
    for(int n=0;n<1200 && !delivered;++n) {
        game.update(.1f,{});
        int credited=0;for(const auto &record:game.economyForTesting(2).economicWorkers)
            credited+=std::max(0,record.completed);
        delivered=game.resource(2,0)>foodBeforeDelivery;
        check(delivered?credited>0:credited==0,"AI batch credit follows actual delivery, not gathering progress");
    }
    check(delivered,"Native workers finish a real gather-and-deposit cycle");
    std::puts("PASS: production original economic quotas, resource memory, pause and civilian exploration");
}
