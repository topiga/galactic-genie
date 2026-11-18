// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

static void aiCommodityChecks(swgb::Assets &assets) {
    swgb::Game game(assets);
    std::string error;
    auto run=[&](int owner,const std::string &condition,const std::string &action) {
        const auto rule="(defrule "+condition+" => "+action+" (disable-self))";
        check(game.loadAiSourceForTesting(owner,"spaceport",rule,{},&error),error.c_str());
        game.update(0,{});
    };
    auto reset=[&](){check(game.initSkirmish(42,&error,false,1,1),error.c_str());};
    reset();
    game.setResourceForTesting(1,0,100);
    game.setResourceForTesting(1,3,0);
    run(1,"(true)","(sell-commodity food)");
    check(game.resource(1,0)==100 && game.resource(1,3)==0,"Trade needs a completed Spaceport");
    check(game.spawnObjectForTesting(1,84,1,45,45)!=0,"Spawn trading Spaceport");
    run(1,"(can-sell-commodity food) (commodity-selling-price food == 70)","(sell-commodity food)");
    check(game.resource(1,0)==0 && game.resource(1,3)==70,"Sell 100 food at the original opening quote");
    check(game.spawnObjectForTesting(1,84,2,50,50)!=0,"Spawn the other player's Spaceport");
    game.setResourceForTesting(2,3,126);
    game.setResourceForTesting(2,0,0);
    run(2,"(true)","(buy-commodity food)");
    check(game.resource(2,0)==0 && game.resource(2,3)==126,"Unaffordable trade has no effect");
    game.setResourceForTesting(2,3,127);
    run(2,"(can-buy-commodity food) (commodity-buying-price food == 127)","(buy-commodity food)");
    check(game.resource(2,0)==100 && game.resource(2,3)==0,"Buyer receives 100 food at the shared reduced quote");
    run(2,"(commodity-buying-price food == 130) (commodity-buying-price metal == 169)","(set-goal 801 1)");
    check(game.aiGoalForTesting(2,801)==1,"Offsetting trades restore the opening rate and preserve ore price");
    game.setResourceForTesting(1,1,0);
    run(1,"(true)","(set-escrow-percentage carbon 100)");
    game.addResourceForTesting(1,1,100);
    run(1,"(can-sell-commodity carbon)","(sell-commodity carbon)");
    check(game.resource(1,1)==100 && game.resource(1,3)==70,"Reserved carbon is unavailable to the seller");
    run(1,"(true)","(sell-commodity carbon)");
    check(game.resource(1,1)==0 && game.aiEscrowForTesting(1,1)==0 && game.resource(1,3)==140,
          "Direct sale checks total stock and caps escrow after the debit");
    game.setResourceForTesting(1,2,100);
    run(1,"(true)","(sell-commodity metal)");
    reset();
    run(1,"(commodity-selling-price metal == 91)","(set-goal 801 1)");
    check(game.aiGoalForTesting(1,801)==1,"New skirmish restores the original opening ore quote");
}
