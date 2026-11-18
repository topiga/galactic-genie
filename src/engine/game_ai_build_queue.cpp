// SPDX-License-Identifier: GPL-3.0-or-later
#include "game.h"
#include "ai_builder_selection.h"
#include "../core/genie_tactical_movement.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace swgb {
bool Game::aiBuild(int player,const dat::Unit &unit) {
    if(player<=0 || size_t(player)>=aiPlayers_.size() || unit.type!=dat::UT_Building ||
       !unitAvailable(player,unit.id) || !aiCanAfford(player,unit))return false;
    auto &state=aiPlayers_[size_t(player)];
    if(state.buildRequests.pending(unit.id)>0 || state.nextBuildRequestSequence==std::numeric_limits<int>::max())return false;
    const int sequence=state.nextBuildRequestSequence;
    if(!state.buildRequests.admit(unit.id,sequence))return false;
    ++state.nextBuildRequestSequence;
    return true; // Accepted request, not completed construction or resources spent.
}

bool Game::aiBuildPositionRejected(int player,int prototype,float x,float y) const {
    if(player<=0 || size_t(player)>=aiPlayers_.size())return true;
    const std::array<int,3> key{prototype,int(float(mapSize_)-y),int(x)};
    const auto &rejected=aiPlayers_[size_t(player)].rejectedBuildPositions;
    return std::find(rejected.begin(),rejected.end(),key)!=rejected.end();
}

bool Game::chooseAiBuildPosition(int player,const dat::Unit &unit,float &x,float &y) {
    struct Restore {Game &g;int owner,failure;~Restore(){g.localPlayer_=owner;g.placementFailureCode_=failure;}} restore{*this,localPlayer_,placementFailureCode_};
    localPlayer_=player;
    if(unit.id==70) {
        const int result=chooseAiPrefabPosition(player,unit,x,y);
        if(result>=0)return result==1;
    }
    // shortcut: Non-prefab/unrepresented grid stages retain the existing native
    // radial placement policy. This selects an anchor, never the eventual builder.
    const Object *anchor=nullptr;
    for(int32_t id:aiPlayers_[size_t(player)].economy.civilianIds) {
        const auto *candidate=findObject(uint32_t(id));
        if(candidate && candidate->active && !candidate->hidden && candidate->unit &&
           isWorker(*candidate) && candidate->garrisonedInId<0) {anchor=candidate;break;}
    }
    if(!anchor)return false;
    float centerX=anchor->x,centerY=anchor->y;
    const std::string symbol=normalizeAiSymbol(unit.name2.empty()?unit.name:unit.name2);
    int resourceType=-1;
    if(symbol.find("dropcarbon")!=std::string::npos)resourceType=1;
    else if(symbol.find("dropchow")!=std::string::npos)resourceType=0;
    else if(symbol.find("dropnova")!=std::string::npos)resourceType=3;
    else if(symbol.find("dropmetal")!=std::string::npos)resourceType=2;
    const Object *resource=nullptr;float best=std::numeric_limits<float>::max();
    if(resourceType>=0)for(const auto &candidate:objects_) {
        if(!isGatherable(candidate) || candidate.resourceType!=resourceType ||
           !playerSees(player,candidate.x,candidate.y))continue;
        const float dx=candidate.x-anchor->x,dy=candidate.y-anchor->y,d=dx*dx+dy*dy;
        if(d<best){best=d;resource=&candidate;}
    }
    if(resource){centerX=resource->x;centerY=resource->y;}
    else for(const auto &building:objects_)if(building.active && !building.hidden &&
        !building.underConstruction && building.player==player && building.unit && building.unit->type==dat::UT_Building) {
        centerX=building.x;centerY=building.y;break;
    }
    static constexpr int directions[8][2]={{1,0},{1,1},{0,1},{-1,1},{-1,0},{-1,-1},{0,-1},{1,-1}};
    for(int radius=3;radius<=14;++radius)for(const auto &direction:directions) {
        float px=centerX+direction[0]*radius,py=centerY+direction[1]*radius;
        snapBuildPositionForAi(unit,px,py);
        if(aiBuildPositionRejected(player,unit.id,px,py) || !placementValid(unit,px,py))continue;
        x=px;y=py;return true;
    }
    return false;
}

uint32_t Game::chooseAiBuildWorker(int player,const dat::Unit &unit,float x,float y) {
    const auto &economy=aiPlayers_[size_t(player)].economy;
    std::vector<AiBuilderCandidate> candidates;
    for(int32_t id:economy.civilianIds) {
        const auto *worker=findObject(uint32_t(id));AiBuilderCandidate candidate;candidate.id=id;
        candidate.resolved=worker && worker->active && worker->player==player && worker->unit;
        if(candidate.resolved) {
            candidate.prototype=worker->unit->id;candidate.carriedAmount=worker->carriedAmount;
            // shortcut: Native action eligibility supplies the actor-AI callback;
            // the original +7c->+28 order code is not represented. Do not invent
            // a700-series order code from a600-series task identifier.
            candidate.hasController=!worker->hidden && worker->garrisonedInId<0 &&
                isWorker(*worker) && !worker->constructionTargetId && !worker->repairTargetId;
        }
        candidate.reservedInCurrentPass=std::find(economy.pendingCivilianAssignments.begin(),
            economy.pendingCivilianAssignments.end(),id)!=economy.pendingCivilianAssignments.end();
        candidates.push_back(candidate);
    }
    const int32_t id=aiSelectBuilder(candidates,[&](const auto &candidate) {
        const auto &worker=*findObject(uint32_t(candidate.id));
        return geniePointFootprintDistance(float(mapSize_)-worker.y,worker.x,float(int(float(mapSize_)-y)),float(int(x)),
            worker.unit->collisionSize[1],worker.unit->collisionSize[0]);
    });
    if(id<0)return 0;
    const auto *worker=findObject(uint32_t(id));
    // shortcut: Native synchronous path solving supplies the original direct
    // reach/path-service provider. It runs AFTER original worker selection.
    Object site;site.unit=&unit;site.player=player;site.x=x;site.y=y;site.active=true;
    float approachX=0,approachY=0;interactionPoint(*worker,site,.35f,approachX,approachY);
    std::vector<std::array<float,2>> path;
    return findPath(*worker,approachX,approachY,path,&site,.35f)?uint32_t(id):0;
}

void Game::updateAiBuildRequests(int player) {
    if(player<=0 || size_t(player)>=aiPlayers_.size())return;
    auto &state=aiPlayers_[size_t(player)];if(state.buildRequests.total()<=0)return;
    refreshAiEconomyKnowledge(player);
    const auto number=[&](int n,int fallback){auto i=state.strategicNumbers.find(n);return i==state.strategicNumbers.end()?fallback:i->second;};
    // shortcut: Caller admits this controller on the native AI scheduler; the
    // original tactical phase's timing is not implied by the selector itself.
    const auto selected=state.buildRequests.select(number(114,25),number(115,10),
        [&](const auto &request){return unitAvailable(player,request.prototype);},
        [&](const auto &request){const auto *u=effectiveUnitForPlayer(player,findUnit(civilizationForPlayer(player),request.prototype));return u && aiCanAfford(player,*u);},
        [](const auto &){return true;}); // Native provider: original5788e0 construction-list suitability remains separate.
    if(!selected)return;
    const auto *request=state.buildRequests.find(*selected);if(!request)return;
    const int prototype=request->prototype,sequence=request->sequence;
    const auto *unit=effectiveUnitForPlayer(player,findUnit(civilizationForPlayer(player),prototype));
    if(!unit || unit->type!=dat::UT_Building)return;
    float x=0,y=0;if(!chooseAiBuildPosition(player,*unit,x,y))return;
    const uint32_t worker=chooseAiBuildWorker(player,*unit,x,y);if(!worker)return;
    state.buildRequests.markDispatched(sequence);
    auto &reserved=state.economy.pendingCivilianAssignments;
    if(std::find(reserved.begin(),reserved.end(),int32_t(worker))==reserved.end())reserved.push_back(int32_t(worker));
    // Command102 payload is captured by value; no object pointers survive enqueue.
    localCommands_.enqueue([this,player,prototype,sequence,worker,x,y] {
        executeAiBuildCommand(player,prototype,sequence,worker,x,y);
    });
}

void Game::failAiBuildRequest(int player,int sequence,int code,float x,float y) {
    if(player<=0 || size_t(player)>=aiPlayers_.size())return;
    auto &state=aiPlayers_[size_t(player)];const auto *request=state.buildRequests.find(sequence);if(!request)return;
    const std::array<int,3> position{request->prototype,int(float(mapSize_)-y),int(x)};
    auto &rejected=state.rejectedBuildPositions;
    auto i=std::find(rejected.begin(),rejected.end(),position);
    if(code==-1) {if(i!=rejected.end())rejected.erase(i);}
    else if(i==rejected.end())rejected.push_back(position);
    state.buildRequests.failed(sequence);
    // Original worker-release/construction callbacks are distinct from this
    // request/blacklist mutation. No refund, deletion or completed count here.
}

void Game::executeAiBuildCommand(int player,int prototype,int sequence,uint32_t workerId,float x,float y) {
    if(player<=0 || size_t(player)>=aiPlayers_.size())return;
    auto &state=aiPlayers_[size_t(player)];const auto *request=state.buildRequests.find(sequence);
    if(!request || !request->inProgress || request->prototype!=prototype)return;
    auto *worker=findObject(workerId);
    const auto *unit=effectiveUnitForPlayer(player,findUnit(civilizationForPlayer(player),prototype));
    // Original command102 silently aborts if its worker/master lookup is absent;
    // this is not a fabricated cancellation acknowledgement.
    if(!worker || !worker->active || worker->player!=player || !isWorker(*worker) ||
       worker->garrisonedInId>=0 || !unit || unit->type!=dat::UT_Building)return;
    struct Restore {Game &g;int owner,failure;~Restore(){g.localPlayer_=owner;g.placementFailureCode_=failure;}} restore{*this,localPlayer_,placementFailureCode_};
    localPlayer_=player;
    if(!placementValid(*unit,x,y)) {failAiBuildRequest(player,sequence,1,x,y);return;}
    // Original5d0d00 calls5d0b70 with reservation/escrow flags bothzero.
    // World execution spends actual stock, unlike the PER planning fact.
    for(const auto &cost:unitCosts(player,*unit))if(cost.flag && cost.type>=0 && cost.amount>0 &&
        resource(player,cost.type)<cost.amount) {failAiBuildRequest(player,sequence,0,x,y);return;}
    for(const auto &cost:unitCosts(player,*unit))if(cost.flag && cost.type>=0 && cost.amount>0)
        resources_[size_t(player)][cost.type]-=cost.amount;
    // Shared factory handles original assets/foundation terrain/HP. Ordinary
    // native UI placement also uses createFoundation; do not duplicate it here.
    auto *foundation=createFoundation(*unit,player,x,y);
    if(!foundation)return; // Executed original null-factory control did not refund.
    foundation->aiBuildRequestSequence=sequence;
    rebuildAdjacency();worker=findObject(workerId);
    if(worker)assignBuilder(*worker,*foundation);
    // Pending survives foundation creation and possible assignment failure;
    // only original completion/failure notifications change request lifecycle.
}

void Game::completeAiBuildRequest(Object &building) {
    if(building.aiBuildRequestSequence<0 || building.player<=0 || size_t(building.player)>=aiPlayers_.size() || !building.unit)return;
    aiPlayers_[size_t(building.player)].buildRequests.acknowledge(
        building.aiBuildRequestSequence,building.unit->id,int(building.spawnId));
}
void Game::cancelAiBuildRequest(Object &building) {
    if(building.aiBuildRequestSequence>=0)
        failAiBuildRequest(building.player,building.aiBuildRequestSequence,6,building.x,building.y);
    // Refund belongs to the caller's explicit cancellation path. Generic death
    // must not call this until its original notification route is established.
}
} // namespace swgb
