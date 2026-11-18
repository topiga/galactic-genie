// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
[[gnu::noinline]] static void crowdYieldGameplayChecks(swgb::Assets &assets) {
    swgb::Scenario scenario;scenario.map.width=scenario.map.height=64;
    scenario.map.tiles.resize(64*64);scenario.nextUnitId=1;
    scenario.playerSlotCount=1;scenario.civilizations.fill(1);
    scenario.players[0].active=true;scenario.players[0].human=true;scenario.players[0].civilization=1;
    const auto &unit=assets.dat().civs[1].units[83];
    const float radius=std::max(.1f,std::max(unit.collisionSize[0],unit.collisionSize[1]));
    const float separation=2*radius+.05f,asideDistance=2*radius+.3f;
    for(bool bothSidesBlocked:{true,false}) {
        swgb::Game game(assets);std::string error;
        check(game.initScenario(scenario,&error),error.c_str());
        const float bx=20+separation+.005f;
        const auto mover=game.spawnObjectForTesting(1,83,1,20,20);
        const auto blocker=game.spawnObjectForTesting(1,83,1,bx,20);
        check(mover && blocker,"Crowd yield mover and idle blocker fixture");
        const float preferredSide=((mover^blocker)&1u)?1.f:-1.f;
        check(game.spawnObjectForTesting(1,83,1,bx,20+preferredSide*asideDistance)!=0,
              "Preferred side-step destination occupied");
        if(bothSidesBlocked)check(game.spawnObjectForTesting(1,83,1,bx,20-preferredSide*asideDistance)!=0,
                                 "Alternative side-step destination occupied");
        game.update(0,{});
        check(game.issuePointMoveForTesting(mover,25,20),"Move through a packed friendly cluster");
        game.update(.05f,{});
        const auto blockerProgress=game.movementProgressForTesting(blocker);
        if(bothSidesBlocked)
            check(blockerProgress[0]>0,"An idle blocker searches beyond two occupied perpendicular destinations");
        else {
            check(blockerProgress[0]>0,"An idle blocker can still yield to the unoccupied alternative side");
            for(int i=0;i<10;++i)game.update(.05f,{});
            const auto position=game.objectPosition(blocker);
            check((position[1]-20)*preferredSide<0,"The alternative yield moves away from the occupied preferred side");
        }
        bool arrived=false;
        for(int i=0;i<600 && !arrived;++i) {
            game.update(.05f,{});
            const auto position=game.objectPosition(mover);
            arrived=std::hypot(position[0]-25,position[1]-20)<.5f;
        }
        check(arrived,"The mover bypasses the friendly cluster and reaches its destination");
    }
    {
        auto game=std::make_unique<swgb::Game>(assets);std::string error;
        check(game->initScenario(scenario,&error),error.c_str());
        const float bx=20+separation+.005f;
        const auto mover=game->spawnObjectForTesting(1,83,1,20,20);
        const auto blocker=game->spawnObjectForTesting(1,83,1,bx,20);
        // A fully occupied neighborhood, not merely the two legacy side points.
        // Keep both starting centers clear so this exercises mobile blocking.
        for(int iy=-7;iy<=7;++iy)for(int ix=-7;ix<=7;++ix){
            const float x=bx+ix*.5f,y=20+iy*.5f;
            if(std::hypot(x-bx,y-20)<separation ||
               std::hypot(x-20,y-20)<separation)continue;
            check(game->spawnObjectForTesting(1,83,1,x,y)!=0,
                  "Fill the idle blocker's surrounding destinations");
        }
        game->update(0,{});
        check(game->issuePointMoveForTesting(mover,25,20),"Request movement into the fully occupied neighborhood");
        const auto before=game->objectPosition(blocker);
        game->update(.05f,{});
        check(game->movementProgressForTesting(blocker)[0]==0 &&
              game->objectPosition(blocker)==before,
              "A fully enclosed idle blocker receives no invalid escape route");
    }
}
