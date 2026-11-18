// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Native movement-adapter regression, not an original pathfinder parity claim.
[[gnu::noinline]] static void overlappingStartMovementChecks(swgb::Assets &assets) {
    swgb::Scenario scenario;scenario.map.width=scenario.map.height=64;
    scenario.map.tiles.resize(64*64);scenario.nextUnitId=1;
    scenario.playerSlotCount=1;scenario.civilizations.fill(1);
    scenario.players[0].active=true;scenario.players[0].human=true;scenario.players[0].civilization=1;
    for(const auto direction:std::array<std::array<float,2>,3>{{{1,0},{0,1},{-1,0}}}) {
        auto game=std::make_unique<swgb::Game>(assets);std::string error;
        check(game->initScenario(scenario,&error),error.c_str());
        const float x=20+2.1f*direction[0],y=20+2.1f*direction[1];
        const auto building=game->spawnObjectForTesting(1,109,1,20,20);
        const auto siege=game->spawnObjectForTesting(1,691,1,x,y);
        check(building && siege,"Create a siege unit overlapping a Command Center edge");
        game->update(0,{});
        check(!game->positionPassableForTesting(siege,20,20),
              "Overlap escape must not permit movement into the deeper building interior");
        check(!game->segmentClearForTesting(siege,x,y,20-2.1f*direction[0],20-2.1f*direction[1]),
              "Shallow endpoints across a building do not permit crossing its deeper interior");
        check(game->segmentClearForTesting(siege,x,y,x+3*direction[0],y+3*direction[1]),
              "An overlapping unit can escape directly outward");
        for(float side:{-1.f,1.f}) {
            check(game->segmentClearForTesting(siege,x,y,x+3*side*direction[1],y+3*side*direction[0]),
                  "Constant-penetration tangents can escape in either direction");
        }
        check(game->segmentClearForTesting(siege,22.1f,22.1f,25,25),
              "A diagonal corner overlap can escape outward");
        check(!game->segmentClearForTesting(siege,22.1f,22.1f,17.9f,17.9f),
              "Opposite shallow corners cannot cross the building interior");
        const float goalX=20-5*direction[0]+.001f*direction[1];
        const float goalY=20-5*direction[1]+.001f*direction[0];
        check(game->issuePointMoveForTesting(siege,goalX,goalY),
              "Find a route out of an overlap and around the building");
        bool arrived=false;
        for(int frame=0;frame<1800 && !arrived;++frame) {
            game->update(1.f/30,{});
            const auto position=game->objectPosition(siege);
            arrived=std::hypot(position[0]-goalX,position[1]-goalY)<.5f;
        }
        check(arrived,"An overlapping siege unit escapes and reaches the opposite side instead of sliding forever");
    }
}
// Keep large Game fixtures out of the combined sanitizer driver's stack frame.
[[gnu::noinline]] static void coupledWaypointMovementChecks(swgb::Assets &assets) {
    swgb::Scenario scenario;scenario.map.width=scenario.map.height=64;
    scenario.map.tiles.resize(64*64);scenario.nextUnitId=2461;
    scenario.playerSlotCount=1;scenario.civilizations.fill(1);
    scenario.players[0].active=true;scenario.players[0].human=true;scenario.players[0].civilization=1;
    auto game=std::make_unique<swgb::Game>(assets);std::string error;
    check(game->initScenario(scenario,&error),error.c_str());
    const auto blocker=game->spawnObjectForTesting(1,993,1,35.7451324f,40.1027718f);
    // Preserve the later odd-ID siege's yielding priority/handedness.
    game->spawnObjectForTesting(1,109,1,10,10);
    const auto siege=game->spawnObjectForTesting(1,691,1,35.3463974f,40.7489052f);
    check(blocker && siege,"Restore the two ground units from the stalled duel snapshot");
    game->setAttackModeForTesting(blocker,3);game->setAttackModeForTesting(siege,3);
    game->update(0,{});
    // Retained route state, not a newly issued formation order (which has its
    // own replanning). The captured first waypoint is occupied by both walkers.
    const std::vector<std::array<float,2>> route={{{35.5f,40.5f}},{{55.5f,54.5f}}};
    check(game->restoreMovementRouteForTesting(blocker,55.5f,54.5f,route) &&
          game->restoreMovementRouteForTesting(siege,55.5f,54.5f,route),
          "Restore retained movement routes without replacing their final destination");
    bool progressed=false,arrived=false;
    for(int frame=0;frame<3600 && !arrived;++frame) {
        game->update(1.f/30,{});
        const auto position=game->objectPosition(siege);
        if(frame==900)progressed=std::hypot(position[0]-35.3463974f,position[1]-40.7489052f)>3;
        arrived=std::hypot(position[0]-55.5f,position[1]-54.5f)<1.f;
    }
    check(progressed,"Mutual yielding makes route progress instead of indefinitely oscillating");
    check(arrived,"Dynamic recovery retains the original command and reaches its final destination");
}
[[gnu::noinline]] static void dynamicDetourGameplayChecks(swgb::Assets &assets) {
    overlappingStartMovementChecks(assets);
    coupledWaypointMovementChecks(assets);
    swgb::Scenario scenario;scenario.map.width=scenario.map.height=64;
    scenario.map.tiles.resize(64*64);scenario.nextUnitId=1;
    scenario.playerSlotCount=1;scenario.civilizations.fill(1);
    scenario.players[0].active=true;scenario.players[0].human=true;scenario.players[0].civilization=1;
    const auto &workerType=assets.dat().civs[1].units[83];
    const float radius=std::max(.1f,std::max(workerType.collisionSize[0],workerType.collisionSize[1]));
    const float separation=2*radius+.05f;
    for(const auto direction:std::array<std::array<float,2>,3>{{{1,0},{0,1},{-1,0}}}) {
        swgb::Game game(assets);std::string error;
        check(game.initScenario(scenario,&error),error.c_str());
        const float bx=20+.6f*direction[0],by=20+.6f*direction[1];
        const float gx=20+5*direction[0],gy=20+5*direction[1];
        const auto mover=game.spawnObjectForTesting(1,83,1,20,20);
        const auto blocker=game.spawnObjectForTesting(1,83,1,bx,by);
        check(mover && blocker,"Dynamic detour worker fixtures");
        game.update(0,{});
        check(game.issuePointMoveForTesting(mover,gx,gy),"Start movement toward a point beyond another unit");
        const auto route=game.localDetourForTesting(mover,gx,gy);
        check(!route.empty(),"Local avoidance finds a route around the stationary unit");
        float ax=20,ay=20;
        for(const auto &point:route) {
            const float dx=point[0]-ax,dy=point[1]-ay,lengthSquared=dx*dx+dy*dy;
            const float t=lengthSquared?std::max(0.f,std::min(1.f,((bx-ax)*dx+(by-ay)*dy)/lengthSquared)):0.f;
            const float distance=std::hypot(ax+t*dx-bx,ay+t*dy-by);
            check(distance+.0001f>=separation,"Detour smoothing and rejoin preserve the dynamic blocker clearance");
            ax=point[0];ay=point[1];
        }
        for(int i=0;i<300;++i)game.update(.05f,{});
        const auto arrived=game.objectPosition(mover);
        check(std::hypot(arrived[0]-gx,arrived[1]-gy)<.5f,"The preserved avoidance route reaches the movement goal");
    }
    {
        swgb::Game game(assets);std::string error;
        check(game.initScenario(scenario,&error),error.c_str());
        const auto mover=game.spawnObjectForTesting(1,83,1,20,20);
        const auto nearBlocker=game.spawnObjectForTesting(1,83,1,20.6f,20);
        const auto distantBlocker=game.spawnObjectForTesting(1,83,1,27,20);
        check(mover && nearBlocker && distantBlocker,"Two separate local obstructions created");
        game.update(0,{});
        check(game.issuePointMoveForTesting(mover,30,20),"Create long-range route past two mobile obstacles");
        const auto route=game.localDetourForTesting(mover,30,20);
        check(route.size()>1,"A unit beyond the four-tile search window does not discard a valid local bypass");
        float ax=20,ay=20;
        for(const auto &point:route) {
            const float dx=point[0]-ax,dy=point[1]-ay,lengthSquared=dx*dx+dy*dy;
            const float t=lengthSquared?std::max(0.f,std::min(1.f,((20.6f-ax)*dx+(20-ay)*dy)/lengthSquared)):0.f;
            check(std::hypot(ax+t*dx-20.6f,ay+t*dy-20)+.0001f>=separation,
                  "The accepted local bypass still avoids the nearby blocker");
            ax=point[0];ay=point[1];
        }
        bool arrived=false;
        for(int i=0;i<600 && !arrived;++i) {
            game.update(.05f,{});
            const auto position=game.objectPosition(mover);
            arrived=std::hypot(position[0]-30,position[1]-20)<.5f;
        }
        check(arrived,"Movement handles the later obstruction and reaches the goal beyond both units");
    }
}
