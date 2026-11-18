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
            ") (disable-self))",{},&error),error.c_str());
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
    std::puts("PASS: production original economic quotas, resource memory, pause and civilian exploration");
}
