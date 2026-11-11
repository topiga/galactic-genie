// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

static void conquestChecks(swgb::Assets &assets) {
    swgb::Game game(assets);std::string error;
    auto start=[&]() {check(game.initSkirmish(0x5A17,&error,false,1,3,75,true),error.c_str());};
    // The fixed opening uses IDs below 512; no reinforcements are produced in these zero-time fixtures.
    auto eliminate=[&](int player,uint32_t except=0) {
        for(uint32_t id=1;id<512;++id)
            if(id!=except && game.objectActive(id) && game.objectPlayer(id)==player)
                check(game.damageObjectForTesting(id,1000000),"Eliminate conquest fixture object");
    };
    start();const auto center=game.commandCenterId(2);const auto survivor=center+5;
    eliminate(2,survivor);game.update(0,{});
    check(game.matchResult()<0 && game.objectActive(survivor),"A surviving worker keeps its player in Conquest after the Command Center falls");
    eliminate(2);game.update(0,{});check(game.matchResult()==1,"Eliminating the remaining enemy force wins Conquest");
    start();eliminate(1);game.update(0,{});check(game.matchResult()==0,"Losing all owned forces loses Conquest");
    start();eliminate(1);eliminate(2);game.update(0,{});check(game.matchResult()==2,"Simultaneous elimination draws Conquest");
    start();
    auto foundation=game.spawnFoundationForTesting(1,70,1,12,24,{});
    check(foundation && game.setConstructionProgressForTesting(foundation,.5f),"Partially built foundation fixture");
    check(game.damageObjectForTesting(foundation,50),"Damage abandoned foundation");
    const float damaged=game.objectHitPoints(foundation);
    for(int i=0;i<30;++i) game.update(1.f/30,{});
    check(std::abs(game.objectHitPoints(foundation)-damaged)<.01f,"Abandoned construction does not regenerate lost health");
    check(game.completeFoundationForTesting(foundation),"Finish damaged construction");game.update(0,{});
    check(std::abs(game.objectHitPoints(foundation)-(game.objectMaxHitPoints(foundation)-50))<.01f,
          "Construction completion preserves combat damage");
    check(game.damageObjectForTesting(foundation,100000),"Destroy completed damaged foundation");
    start();
    const auto enemyHome=game.objectPosition(game.commandCenterId(2));
    const auto troop=game.objectUnitId(game.commandCenterId(2)+10);
    check(game.damageObjectForTesting(game.commandCenterId(2),1000000),"Remove AI Command Center");
    const auto reinforcement=game.spawnObjectForTesting(3,troop,2,enemyHome[0]+2,enemyHome[1]+6);
    check(reinforcement && game.spawnObjectForTesting(3,troop,2,enemyHome[0]+3,enemyHome[1]+6),"Complete surviving assault group");
    const auto before=game.moveAnchorForTesting(reinforcement);
    game.updateSkirmishArmyForTesting(2);
    check(game.moveAnchorForTesting(reinforcement)!=before,"Conquest AI keeps scouting after losing its original Command Center");
    std::puts("PASS: Conquest survivors, victory/defeat/draw and AI without its original Command Center");
}
