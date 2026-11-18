// SPDX-License-Identifier: GPL-3.0-or-later
#include "game.h"
#include "ai_economy.h"
#include "ai_gather_allocation.h"
#include "ai_gather_quota.h"
#include "ai_group_search.h"
#include "ai_gather_controller.h"
#include "ai_resource_lifecycle.h"
#include <chrono>
#include <algorithm>
#include <cmath>
#include <map>
namespace swgb {
namespace {
bool includesId(const std::vector<int32_t> &ids,int32_t id) {
    return std::find(ids.begin(),ids.end(),id)!=ids.end();
}
// Original588340 admission differs from selector586ec0: class3 is admitted, class5 is not.
int originalResourceBucket(int cls) {
    switch(cls) {
    case 1:case 3:case 4:case 7:case 23:case 24:case 25:case 27:case 60:return 0;
    case 31:return 1;case 30:return 2;case 29:return 3;default:return -1;
    }
}
}
bool Game::aiEconomicWorkerIdle(const Object &o) const {
    // shortcut: Native action slots supply the original controller/task/group callbacks.
    const int task=o.tacticalMovement.present || o.pointMovement.present?610:
        o.exploration?605:o.attackTargetId?600:o.constructionTargetId?602:
        o.repairTargetId?618:o.gatherTargetId || o.dropOffTargetId?609:
        o.state==State::Idle?-1:0;
    const int action=o.tacticalMovement.present?o.tacticalMovement.state:
        o.pointMovement.present?(o.pointMovement.status==GenieMovementStatus::Active?4:1):2;
    return aiGroupSearchIdle(action,task,-1,o.marchGroupId!=0,false);
}
void Game::removeAiEconomicWorker(int player,int32_t id) {
    auto &economy=aiPlayers_[size_t(player)].economy;
    for(auto &worker:economy.economicWorkers)if(worker.id==id)worker=AiEconomicWorker{};
}
void Game::refreshAiEconomyKnowledge(int player) {
    auto &state=aiPlayers_[size_t(player)];auto &economy=state.economy;
    const auto aliveOwned=[&](int32_t id) {const auto *o=findObject(uint32_t(id));
        return o && o->active && o->player==player && o->hitPoints>0;};
    const auto removeDead=[&](auto &ids) {ids.erase(std::remove_if(ids.begin(),ids.end(),
        [&](int32_t id){return !aliveOwned(id);}),ids.end());};
    removeDead(economy.civilianIds);removeDead(economy.civilianExplorers);
    removeDead(economy.pendingCivilianAssignments);
    for(auto &record:economy.economicWorkers)if(record.id>=0 && !aliveOwned(record.id))record=AiEconomicWorker{};
    std::map<int,std::vector<uint8_t>> regions;
    const auto zone=[&](int restriction,float x,float y)->std::optional<uint8_t> {
        auto i=regions.find(restriction);
        if(i==regions.end()) {std::vector<uint8_t> tiles;if(!buildAiMilitaryRegions(restriction,tiles))return std::nullopt;
            i=regions.emplace(restriction,std::move(tiles)).first;}
        const int ox=int(float(mapSize_)-y),oy=int(x);
        if(ox<0 || oy<0 || ox>=mapSize_ || oy>=mapSize_)return std::nullopt;
        return i->second[size_t(oy)*mapSize_+ox];
    };
    const auto metric=[&](const Object &o)->std::optional<AiResourceDepotMetric> {
        std::vector<AiResourceDepot> depots;
        const auto ownZone=zone(o.unit->terrainRestriction,o.x,o.y);
        if(o.unit->cls!=25 && !ownZone)return std::nullopt;
        for(int32_t id:economy.depotIds) {
            const auto *depot=findObject(uint32_t(id));if(!depot || !depot->unit)continue;
            const auto at=zone(o.unit->terrainRestriction,depot->x,depot->y);
            if(o.unit->cls!=25 && !at)return std::nullopt;
            depots.push_back({id,depot->unit->id,float(mapSize_)-depot->y,depot->x,true,
                ownZone && at && *ownZone==*at});
        }
        return aiResourceDepotMetric(o.unit->cls,float(mapSize_)-o.y,o.x,true,depots);
    };
    const auto refresh=[&](int bucket) {
        // Preflight unsupported native region mappings, separately from original missing IDs.
        std::unordered_map<int32_t,AiResourceDepotMetric> values;
        for(const auto &record:economy.resources[size_t(bucket)].records)if(record.invalid!=1) {
            const auto *object=findObject(uint32_t(record.id));
            if(!object || !object->active || !object->unit)continue;
            const auto value=metric(*object);if(!value)return false;
            values.emplace(record.id,*value);
        }
        refreshAiResourceBucket(economy.resources[size_t(bucket)],[&](int32_t id)->std::optional<AiResourceDepotMetric> {
            auto i=values.find(id);return i==values.end()?std::nullopt:std::optional<AiResourceDepotMetric>(i->second);
        });
        return true;
    };
    // shortcut: Poll native registry changes at admitted economy ticks instead of
    // original MainAI5960d0/InfoAI58a660 notifications. Mutation precedes refresh.
    for(size_t i=0;i<economy.depotIds.size();) {
        const int32_t id=economy.depotIds[i];const auto *object=findObject(uint32_t(id));
        const auto prior=economy.depotPrototypes.find(id);
        if(aliveOwned(id) && object && object->unit && prior!=economy.depotPrototypes.end() &&
           prior->second==std::make_pair(int(object->unit->id),int(object->unit->cls))) {++i;continue;}
        const auto old=prior==economy.depotPrototypes.end()?std::make_pair(-1,-1):prior->second;
        economy.depotIds.erase(economy.depotIds.begin()+i);economy.depotPrototypes.erase(id);
        for(int bucket:aiDepotRefreshBuckets(old.first,old.second,true))
            economy.pendingDepotRefresh[size_t(bucket)]=!refresh(bucket);
    }
    for(const auto &o:objects_)if(o.active && o.player==player && o.unit && o.hitPoints>0) {
        if(o.unit->cls==58 && !includesId(economy.civilianIds,int32_t(o.spawnId)))economy.civilianIds.push_back(int32_t(o.spawnId));
        const int cls=o.unit->cls;
        if(((cls>=6 && cls<=10) || cls==18) && !includesId(economy.depotIds,int32_t(o.spawnId))) {
            economy.depotIds.push_back(int32_t(o.spawnId));economy.depotPrototypes[int32_t(o.spawnId)]={o.unit->id,cls};
            for(int bucket:aiDepotRefreshBuckets(o.unit->id,cls,false))
                economy.pendingDepotRefresh[size_t(bucket)]=!refresh(bucket);
        }
    }
    for(size_t bucket=0;bucket<4;++bucket)if(economy.pendingDepotRefresh[bucket])
        economy.pendingDepotRefresh[bucket]=!refresh(int(bucket));
    std::unordered_set<uint32_t> currentlyVisible;
    for(const auto &o:objects_) {
        if(!o.active || !o.unit || visibleTargets_.slot(size_t(player-1),o.spawnId)==size_t(-1))continue;
        const int bucket=originalResourceBucket(o.unit->cls);if(bucket<0)continue;
        currentlyVisible.insert(o.spawnId);
        auto known=economy.resourceSlots.find(o.spawnId);
        if(known!=economy.resourceSlots.end() && economy.visibleResources.count(o.spawnId))continue;
        const auto value=metric(o);if(!value) {currentlyVisible.erase(o.spawnId);continue;}
        auto &resources=economy.resources[size_t(bucket)];
        if(known==economy.resourceSlots.end()) {
            economy.resourceSlots.emplace(o.spawnId,std::make_pair(bucket,resources.records.size()));
            resources.records.push_back(makeAiResourceRecord(int32_t(o.spawnId),float(mapSize_)-o.y,o.x,uint8_t(bucket),*value));
            observeAiResourceMetric(resources,resources.records.back(),*value);
        } else {
            // shortcut: Re-entry into native sight supplies the original duplicate
            // observation callback. It never rewrites original remembered byteXY.
            auto &oldBucket=economy.resources[size_t(known->second.first)];
            observeAiResourceMetric(oldBucket,oldBucket.records[known->second.second],*value);
        }
    }
    economy.visibleResources=std::move(currentlyVisible);
    // shortcut: This original InfoAI timer is sampled at native economy cadence.
    const uint32_t now=uint32_t(gameTime_*1000.0);
    if(economy.foodRefreshTime && aiFoodMetricRefreshDue(now,*economy.foodRefreshTime) && refresh(0))
        economy.foodRefreshTime=now;
}
void Game::updateAiGatherers(int player,AiPlayerState &state) {
    if(!state.scripted) {updateAiGatherersFallback(player,state);return;}
    // Verified profile defaults, overridden by PER setters. Other strategic numbers
    // are never silently synthesized by this bounded controller.
    const auto sn=[&](int n,int initial) {auto i=state.strategicNumbers.find(n);return i==state.strategicNumbers.end()?initial:i->second;};
    const std::array<int32_t,4> percentages{sn(117,0),sn(120,0),sn(119,0),sn(118,0)};
    const std::array<int32_t,4> adjustments{sn(156,0),sn(157,0),sn(158,0),sn(159,0)};
    if(std::none_of(percentages.begin(),percentages.end(),[](int n){return n>0;}))return;
    refreshAiEconomyKnowledge(player);auto &economy=state.economy;
    int militaryExplorers=0;for(const auto &group:state.militaryGroups)if(group.type==101)militaryExplorers+=int(group.members.size());
    size_t explored=0;const auto sight=sharedSightMask(player);
    for(uint16_t tile:observedTiles_)if(tile&sight)++explored;
    int firstEnemy=-1;for(size_t other=1;other<aiPlayers_.size();++other)if(int(other)!=player && other<players_[size_t(player-1)].diplomacy.size() && players_[size_t(player-1)].diplomacy[other]==3) {firstEnemy=int(other);break;}
    int knownEnemies=0;for(const auto &known:state.militaryKnownObjects) {
        const auto *o=findObject(uint32_t(known.id));if(o && o->player==firstEnemy)++knownEnemies;
    }
    const auto quota=aiGatherQuota({int(economy.civilianIds.size()),militaryExplorers,
        observedTiles_.empty()?0:int(100*explored/observedTiles_.size()),knownEnemies,
        sn(0,34),sn(18,4),sn(3,2),sn(5,-1),sn(32,100),sn(145,10),sn(35,0),sn(179,30),false});
    if(!quota)return;
    economy.explorerQuota=quota->explorers;economy.gathererQuota=quota->gatherers;
    if(gameTime_*1000.0<=5000.0)return;
    // shortcut: Native admitted economy ticks replace original phase2/3 budget scheduling.
    // No native force-gatherer override is assumed for original tactical+1648.
    // shortcut: Pending native commands are consumed between these admitted ticks;
    // Original59ddf2 clears after the shared tactical pass; this controller still
    // has an independent native cadence, so its previous-pass reservations end here.
    economy.pendingCivilianAssignments.clear();
    const auto registered=[&](int32_t id) {return std::any_of(economy.economicWorkers.begin(),economy.economicWorkers.end(),[&](const auto &w){return w.id==id;});};
    const auto available=[&]()->Object * {
        for(int32_t id:economy.civilianIds) {
            if(registered(id) || includesId(economy.civilianExplorers,id) || includesId(economy.pendingCivilianAssignments,id))continue;
            auto *o=findObject(uint32_t(id));if(o && !o->hidden && o->garrisonedInId<0 && aiEconomicWorkerIdle(*o))return o;
        }return nullptr;
    };
    while(int(economy.civilianExplorers.size())<quota->explorers) {
        auto *worker=available();if(!worker)break;
        economy.civilianExplorers.push_back(int32_t(worker->spawnId));
    }
    while(!economy.civilianExplorers.empty() && int(economy.civilianExplorers.size())>quota->explorers) {
        const int32_t id=economy.civilianExplorers.front();economy.civilianExplorers.erase(economy.civilianExplorers.begin());queueAiMilitaryStop(player,id);
    }
    int count=int(std::count_if(economy.economicWorkers.begin(),economy.economicWorkers.end(),[](const auto &w){return w.id>=0;}));
    while(count<quota->gatherers) {
        auto slot=std::find_if(economy.economicWorkers.begin(),economy.economicWorkers.end(),[](const auto &w){return w.id<0;});
        auto *worker=available();if(slot==economy.economicWorkers.end() || !worker)break;
        *slot=AiEconomicWorker{};slot->id=int32_t(worker->spawnId);++count;
    }
    AiGatherAllocation allocation;
    const auto allocate=[&] {
        economy.assigned={};int valid=0;
        for(const auto &record:economy.economicWorkers) {
            if(record.resource<0 || record.resource>=4)continue;
            const auto *worker=findObject(uint32_t(record.id));if(!worker)continue;++valid;
            if(worker->gatherTargetId || worker->dropOffTargetId || includesId(economy.pendingCivilianAssignments,record.id))++economy.assigned[size_t(record.resource)];
        }
        allocation=allocateAiGatherers(valid,quota->gatherers,percentages,adjustments);
        economy.desired=allocation.desired;
    };
    allocate();
    for(int32_t id:economy.civilianIds) {
        auto *worker=findObject(uint32_t(id));if(!worker || worker->hidden || worker->garrisonedInId>=0)continue;
        if(includesId(economy.civilianExplorers,id)) {if(aiEconomicWorkerIdle(*worker))queueAiCivilianExplore(*worker);continue;}
        auto record=std::find_if(economy.economicWorkers.begin(),economy.economicWorkers.end(),[&](const auto &w){return w.id==id;});
        if(record==economy.economicWorkers.end())continue;
        // Native action slots map to original5a8670 inputs; batch and retry policy
        // itself is shared with the original-derived controller, not duplicated here.
        const int task=worker->constructionTargetId?602:worker->repairTargetId?618:
            worker->attackTargetId?600:worker->exploration?605:
            worker->gatherTargetId || worker->dropOffTargetId?609:
            worker->tacticalMovement.present || worker->pointMovement.present?610:
            worker->state==State::Idle?-1:0;
        const int command=worker->tacticalMovement.present || worker->pointMovement.present?710:-1;
        const int priority=worker->tacticalMovement.present?worker->tacticalMovement.priority:
            worker->pointMovement.present?100:0;
        const uint32_t hostTime=uint32_t(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count());
        const AiGatherControllerInput input{true,false,true,task,command,priority,
            worker->carriedAmount,hostTime,economy.assigned,economy.desired};
        const auto assignBucket=[&](int bucket,bool force,int retries,int failedCooldown)->bool {
            std::vector<int32_t> excluded;std::vector<uint8_t> zones;
            if(!buildAiMilitaryRegions(worker->unit->terrainRestriction,zones))return false;
            const auto zone=[&](int x,int y)->std::optional<uint8_t> {
                if(x<0 || y<0 || x>=mapSize_ || y>=mapSize_)return std::nullopt;
                return zones[size_t(y)*mapSize_+x];
            };
            const auto ownZone=zone(int(float(mapSize_)-worker->y),int(worker->x));
            for(int retry=0;retry<retries;++retry) {
                const auto selected=selectAiLandResource(economy.resources[size_t(bucket)],bucket,player,
                    float(mapSize_)-worker->y,worker->x,uint32_t(gameTime_*1000.0),excluded,
                    [&](int32_t targetId)->std::optional<AiResourceTarget> {
                        const auto *o=findObject(uint32_t(targetId));if(!o || !o->active || !o->unit)return std::nullopt;
                        //5ae220/5af970 counts nearby food/unassigned workers available for a hunt.
                        int hunters=0;
                        if(o->unit->cls==60 && o->hitPoints>0)for(const auto &candidate:economy.economicWorkers) {
                            if(candidate.id<0 || (candidate.resource!=0 && candidate.resource!=-1) ||
                               includesId(economy.pendingCivilianAssignments,candidate.id))continue;
                            const auto *hunter=findObject(uint32_t(candidate.id));if(!hunter || !hunter->unit)continue;
                            const uint32_t huntedId=hunter->attackTargetId?hunter->attackTargetId:hunter->gatherTargetId;
                            const auto *hunted=findObject(huntedId);
                            if(hunted && hunted->unit && hunted->unit->cls==60 && hunted->hitPoints>0)continue;
                            // shortcut: Native collision boxes supply551b80's original master fields.
                            const float dx=float(std::max(0.,std::abs(double(hunter->y)-o->y)-hunter->unit->collisionSize[1]-o->unit->collisionSize[1]));
                            const double dy=std::max(0.,std::abs(double(hunter->x)-o->x)-hunter->unit->collisionSize[0]-o->unit->collisionSize[0]);
                            if(std::sqrt(double(dx)*dx+dy*dy)<10.)++hunters;
                        }
                        // shortcut: Native state/task references supply original live lookup fields;
                        // the specialized farm-blocking registry remains unrepresented.
                        return AiResourceTarget{o->unit->id,o->unit->cls,o->player,float(mapSize_)-o->y,o->x,o->resourceAmount,
                            uint8_t(o->hitPoints<=0?3:o->constructionRemaining>0?0:2),o->taskReferences,
                            uint8_t(playerSees(player,o->x,o->y)?15:0),o->garrisonedInId>=0,false,hunters};
                    },[&](int n)->std::optional<int> {
                        if(n==204)return sn(n,5);
                        if(n==233 || n==234 || n==235 || n==237 || n==238)return sn(n,-1);
                        return std::nullopt;
                    },[&](uint8_t x,uint8_t y)->std::optional<bool> {const auto at=zone(x,y);if(!ownZone || !at)return std::nullopt;return *ownZone==*at;});
                if(selected.status!=AiResourceSelectionStatus::Selected)return false;
                auto *target=findObject(uint32_t(selected.id));std::vector<std::array<float,2>> path;
                // Native pathfinding/action construction remain explicit adapters to original5a94f0.
                if(!target || !findPath(*worker,target->x,target->y,path,target) || !issueGatherCommand(*worker,*target)) {
                    excluded.push_back(selected.id);
                    auto slot=economy.resourceSlots.find(uint32_t(selected.id));
                    if(slot!=economy.resourceSlots.end())economy.resources[size_t(slot->second.first)].records[slot->second.second].cooldown=uint8_t(bucket==0?2:failedCooldown);
                    continue;
                }
                record->resource=bucket;
                record->batch=force?10:std::min(sn(148,20),sn(149,40));
                if(!force && sn(203,0)==1)record->batch=target->unit->id==50?50:
                    target->unit->cls==27 || bucket==0?int(target->resourceAmount):bucket==1?75:50;
                record->completed=0;
                if(!includesId(economy.pendingCivilianAssignments,id))economy.pendingCivilianAssignments.push_back(id);
                return true;
            }
            return false;
        };
        updateAiGatherController(*record,input,[&](int bucket,bool force,const std::array<int32_t,4> *tried) {
            //5a954a forces the previous resource while its credited batch is incomplete.
            if(record->batch>record->completed)bucket=record->resource;
            if(bucket>=0 && bucket<4)return assignBucket(bucket,force,5,15);
            //5a95a6's fallback visits allocation-order buckets, one candidate each;
            // it does not use the explicit-bucket five-attempt policy.
            for(int next:allocation.order) {
                if(tried && (*tried)[size_t(next)]==1)continue;
                if(assignBucket(next,force,1,5))return true;
            }
            return false;
        });
        allocate();
    }
}
} // namespace swgb
