// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
static void aiBuildEscrowChecks(swgb::Assets &assets) {
    swgb::Game game(assets);std::string error;
    check(game.initSkirmish(0x5A17,&error,false,1,1),error.c_str());
    const auto run=[&](const std::string &source) {
        check(game.loadAiSourceForTesting(1,"build-escrow",source,{},&error),error.c_str());game.update(0,{});
    };
    run("(defrule (true) => (set-escrow-percentage carbon 100) (disable-self))");
    check(game.aiObjectCountForTesting(-1,"70")==0 && game.aiObjectCountForTesting(17,"70")==0,
          "AI unit constants reject invalid player slots");
    const auto costs=game.unitCostsForTesting(1,assets.dat().civs[1].units[70]);
    for(int type=0;type<4;++type)game.setResourceForTesting(1,type,0);
    float carbon=0;
    for(const auto &cost:costs)if(cost.flag && cost.type>=0 && cost.amount>0) {
        game.addResourceForTesting(1,cost.type,cost.amount);
        if(cost.type==1)carbon+=cost.amount;
    }
    check(carbon>0 && game.aiEscrowForTesting(1,1)==carbon,"Prefab cost is held in real AI escrow");
    const std::string probe="(defconst own-prefab 70)(defrule (can-build own-prefab) => (set-goal 960 1))"
        "(defrule (can-build-with-escrow own-prefab) => (set-goal 961 1))";
    run(probe);
    check(game.aiGoalForTesting(1,960)==0 && game.aiGoalForTesting(1,961)==1,
          "Original build gates differ only in including escrowed income");
    game.setResourceForTesting(1,1,carbon-1);run(probe);
    check(game.aiGoalForTesting(1,960)==0 && game.aiGoalForTesting(1,961)==0,
          "Escrow-aware build still requires the full effective cost");
}
