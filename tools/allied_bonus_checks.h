// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Actual original601f90 selector with complete recipient masters and explicit zero gates.
static void alliedBonusChecks(swgb::Assets &assets) {
    const struct {
        int secondCiv,diplomacy12,diplomacy21;
        std::vector<int> effects1,effects2;
        float sight1,sight2;
    } cases[]={
        {6,3,3,{9,15},{9,15},7,7},
        {6,0,0,{9,15},{9,15},7,7},
        {6,0,3,{9,15},{9,15},7,7},
        {1,3,3,{9,15},{4,10},7,5},
        {1,0,0,{9,10,15},{4,10,15},7,7},
        {1,0,3,{9,10,15},{4,10},7,5},
    };
    swgb::Game game(assets);std::string error;
    for(const auto &c:cases) {
        swgb::Scenario scenario;
        scenario.map.width=scenario.map.height=24;scenario.map.tiles.resize(24*24);
        for(int p=0;p<2;++p) {
            auto &player=scenario.players[size_t(p)];player.active=true;player.human=p==0;
            player.civilization=scenario.civilizations[size_t(p)]=p?c.secondCiv:6;
            player.populationLimit=200;
            player.diplomacy[1]=p?c.diplomacy21:0;
            player.diplomacy[2]=p?0:c.diplomacy12;
        }
        check(game.initScenario(scenario,&error),error.c_str());
        check(game.civilizationEffectsForTesting(1)==c.effects1 && game.civilizationEffectsForTesting(2)==c.effects2,
              "Original whole selector orders own tree and unique directional allied civilization teams");
        for(int player=1;player<=2;++player) {
            const auto &unit=assets.dat().civs[player==1?6:c.secondCiv].units[4];
            const float expected=player==1?c.sight1:c.sight2;
            check(game.unitAttributeForTesting(player,unit,1,unit.lineOfSight)==expected &&
                  game.unitSearchRadiusForTesting(player,unit)==expected,
                  "Original whole-selector team15 mutation remains recipient-specific and deduplicated");
        }
    }
    std::puts("PASS: original whole allied-team selector, directional alliances and same-civilization deduplication");
}
