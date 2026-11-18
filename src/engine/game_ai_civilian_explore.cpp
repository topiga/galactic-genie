// SPDX-License-Identifier: GPL-3.0-or-later
#include "game.h"
#include "../core/genie_explore_search.h"
#include <algorithm>
#include <cmath>

namespace swgb {
bool Game::queueAiCivilianExplore(Object &worker) {
    if(!worker.active || !worker.unit || worker.player<=0 || size_t(worker.player)>=aiPlayers_.size())return false;
    auto &economy=aiPlayers_[size_t(worker.player)].economy;
    const int32_t id=int32_t(worker.spawnId);
    const auto append=[&](std::vector<int32_t> &list) {
        if(std::find(list.begin(),list.end(),id)==list.end())list.push_back(id);
    };
    // Original5a9c90/5a8310 dispatch: reserve the assignment even when the
    // tactical waypoint search fails. That failure emits negative705 and lets
    // actor41c1d0 find a local unexplored destination; it is not a Stop.
    append(economy.pendingCivilianAssignments);
    append(economy.civilianExplorers);
    removeAiEconomicWorker(worker.player,id);
    if(queueAiExploreDestination(worker))return true;
    return queueExplore(worker,-1.f,-1.f);
}

bool Game::chooseLocalExploreDestination(Object &object,float commandX,float commandY) {
    if(!object.exploration || !object.unit || mapSize_<=0)return false;
    GenieExploreSearch search;
    search.command=705;
    search.x=float(mapSize_)-object.y;search.y=object.x;
    search.destinationX=commandY==-1.f?-1.f:float(mapSize_)-commandY;
    search.destinationY=commandX==-1.f?-1.f:commandX;
    // Known-coordinate705 fails before the original actor-local RNG draw.
    if(search.destinationX!=-1.f && search.destinationY!=-1.f)return false;
    search.lineOfSight=modifiedUnitAttribute(object,1,object.unit->lineOfSight);
    std::vector<uint8_t> zones;
    if(!buildAiMilitaryRegions(object.unit->terrainRestriction,zones))return false;
    if(search.x<0 || search.y<0 || search.x>=mapSize_ || search.y>=mapSize_)return false;
    const uint8_t zone=zones[size_t(int(search.y))*mapSize_+int(search.x)];
    const uint16_t mask=sharedSightMask(object.player);
    int x=-1,y=-1;
    const bool found=search.choose(mapSize_,mapSize_,legacyRandomState_,&object.explorationHistory,
        [&](int a,int b) {
            const size_t index=size_t(b)*mapSize_+a;
            const size_t native=size_t(mapSize_-1-a)*mapSize_+b;
            // Original615430 rejects both currently seen and remembered tiles.
            if((visibleTiles_[native]&mask) || (observedTiles_[native]&mask) || zones[index]!=zone)return false;
            // shortcut: Native collision and path providers stand in for actor virtual1a0/230.
            return positionPassable(object,float(b),float(mapSize_-a),true);
        },
        [&](int a,int b) {
            std::vector<std::array<float,2>> path;
            return findPath(object,float(b),float(mapSize_-a),path,nullptr,search.lineOfSight-1.f);
        },x,y);
    if(!found)return false;
    // Original420460 records the selected integer point in the eight-entry
    // history, then invokes41cfd0 which retargets the action and stores the
    // concrete actor-AI destination. The next514 therefore bypasses local search.
    object.explorationHistory.append(x,y);
    const float nx=float(y),ny=float(mapSize_-x);
    object.exploration->retarget(true,nx,ny,1.f);
    object.explorationChild.begin(nx,ny);
    object.moveAnchorX=nx;object.moveAnchorY=ny;object.moveGoalActive=true;
    if(!issueMove(object,nx,ny))object.moveGoalActive=false;
    return true;
}
} // namespace swgb
