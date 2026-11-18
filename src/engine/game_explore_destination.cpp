// SPDX-License-Identifier: GPL-3.0-or-later
#include "game.h"
#include "../core/genie_explore_spiral.h"
#include "../core/genie_explore_revisit.h"

namespace swgb {
void Game::updateAiExplorationInformation(int player) {
    if(player<=0 || size_t(player)>=aiPlayers_.size() || mapSize_<=0)return;
    auto &state=aiPlayers_[size_t(player)];
    const size_t count=size_t(mapSize_)*mapSize_;
    if(state.explorationInformation.size()!=count) {
        state.explorationInformation.assign(count,255);
        state.explorationObserved.assign(count,0);
        state.explorationQuarters={};
    }
    // Original615771 queues only the first-ever reveal (old sight byte255),
    // then585570 marks information0 and increments the quarter's first field.
    // shortcut: Native shared observation supplies these events; original shared-sight queue propagation is not yet closed.
    const uint16_t mask=sharedSightMask(player);
    for(int y=0;y<mapSize_;++y)for(int x=0;x<mapSize_;++x) {
        const size_t index=size_t(y)*mapSize_+x;
        const size_t native=size_t(mapSize_-1-x)*mapSize_+y;
        if(native>=observedTiles_.size() || !(observedTiles_[native]&mask) || state.explorationObserved[index])continue;
        state.explorationObserved[index]=1;
        state.explorationInformation[index]=0;
        ++state.explorationQuarters[size_t((4*x/mapSize_)*4+4*y/mapSize_)].explored;
    }

}

void Game::updateAiExplorationRevisit(int player) {
    auto &state=aiPlayers_[size_t(player)];
    // shortcut: RMS starts after player construction; retain unknown until its
    // full-session constructor/prepass context is available.
    if(!state.explorationRevisitTime ||
       uint32_t(uint32_t(gameTime_*1000.0)-*state.explorationRevisitTime)<=120000u)return;
    const auto candidates=std::count_if(state.explorationInformation.begin(),state.explorationInformation.end(),
        [](uint8_t cell){return cell==255 || cell==2;});
    if(!genieExploreRevisitDue(uint32_t(gameTime_*1000.0),*state.explorationRevisitTime,
            uint32_t(candidates),uint32_t(state.explorationInformation.size())))return;
    const auto mask=sharedSightMask(player);
    for(int y=0;y<mapSize_;++y)for(int x=0;x<mapSize_;++x) {
        auto &cell=state.explorationInformation[size_t(y)*mapSize_+x];
        const size_t native=size_t(mapSize_-1-x)*mapSize_+y;
        if(cell!=255 && native<visibleTiles_.size() && !(visibleTiles_[native]&mask))cell=2;
    }
}

bool Game::reissueAiExploreDestination(Object &object) {
    if(!object.active || !object.unit || object.player<=0 || size_t(object.player)>=aiPlayers_.size() || mapSize_<=0)return false;
    // Concrete705 destinations fail actor-local41c1d0 before any draw. The
    // forwarded514/task605 enters5d2d90: save rand(), dispatch, srand(saved).
    const uint32_t saved=genieRandomDraw(legacyRandomState_);
    const bool queued=[&] {
        updateAiExplorationInformation(object.player);
        auto &state=aiPlayers_[size_t(object.player)];
        std::vector<uint8_t> zones;
        if(!buildAiMilitaryRegions(object.unit->terrainRestriction,zones))return false;
        const float ox=float(mapSize_)-object.y,oy=object.x;
        if(!std::isfinite(ox) || !std::isfinite(oy) || ox<0 || oy<0 || ox>=mapSize_ || oy>=mapSize_)return false;
        const uint8_t zone=zones[size_t(int(oy))*mapSize_+int(ox)];
        GenieExploreSpiral search;search.x=ox;search.y=oy;search.class58=object.unit->cls==58;
        for(size_t i=0;i<search.quarters.size();++i)
            search.quarters[i]={int32_t(state.explorationQuarters[i].explored),int32_t(state.explorationQuarters[i].received)};
        const uint16_t mask=sharedSightMask(object.player);
        int x=-1,y=-1;
        const bool found=search.choose(mapSize_,mapSize_,legacyRandomState_,
            [&](int a,int b){return state.explorationInformation[size_t(b)*mapSize_+a];},
            [&](int a,int b){
                const size_t native=size_t(mapSize_-1-a)*mapSize_+b;
                return native<visibleTiles_.size() && (visibleTiles_[native]&mask)!=0;
            },
            // shortcut: Native footprint/collision provider stands in for actor virtual1a0.
            [&](int a,int b){return positionPassable(object,float(b),float(mapSize_-a),true);},
            [&](int a,int b){return zones[size_t(b)*mapSize_+a]==zone;},
            [&](int a,int b){
                // shortcut: Native pathfinding approximates virtual230/4a1d50;
                // the original request tolerance is masterLOS minus1.
                std::vector<std::array<float,2>> path;
                return findPath(object,float(b),float(mapSize_-a),path,nullptr,modifiedUnitAttribute(object,1,object.unit->lineOfSight)-1.0f);
            },
            [&](int a,int b,uint8_t value){state.explorationInformation[size_t(b)*mapSize_+a]=value;},
            [&] {
                // Original592300 reopens known hidden cells in this actor's zone.
                for(int b=0;b<mapSize_;++b)for(int a=0;a<mapSize_;++a) {
                    const size_t index=size_t(b)*mapSize_+a;
                    const size_t native=size_t(mapSize_-1-a)*mapSize_+b;
                    if(zones[index]==zone && state.explorationInformation[index]!=255 &&
                       !(visibleTiles_[native]&mask))state.explorationInformation[index]=2;
                }
            },x,y);
        auto &economy=state.economy;
        const int32_t id=int32_t(object.spawnId);
        if(!found) {
            // Original5a55d0 releases the civilian exploration assignment and
            // gathering record, not the general civilian pool or pending list.
            auto &explorers=economy.civilianExplorers;
            explorers.erase(std::remove(explorers.begin(),explorers.end(),id),explorers.end());
            removeAiEconomicWorker(object.player,id);
            return false; // The notification caller queues original opcode1 Stop.
        }
        //5a9ee0 reserves this dispatch, then recognizes civilians through +f8.
        const auto append=[&](std::vector<int32_t> &ids) {
            if(std::find(ids.begin(),ids.end(),id)==ids.end())ids.push_back(id);
        };
        append(economy.pendingCivilianAssignments);
        if(std::find(economy.civilianIds.begin(),economy.civilianIds.end(),id)!=economy.civilianIds.end()) {
            append(economy.civilianExplorers);
            removeAiEconomicWorker(object.player,id);
        }
        //5a9ee0 retries5ad950 before using the supplied spiral destination.
        if(queueAiExploreDestination(object))return true;
        return queueExplore(object,float(y),float(mapSize_-x));
    }();
    legacyRandomState_=saved;
    return queued;
}
} // namespace swgb
