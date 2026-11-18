// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "circular_obstacle_gameplay_checks.h"
#include "siege_exit_checks.h"
// Restored native Arid crowd: all cargo must be credited through ordinary drop-off.
[[gnu::noinline]] static void crowdedDepotDeliveryChecks(swgb::Assets &assets) {
    swgb::Scenario scenario;scenario.map.width=scenario.map.height=120;
    scenario.map.tiles.resize(120*120);scenario.playerSlotCount=2;scenario.civilizations.fill(1);
    for(int p=0;p<2;++p){scenario.players[p].active=true;scenario.players[p].human=true;scenario.players[p].civilization=1;}
    const auto add=[&](uint32_t id,uint16_t prototype,float x,float y){
        swgb::ScenarioUnit u;u.spawnId=id;u.unitId=prototype;u.player=2;u.x=120-y;u.y=x;scenario.units.push_back(u);
    };
    add(1925,109,81.0f,12.0f);
    add(1929,293,79.974945068f,17.647932053f);
    add(1931,293,79.580841064f,15.410697937f);
    add(2034,70,81.0f,3.0f);
    add(2035,68,82.0f,22.0f);
    add(2040,83,79.805297852f,16.639230728f);
    add(2041,70,79.0f,3.0f);
    add(2051,70,84.0f,3.0f);
    add(2052,83,79.502975464f,16.98756218f);
    add(2055,83,79.562927246f,17.438938141f);
    add(2057,83,79.808654785f,15.021357536f);
    add(2061,70,77.0f,3.0f);
    add(2069,562,82.0f,26.0f);
    add(2071,12,84.5f,12.5f);
    add(2072,87,84.5f,15.5f);
    add(2078,70,82.0f,1.0f);
    add(2087,70,75.0f,3.0f);
    add(2090,101,77.0f,16.0f);
    add(2091,83,79.493118286f,16.309179306f);
    add(2094,83,79.446266174f,14.749534607f);
    add(2095,323,95.0f,10.0f);
    add(2096,83,79.198631287f,15.141284943f);
    add(2097,70,73.0f,5.0f);
    add(2101,83,79.191833496f,16.652046204f);
    add(2102,104,81.5f,15.5f);
    add(2105,70,71.0f,10.0f);
    add(2106,83,79.518157959f,15.858880997f);
    add(2107,83,79.964607239f,17.19383812f);
    add(2111,70,75.0f,11.0f);
    add(2117,70,80.0f,6.0f);
    add(2119,84,77.0f,8.0f);
    add(2120,103,84.5f,9.5f);
    add(2130,82,87.0f,6.0f);
    add(2131,49,88.0f,12.0f);
    add(2132,209,73.0f,20.0f);
    add(2133,101,71.0f,2.0f);
    add(2134,317,89.0f,20.0f);
    add(2136,12,78.5f,12.5f);
    add(2138,70,67.0f,9.0f);
    add(2146,335,81.5f,8.5f);
    auto game=std::make_unique<swgb::Game>(assets);auto &g=*game;std::string error;
    check(g.initScenario(scenario,&error),error.c_str());g.update(0,{});
    check(g.setWorkerCargoForTesting(1929,2,10.0f) && g.issueDropOffForTesting(1929,1925),"Restore captured cargo and its normal drop-off order");
    check(g.restoreMovementRouteForTesting(1929,83.349998f,13.54719f,{{79.5f,16.5f},{79.5f,14.35f}},1925),"Restore the retained work approach");
    check(g.setWorkerCargoForTesting(1931,2,10.0f) && g.issueDropOffForTesting(1931,1925),"Restore captured cargo and its normal drop-off order");
    check(g.restoreMovementRouteForTesting(1931,79.580841f,14.35f,{{79.580841f,14.35f}},1925),"Restore the retained work approach");
    check(g.setWorkerCargoForTesting(2040,0,4.767059f) && g.issueDropOffForTesting(2040,1925),"Restore captured cargo and its normal drop-off order");
    check(g.restoreMovementRouteForTesting(2040,79.805298f,14.35f,{{79.805298f,14.35f}},1925),"Restore the retained work approach");
    check(g.setWorkerCargoForTesting(2052,2,10.0f) && g.issueDropOffForTesting(2052,1925),"Restore captured cargo and its normal drop-off order");
    check(g.restoreMovementRouteForTesting(2052,83.349998f,11.223755f,{{79.5f,14.35f}},1925),"Restore the retained work approach");
    check(g.setWorkerCargoForTesting(2055,2,10.0f) && g.issueDropOffForTesting(2055,1925),"Restore captured cargo and its normal drop-off order");
    check(g.restoreMovementRouteForTesting(2055,83.349998f,12.0f,{{79.5f,14.35f}},1925),"Restore the retained work approach");
    check(g.restoreMovementRouteForTesting(2057,95.720078f,24.7899f,{{79.5f,17.5f},{86.5f,22.5f},{95.720078f,24.7899f}}),"Restore the retained work approach");
    check(g.setWorkerCargoForTesting(2091,0,3.957325f) && g.issueDropOffForTesting(2091,1925),"Restore captured cargo and its normal drop-off order");
    check(g.restoreMovementRouteForTesting(2091,78.650002f,11.223753f,{{79.5f,18.5f},{75.5f,22.5f},{70.5f,22.5f},{70.5f,17.5f},{77.5f,11.5f},{78.650002f,11.223753f}},1925),"Restore the retained work approach");
    check(g.restoreMovementRouteForTesting(2094,94.720078f,25.7899f,{{79.5f,17.5f},{86.5f,22.5f},{94.720078f,25.7899f}}),"Restore the retained work approach");
    check(g.setWorkerCargoForTesting(2096,2,10.0f) && g.issueDropOffForTesting(2096,1925),"Restore captured cargo and its normal drop-off order");
    check(g.restoreMovementRouteForTesting(2096,78.650002f,13.54719f,{{79.5f,18.5f},{75.5f,22.5f},{70.5f,22.5f},{70.5f,17.5f},{74.5f,13.5f},{78.650002f,13.54719f}},1925),"Restore the retained work approach");
    check(g.setWorkerCargoForTesting(2101,2,10.0f) && g.issueDropOffForTesting(2101,1925),"Restore captured cargo and its normal drop-off order");
    check(g.restoreMovementRouteForTesting(2101,78.650002f,10.452808f,{{79.5f,18.5f},{75.5f,22.5f},{70.5f,22.5f},{70.5f,17.5f},{78.650002f,10.452808f}},1925),"Restore the retained work approach");
    check(g.setWorkerCargoForTesting(2106,2,10.0f) && g.issueDropOffForTesting(2106,1925),"Restore captured cargo and its normal drop-off order");
    check(g.restoreMovementRouteForTesting(2106,79.452812f,9.65f,{{79.5f,14.35f}},1925),"Restore the retained work approach");
    check(g.setWorkerCargoForTesting(2107,2,10.0f) && g.issueDropOffForTesting(2107,1925),"Restore captured cargo and its normal drop-off order");
    check(g.restoreMovementRouteForTesting(2107,83.349998f,12.776246f,{{79.5f,17.5f},{79.5f,14.35f}},1925),"Restore the retained work approach");
    const float foodBefore=g.resource(2,0),carbonBefore=g.resource(2,2);
    const std::array<uint32_t,12> workers={1929,1931,2040,2052,2055,2057,2091,2094,2096,2101,2106,2107};
    const std::array<uint32_t,10> loaded={1929,1931,2040,2052,2055,2091,2096,2101,2106,2107};
    bool delivered=false;
    for(int frame=0;frame<7200 && !delivered;++frame){
        std::array<std::array<float,2>,12> previous{};for(size_t i=0;i<workers.size();++i)previous[i]=g.objectPosition(workers[i]);
        g.update(1.f/30,{});
        for(size_t i=0;i<workers.size();++i){auto p=g.objectPosition(workers[i]);check(g.segmentClearForTesting(workers[i],previous[i][0],previous[i][1],p[0],p[1]),"Crowd recovery preserves static movement clearance");}
        delivered=std::all_of(loaded.begin(),loaded.end(),[&](uint32_t id){return g.workerCargoForTesting(id)<.001f;});
    }
    check(delivered,"Every captured carrier reaches the depot and deposits without task cancellation or cargo loss");
    check(std::abs(g.resource(2,0)-foodBefore-8.724384f)<.001f && std::abs(g.resource(2,2)-carbonBefore-80.f)<.001f,"Crowd recovery credits the exact food and carbon cargo");
}
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
// A timer eligibility regression only: this packed snapshot still needs the
// original cooperative path-blocker lifecycle before all cargo can be delivered.
[[gnu::noinline]] static void failedYieldRecoveryChecks(swgb::Assets &assets) {
    swgb::Scenario scenario;scenario.map.width=scenario.map.height=120;
    scenario.map.tiles.resize(120*120);scenario.playerSlotCount=2;scenario.civilizations.fill(1);
    for(int p=0;p<2;++p){scenario.players[p].active=true;scenario.players[p].human=true;scenario.players[p].civilization=1;}
    auto game=std::make_unique<swgb::Game>(assets);auto &g=*game;std::string error;
    check(g.initScenario(scenario,&error),error.c_str());
    std::map<uint32_t,uint32_t> ids;ids[1925]=g.spawnObjectForTesting(1,109,2,81.0f,12.0f);
    ids[1929]=g.spawnObjectForTesting(1,293,2,79.974945068f,17.647932053f);
    ids[1931]=g.spawnObjectForTesting(1,293,2,79.580841064f,15.410697937f);
    ids[2040]=g.spawnObjectForTesting(1,83,2,79.805297852f,16.639230728f);
    ids[2052]=g.spawnObjectForTesting(1,83,2,79.502975464f,16.98756218f);
    ids[2055]=g.spawnObjectForTesting(1,83,2,79.562927246f,17.438938141f);
    ids[2057]=g.spawnObjectForTesting(1,83,2,79.808654785f,15.021357536f);
    ids[2090]=g.spawnObjectForTesting(1,101,2,77.0f,16.0f);
    ids[2091]=g.spawnObjectForTesting(1,83,2,79.493118286f,16.309179306f);
    ids[2094]=g.spawnObjectForTesting(1,83,2,79.446266174f,14.749534607f);
    ids[2096]=g.spawnObjectForTesting(1,83,2,79.198631287f,15.141284943f);
    ids[2101]=g.spawnObjectForTesting(1,83,2,79.191833496f,16.652046204f);
    ids[2102]=g.spawnObjectForTesting(1,104,2,81.5f,15.5f);
    ids[2106]=g.spawnObjectForTesting(1,83,2,79.518157959f,15.858880997f);
    ids[2107]=g.spawnObjectForTesting(1,83,2,79.964607239f,17.19383812f);
    g.update(0,{});
    check(g.restoreMovementRouteForTesting(ids[1929],83.349998f,13.54719f,{{79.5f,16.5f},{79.5f,14.35f}}),"Restore the crowded approach waypoint");
    check(g.restoreMovementRouteForTesting(ids[1931],79.580841f,14.35f,{{79.580841f,14.35f}}),"Restore the crowded approach waypoint");
    check(g.restoreMovementRouteForTesting(ids[2040],79.805298f,14.35f,{{79.805298f,14.35f}}),"Restore the crowded approach waypoint");
    check(g.restoreMovementRouteForTesting(ids[2052],83.349998f,11.223755f,{{79.5f,14.35f}}),"Restore the crowded approach waypoint");
    check(g.restoreMovementRouteForTesting(ids[2055],83.349998f,12.0f,{{79.5f,14.35f}}),"Restore the crowded approach waypoint");
    check(g.restoreMovementRouteForTesting(ids[2057],95.720078f,24.7899f,{{79.5f,17.5f},{86.5f,22.5f},{95.720078f,24.7899f}}),"Restore the crowded approach waypoint");
    check(g.restoreMovementRouteForTesting(ids[2091],78.650002f,11.223753f,{{79.5f,18.5f},{75.5f,22.5f},{70.5f,22.5f},{70.5f,17.5f},{77.5f,11.5f},{78.650002f,11.223753f}}),"Restore the crowded approach waypoint");
    check(g.restoreMovementRouteForTesting(ids[2094],94.720078f,25.7899f,{{79.5f,17.5f},{86.5f,22.5f},{94.720078f,25.7899f}}),"Restore the crowded approach waypoint");
    check(g.restoreMovementRouteForTesting(ids[2096],78.650002f,13.54719f,{{79.5f,18.5f},{75.5f,22.5f},{70.5f,22.5f},{70.5f,17.5f},{74.5f,13.5f},{78.650002f,13.54719f}}),"Restore the crowded approach waypoint");
    check(g.restoreMovementRouteForTesting(ids[2101],78.650002f,10.452808f,{{79.5f,18.5f},{75.5f,22.5f},{70.5f,22.5f},{70.5f,17.5f},{78.650002f,10.452808f}}),"Restore the crowded approach waypoint");
    check(g.restoreMovementRouteForTesting(ids[2106],79.452812f,9.65f,{{79.5f,14.35f}}),"Restore the crowded approach waypoint");
    check(g.restoreMovementRouteForTesting(ids[2107],83.349998f,12.776246f,{{79.5f,17.5f},{79.5f,14.35f}}),"Restore the crowded approach waypoint");
    bool recoveryAttempted=false;
    for(int frame=0;frame<40;++frame) {
        g.update(1.f/30,{});
        recoveryAttempted |= g.movementProgressForTesting(ids[2091])[2]>0;
    }
    check(recoveryAttempted,"Failed mutual yielding must allow the existing detour recovery to run");
}
[[gnu::noinline]] static void dynamicDetourGameplayChecks(swgb::Assets &assets) {
    circularObstacleDetourChecks(assets);
    siegeExitChecks(assets);
    failedYieldRecoveryChecks(assets);
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
