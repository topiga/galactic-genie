// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

static void aiResearchChecks(swgb::Assets &assets) {
    auto &data=const_cast<swgb::dat::DatFile &>(assets.dat());
    auto &tech=data.techs[1];
    struct Restore { swgb::dat::Tech &tech; swgb::dat::Tech saved; ~Restore(){tech=std::move(saved);} } restore{tech,tech};
    tech.requiredTechCount=0; tech.locationId=109; tech.researchTime=100; tech.civ=-1;
    tech.costs[0]={0,500,1}; tech.costs[1]={-1,0,0}; tech.costs[2]={-1,0,0};
    swgb::Game game(assets); std::string error;
    check(game.initSkirmish(0x5A17,&error,false,1,1),error.c_str());
    auto run=[&](const std::string &source) {
        check(game.loadAiSourceForTesting(1,"research-gates",source,{},&error),error.c_str());
        game.update(0,{});
    };
    auto canResearch=[&](const char *symbol="tech-level-2",bool escrow=false) {
        run(std::string("(defconst alias-tech 1)(defrule (")+
            (escrow?"can-research-with-escrow ":"can-research ")+symbol+
            ") => (set-goal 801 1))");
        return game.aiGoalForTesting(1,801)==1;
    };
    check(canResearch() && canResearch("alias-tech") && canResearch("1"),"AI research resolves age tokens, aliases and numeric IDs");
    check(!canResearch("bad-tech") && !canResearch("99999"),"Unknown/out-of-range research cannot match");
    const auto center=game.commandCenterId(1);
    check(game.queueUnitForTesting(center,83),"Queue busy researcher fixture");
    check(!canResearch(),"Queued training excludes research building");
    const auto foundation=game.spawnFoundationForTesting(1,109,1,45,45,{});
    check(foundation && !canResearch(),"Unfinished second researcher cannot satisfy gate");
    check(game.cancelProductionForTesting(center) && canResearch(),"Idle completed researcher satisfies gate again");
    check(game.queueTechnologyForTesting(center,1) && !canResearch(),"In-progress research is unavailable");
    check(game.cancelProductionForTesting(center),"Cancel research gate fixture");
    tech.requiredTechCount=1; std::fill(std::begin(tech.requiredTechs),std::end(tech.requiredTechs),-1);
    check(!canResearch(),"Missing prerequisites prevent research"); tech.requiredTechCount=0;
    tech.civ=2; check(!canResearch(),"Civilization-exclusive research is unavailable"); tech.civ=-1;
    game.setResourceForTesting(1,0,499);
    check(!canResearch() && !canResearch("tech-level-2",true),"Both gates enforce effective affordability");
    game.setResourceForTesting(1,0,500);
    run("(defrule (true) => (set-escrow-percentage food 100) (disable-self))");
    check(game.aiEscrowForTesting(1,0)==0 && canResearch(),"Changing escrow percentage doesn't reserve existing stock");
    game.setResourceForTesting(1,0,0);
    game.addResourceForTesting(1,0,600);
    check(game.resource(1,0)==600 && game.aiEscrowForTesting(1,0)==600,"Future income accumulates escrow");
    check(!canResearch() && canResearch("tech-level-2",true),"Research gates distinguish escrow and retain it through script reload");
    run("(defrule (can-research-with-escrow tech-level-2) => (release-escrow food) (release-escrow nova) (research tech-level-2) (disable-self))");
    check(game.resource(1,0)==100 && game.aiEscrowForTesting(1,0)==0 && game.productionQueueSizeForTesting(center)==1,
          "Original age-up rule releases savings and pays once into an idle researcher");
    game.addResourceForTesting(1,0,20);
    check(game.aiEscrowForTesting(1,0)==20,"Release retains income percentage");
    check(game.cancelProductionForTesting(center) && game.resource(1,0)==620 && game.aiEscrowForTesting(1,0)==20,
          "Paid research refund bypasses escrow income");
    run("(defrule (true) => (set-escrow-percentage food -5) (disable-self))");
    game.addResourceForTesting(1,0,10);
    check(game.aiEscrowForTesting(1,0)==20,"Zero-clamped rate keeps old escrow and saves no new income");
    run("(defrule (true) => (set-escrow-percentage food 150) (disable-self))");
    game.addResourceForTesting(1,0,10);
    check(game.aiEscrowForTesting(1,0)==30,"Escrow percentage clamps at 100");
    game.addResourceForTesting(1,0,-625);
    check(game.resource(1,0)==15 && game.aiEscrowForTesting(1,0)==15,"Resource removal caps escrow to remaining stock");
    check(game.aiEscrowForTesting(2,0)==0,"Escrow is isolated by player");
    check(game.researchTechnologyForTesting(1,1) && !canResearch(),"Completed research cannot be purchased again");
    check(game.initSkirmish(0x5A17,&error,false,1,1),error.c_str());
    check(game.aiEscrowForTesting(1,0)==0,"New match clears escrow");
    run("(defrule (true) => (set-goal 801 1))"); game.addResourceForTesting(1,0,10);
    check(game.aiEscrowForTesting(1,0)==0,"New match clears escrow percentage");
    std::puts("PASS: original AI research gates, busy researchers, age-up payment and persistent escrow lifecycle");
}
