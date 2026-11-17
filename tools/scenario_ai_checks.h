// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
static void scenarioAiChecks(swgb::Assets &assets) {
    swgb::Scenario scenario;
    scenario.map.width=scenario.map.height=32;scenario.map.tiles.resize(32*32);
    scenario.playerSlotCount=3;scenario.civilizations.fill(1);
    for(int i=0;i<3;++i){auto &p=scenario.players[i];p.active=true;p.human=i==0;p.civilization=1;}
    scenario.players[0].aiType=0;scenario.players[0].aiName="missing-human";
    auto &computer=scenario.players[1];computer.aiType=0;computer.aiName="embedded";
    computer.aiSource="(defrule (difficulty == easy) => (set-goal 919 7) (disable-self))";
    scenario.players[2].aiType=2;
    swgb::Game game(assets);std::string error;
    check(game.initScenario(scenario,&error,3),error.c_str());
    check(game.aiRuleCountForTesting(1)==0&&game.aiRuleCountForTesting(2)==1&&game.aiRuleCountForTesting(3)==0,
          "Scenario activates only configured computer personalities");
    check(game.aiGoalForTesting(2,919)==0,"Loading scenario AI does not evaluate rules");
    game.update(1.f/30,{});
    check(game.aiGoalForTesting(2,919)==7,"First admitted AI evaluation sees selected scenario difficulty");
    scenario.players[2].aiType=0;scenario.players[2].aiName="missing-personality";
    check(!game.initScenario(scenario,&error,3)&&game.aiGoalForTesting(2,919)==7,
          "Missing later personality leaves existing world and AI intact");
    scenario.players[2].aiType=255;
    check(!game.initScenario(scenario,&error,3)&&game.aiGoalForTesting(2,919)==7,
          "Unsupported AI mode fails before reset");
    check(!game.initScenario(scenario,&error,5),"Invalid scenario difficulty rejected");
    scenario.players[2].active=false;
    check(game.initScenario(scenario,&error,2),error.c_str());game.update(1.f/30,{});
    check(game.aiGoalForTesting(2,919)==0,"Reload resets AI and changes difficulty facts consistently");
    computer.aiType=1;computer.aiName="RandomGame";computer.aiSource.clear();
    check(game.initScenario(scenario,&error,3),error.c_str());
    check(game.aiRuleCountForTesting(2)>0,"Original standard personality loads from installed game archives");
    std::puts("PASS: scenario AI activation, deferred evaluation, difficulty, inactive/human isolation and atomic script failure");
}
