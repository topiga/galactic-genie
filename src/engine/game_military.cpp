// SPDX-License-Identifier: GPL-3.0-or-later
#include "game.h"
#include "ai_military.h"
#include "ai_military_admission.h"
#include "rms_zones.h"
#include "../core/genie_tactical_movement.h"
#include "../core/genie_random.h"
#include "../core/genie_explore_records.h"
#include <chrono>
#include <limits>

namespace swgb {
void Game::refreshAiMilitaryMembers(int player) {
    auto &state=aiPlayers_[size_t(player)];
    // shortcut: Native active-object lifecycle supplies admission/removal, not original controller notifications.
    state.militaryHomes.erase(std::remove_if(state.militaryHomes.begin(),state.militaryHomes.end(),[&](int32_t id) {
        const auto *object=findObject(uint32_t(id));
        return !object || !object->active || object->player!=player || object->unit->id!=109;
    }),state.militaryHomes.end());
    for(auto it=state.militarySoldiers.begin();it!=state.militarySoldiers.end();) {
        const auto *object=findObject(uint32_t(*it));
        if(!object || !object->active || object->player!=player || !object->unit ||
           !aiMilitarySoldierClass(object->unit->cls)) {
            removeAiMilitaryMember(state.militaryGroups,*it);
            it=state.militarySoldiers.erase(it);
        } else ++it;
    }
    for(const auto &object:objects_) {
        if(object.active && object.player==player && object.unit && object.unit->id==109 && object.spawnId<=uint32_t(INT32_MAX) &&
           std::find(state.militaryHomes.begin(),state.militaryHomes.end(),int32_t(object.spawnId))==state.militaryHomes.end())
            state.militaryHomes.push_back(int32_t(object.spawnId));
        if(!object.active || object.player!=player || !object.unit ||
           !aiMilitarySoldierClass(object.unit->cls) || object.spawnId>uint32_t(INT32_MAX))continue;
        const auto id=int32_t(object.spawnId);
        if(std::find(state.militarySoldiers.begin(),state.militarySoldiers.end(),id)==state.militarySoldiers.end())
            state.militarySoldiers.push_back(id);
    }
}

void Game::queueAiMilitaryStop(int player,int32_t id) {
    const auto *object=findObject(uint32_t(id));
    if(!object || !object->active || object->player!=player)return;
    localCommands_.enqueue([this,player,id] {
        auto *actor=findObject(uint32_t(id));
        if(actor && actor->active && actor->player==player && !actor->hidden && actor->garrisonedInId<0)
            stopUnit(*actor);
    });
}

bool Game::buildAiMilitaryRegions(int restriction,std::vector<uint8_t> &tiles) const {
    const auto &restrictions=assets_.dat().terrainRestrictions;
    if(mapSize_<=0 || restriction<0 || size_t(restriction)>=restrictions.size())return false;
    std::vector<uint8_t> terrain(size_t(mapSize_)*mapSize_);
    for(int y=0;y<mapSize_;++y)for(int x=0;x<mapSize_;++x) {
        const int value=terrainAt(y,mapSize_-1-x);
        if(value<0 || value>255)return false;
        terrain[size_t(y)*mapSize_+x]=uint8_t(value);
    }
    // shortcut: Original-coordinate raster is shared; original per-object zone-cache lifetime is not yet represented.
    RmsZones result;
    if(!generateRmsZones(uint32_t(mapSize_),terrain,restrictions[size_t(restriction)].passableBuildableDmgMultiplier,result))return false;
    tiles=std::move(result.tiles);return true;
}

std::optional<AiMilitaryGroups> Game::availableAiMilitaryGroups(int player,std::optional<int> requested,bool exploring) {
    if(player<=0 || size_t(player)>=aiPlayers_.size() || mapSize_<=0)return std::nullopt;
    auto &state=aiPlayers_[size_t(player)];
    if(!state.strategicNumbers.count(230) || (!requested && !state.strategicNumbers.count(227)))return std::nullopt;
    std::vector<const Object*> available;
    for(int32_t id:state.militarySoldiers) {
        const auto *object=findObject(uint32_t(id));
        if(!object || object->unit->cls==51)continue;
        // shortcut: Original class50 gate reads game+9c2; its native option mapping is unresolved.
        if(std::any_of(state.militaryGroups.begin(),state.militaryGroups.end(),[&](const auto &group) {
            return std::find(group.members.begin(),group.members.end(),id)!=group.members.end();
        }))continue;
        // Native dispatch cannot command contained/hidden actors. The original pool itself has no such filter.
        if(object->hidden || object->garrisonedInId>=0)continue;
        available.push_back(object);
    }
    if(available.empty())return AiMilitaryGroups{};
    std::map<int,std::vector<uint8_t>> zones;
    const auto region=[&](const Object &center,const Object &sample)->std::optional<uint8_t> {
        const int restriction=center.unit->terrainRestriction;
        if(!std::isfinite(sample.x) ||
           !std::isfinite(sample.y) || sample.x<0 || sample.y<0 || sample.x>=mapSize_ || sample.y>=mapSize_)return std::nullopt;
        auto it=zones.find(restriction);
        if(it==zones.end()) {
            std::vector<uint8_t> tiles;
            if(!buildAiMilitaryRegions(restriction,tiles))return std::nullopt;
            it=zones.emplace(restriction,std::move(tiles)).first;
        }
        const int originalX=int(float(mapSize_)-sample.y),originalY=int(sample.x);
        if(originalX<0 || originalX>=mapSize_ || originalY<0 || originalY>=mapSize_)return std::nullopt;
        return it->second[size_t(originalY)*mapSize_+originalX];
    };
    std::vector<AiMilitaryCandidate> candidates;
    for(const auto *object:available) {
        const auto own=region(*object,*object);if(!own)return std::nullopt;
        bool eligible=true;
        if(exploring) {
            double speed=modifiedUnitAttribute(*object,5,object->unit->speed);
            const auto *graphic=assets_.dat().graphic(objectGraphic(*object,*object->unit));
            if(graphic && graphic->speedMultiplier>0)speed*=graphic->speedMultiplier;
            // shortcut: Native production has no original action104 hold-refcount; defer these candidates.
            eligible=object->productionQueue.empty() && aiExploreCandidateEligible(object->unit->heroMode,speed);
        }
        candidates.push_back({int(object->spawnId),float(mapSize_)-object->y,object->x,*own,eligible});
    }
    const size_t n=std::min<size_t>(available.size(),40);
    std::vector<uint8_t> projected(n*n);
    for(size_t i=0;i<n;++i)for(size_t j=0;j<n;++j) {
        const auto value=region(*available[i],*available[j]);if(!value)return std::nullopt;
        projected[i*n+j]=*value;
    }
    return selectAiMilitaryGroups(candidates,projected,
        requested?*requested:aiMilitaryAttackCount(candidates.size(),state.strategicNumbers.at(227)),
        float(state.strategicNumbers.at(230)));
}

void Game::reconcileAiMilitaryGroups(int player) {
    auto &state=aiPlayers_[size_t(player)];
    for(int key:{16,26,36,42,43,98})if(!state.strategicNumbers.count(key))return;
    refreshAiMilitaryMembers(player);
    if(state.militarySoldiers.size()>size_t(INT32_MAX))return;
    std::vector<AiMilitaryAdmissionGroup> existing;
    for(const auto &group:state.militaryGroups)
        existing.push_back({group.id,group.type,group.state,int(group.members.size()),group.flags});
    const AiMilitaryAdmissionSettings settings{int(state.militarySoldiers.size()),
        state.strategicNumbers.at(43),state.strategicNumbers.at(42),
        state.strategicNumbers.at(16),state.strategicNumbers.at(26),
        state.strategicNumbers.at(36),state.strategicNumbers.at(98)};
    // shortcut: Ordinary100/101 admission shares native refresh cadence;
    // original phase0 and the separate special112 source registry remain unported.
    const auto plan=planAiMilitaryAdmission(std::move(existing),settings,state.nextMilitaryGroup,legacyRandomState_);
    if(!plan)return;
    for(const auto &operation:plan->operations) {
        if(operation.create) {
            AiMilitaryGatherGroup group{operation.id,1,operation.desired,{}};
            group.type=operation.type;
            state.militaryGroups.push_back(std::move(group));
        } else {
            // Original5acee0 releases the registry entry without stopping members.
            state.militaryGroups.erase(std::remove_if(state.militaryGroups.begin(),state.militaryGroups.end(),
                [&](const auto &group){return group.id==operation.id;}),state.militaryGroups.end());
        }
    }
    state.nextMilitaryGroup=plan->nextId;legacyRandomState_=plan->randomState;
}

bool Game::queueAiExploreDestination(Object &object) {
    auto &state=aiPlayers_[size_t(object.player)];
    const Object *home=nullptr;
    for(int32_t id:state.militaryHomes)if((home=findObject(uint32_t(id))))break;
    if(mapSize_<=0)return false;
    std::vector<uint8_t> zones;
    if(!buildAiMilitaryRegions(object.unit->terrainRestriction,zones))return false;
    const int ox=int(float(mapSize_)-object.y),oy=int(object.x);
    if(ox<0 || oy<0 || ox>=mapSize_ || oy>=mapSize_)return false;
    const uint8_t zone=zones[size_t(oy)*mapSize_+ox];
    updateAiExplorationInformation(object.player);
    auto &information=state.explorationInformation;
    const int hx=home?int(float(mapSize_)-home->y):0,hy=home?int(home->x):0;
    constexpr int offsets[8][2]={{-1,-1},{0,-1},{1,-1},{1,0},{1,1},{0,1},{-1,1},{-1,0}};
    float best=-1;int bx=-1,by=-1;
    if(home)for(int radius:{10,15,20})for(const auto &offset:offsets) {
        const int x=hx+radius*offset[0],y=hy+radius*offset[1];
        if(x<0 || y<0 || x>=mapSize_ || y>=mapSize_)continue;
        const size_t index=size_t(y)*mapSize_+x;
        if(information[index]!=255 || zones[index]!=zone)continue;
        const double distance=geniePointFootprintDistance(float(mapSize_)-object.y,object.x,float(x),float(y),
            object.unit->collisionSize[1],object.unit->collisionSize[0]);
        if(best<0 || distance<best) {best=float(distance);bx=x;by=y;}
    }
    if(bx<0) {
        const auto destination=genieExploreRememberedDestination(int(object.spawnId),object.player,mapSize_,mapSize_,
            state.militaryKnownObjects,
            [&](int x,int y){return information[size_t(y)*mapSize_+x];},
            [&](int x,int y){return x<mapSize_ && y<mapSize_ && zones[size_t(y)*mapSize_+x]==zone;},
            [&](int x,int y,int){return geniePointFootprintDistance(float(mapSize_)-object.y,object.x,float(x),float(y),
                object.unit->collisionSize[1],object.unit->collisionSize[0]);},
            [&](int x,int y){
                // shortcut: Native pathfinding supplies virtual230; only the selected record is tested.
                std::vector<std::array<float,2>> path;
                return findPath(object,float(y),float(mapSize_-x),path,nullptr,
                    modifiedUnitAttribute(object,1,object.unit->lineOfSight)-1.0f);
            },
            [&](int x,int y,uint8_t value){if(x<mapSize_ && y<mapSize_)information[size_t(y)*mapSize_+x]=value;});
        if(!destination)return false;
        bx=(*destination)[0];by=(*destination)[1];
    }
    information[size_t(by)*mapSize_+bx]=0;
    return queueExplore(object,float(by),float(mapSize_-bx));
}

bool Game::formAiMilitaryGroups(int player) {
    if(player<=0 || size_t(player)>=aiPlayers_.size() || mapSize_<=0)return false;
    auto &state=aiPlayers_[size_t(player)];refreshAiMilitaryMembers(player);
    for(int key:{34,41,227,230})if(!state.strategicNumbers.count(key))return false;
    const auto groups=availableAiMilitaryGroups(player,std::nullopt);
    if(!groups)return false;
    for(const auto &members:*groups) {
        if(state.nextMilitaryGroup==INT32_MAX)return false;
        AiMilitaryGatherGroup group{state.nextMilitaryGroup++,1,0,{}};
        group.members.assign(members.begin(),members.end());group.desiredCount=int(members.size());
        group.flags|=8u; // Original5ae70a marks attack-now groups to shrink requested strength after losses.
        if(!members.empty())group.lastMemberAdded=uint32_t(std::llround(gameTime_*1000.0));
        for(int32_t id:group.members)queueAiMilitaryStop(player,id);
        state.militaryGroups.push_back(std::move(group));
    }
    return true;
}

bool Game::aiMilitaryCanUseNativePolicy(int player,uint32_t id) const {
    if(player<=0 || size_t(player)>=aiPlayers_.size())return false;
    const auto &groups=aiPlayers_[size_t(player)].militaryGroups;
    for(const auto &group:groups)if(std::find(group.members.begin(),group.members.end(),int32_t(id))!=group.members.end())
        return false; // Original group planning/search now owns grouped actors, including suspended searches.
    const auto *object=findObject(id);
    const bool unsupported=object && (!aiMilitarySoldierClass(object->unit->cls) || object->unit->cls==51);
    return !aiPlayers_[size_t(player)].scripted && (groups.empty() || unsupported);
}

void Game::queueAiMilitaryAttack(int player,const AiMilitaryGatherGroup &group,const Object &target) {
    const auto members=group.members;const auto targetId=target.spawnId;
    // Original44d720 emits one local opcode2 packet. Formation placement remains the shared native provider.
    localCommands_.enqueue([this,player,members,targetId] {
        auto *enemy=findObject(targetId);
        if(!enemy || !enemy->active || enemy->hidden)return;
        std::vector<Object*> actors;
        for(int32_t id:members) {
            auto *actor=id>=0?findObject(uint32_t(id)):nullptr;
            if(actor && actor->active && !actor->hidden && actor->garrisonedInId<0 && actor->player==player)
                actors.push_back(actor);
        }
        issueGroupAttack(actors,*enemy);
    });
}

void Game::updateAiMilitaryTargets(int player,uint32_t budget) {
    auto &state=aiPlayers_[size_t(player)];
    const auto visible=[&](const Object &object) {
        return visibleTargets_.slot(size_t(player-1),object.spawnId)!=size_t(-1);
    };
    // shortcut: Observed native objects supply append/update order until original AI knowledge callbacks are ported.
    for(const auto &object:objects_)if(object.active && object.unit && object.spawnId<=uint32_t(INT32_MAX) && visible(object)) {
        const int32_t id=int32_t(object.spawnId);
        auto slot=state.militaryKnownSlots.emplace(id,state.militaryKnownObjects.size());
        if(slot.second)state.militaryKnownObjects.push_back({});
        auto &record=state.militaryKnownObjects[slot.first->second];
        //58861f/28/31 truncates coordinates;587f2e/3c/4a retains their low bytes.
        record={id,object.player,{uint8_t(int(float(mapSize_)-object.y)),uint8_t(int(object.x)),
                                uint8_t(int(object.heightAboveGround))}};
    }
    std::map<int,std::vector<uint8_t>> zones;
    const auto region=[&](const Object &member,const Object &sample)->std::optional<uint8_t> {
        const int restriction=member.unit->terrainRestriction;
        auto found=zones.find(restriction);
        if(found==zones.end()) {
            std::vector<uint8_t> tiles;
            if(!buildAiMilitaryRegions(restriction,tiles))return std::nullopt;
            found=zones.emplace(restriction,std::move(tiles)).first;
        }
        const float x=float(mapSize_)-sample.y,y=sample.x;
        if(!std::isfinite(x) || !std::isfinite(y) || x<0 || y<0 || x>=mapSize_ || y>=mapSize_)return std::nullopt;
        return found->second[size_t(int(y))*mapSize_+int(x)];
    };
    const auto allied=[&](int owner) {
        return owner==player || (owner>=0 && size_t(owner)<players_[size_t(player-1)].diplomacy.size() &&
                                players_[size_t(player-1)].diplomacy[size_t(owner)]==0);
    };
    const auto distance=[&](const Object &object,float originalX,float originalY) {
        return float(geniePointFootprintDistance(float(mapSize_)-object.y,object.x,originalX,originalY,
                                                object.unit->collisionSize[1],object.unit->collisionSize[0]));
    };
    const auto range=[&](const Object &object) {return modifiedUnitAttribute(object,12,object.unit->maxRange);};
    const auto damage=[&](const Object &source,const Object &target) {
        return AiGroupDamageInput{float(canAttackTarget(source,target)?attackDamage(source,target):0),
            modifiedUnitAttribute(source,10,source.unit->reloadTime),source.unit->cls==50};
    };
    const auto clock=[] {return uint32_t(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count());};
    const uint32_t started=clock();
    const uint32_t simulationTime=uint32_t(std::llround(gameTime_*1000.0));
    for(auto &group:state.militaryGroups) {
        if(group.type==101) {
            const bool retry=group.initialized && state.strategicNumbers.count(76) &&
                aiExploreGroupRetry(group.flags,group.search.lastBusyTime,simulationTime,state.strategicNumbers.at(76));
            if(group.state==6 && (!group.initialized || retry)) {
                group.targetId=-1;group.initialized=true;
                for(int32_t id:group.members) {
                    auto *object=id>=0?findObject(uint32_t(id)):nullptr;
                    if(object && object->active && object->player==player)queueAiExploreDestination(*object);
                }
                if(retry)group.search.lastBusyTime=simulationTime;
            }
            continue;
        }
        if(group.state==18) {
            advanceAiGroupSearch(group.search,group.flags,group.state,simulationTime,[&] {
                for(int32_t id:group.members)queueAiMilitaryStop(player,id);
            });
            continue;
        }
        if(group.state!=2)continue;
        auto *first=group.members.empty()?nullptr:findObject(uint32_t(group.members.front()));
        if(group.targetId>=0) {
            const auto *target=findObject(uint32_t(group.targetId));
            AiGroupRetainedTarget retained;
            retained.resolved=target && target->active;
            retained.memberResolved=first && first->active;
            if(target) {
                retained.hp=target->hitPoints;retained.allied=allied(target->player);
                retained.visibility=visible(*target)?1:0;retained.unitType=target->unit->type;
            }
            const auto decision=retainAiGroupTarget(retained);
            if(decision==AiGroupTargetRetention::Retain)continue;
            if(decision==AiGroupTargetRetention::ResetAndStop)
                for(int32_t id:group.members)queueAiMilitaryStop(player,id);
            group.targetId=-1;group.state=1;group.initialized=false;group.targetCandidates.clear();group.targetRanking={};group.targetContinuation={};
            continue;
        }
        if(!first || !first->active)continue;
        if(!group.targetContinuation.active) {
            std::vector<AiGroupMemberInput> members;
            bool supported=true;
            for(int32_t id:group.members) {
                const auto *object=findObject(uint32_t(id));
                if(!object || !object->active){supported=false;break;}
                const auto own=region(*object,*object);
                if(!own){supported=false;break;}
                members.push_back({id,true,*own,object->unit->trait,object->unit->cls,
                    {float(mapSize_)-object->y,object->heightAboveGround,object->x},object->hitPoints,range(*object)});
            }
            const auto aggregate=supported?aggregateAiTargetGroup(members):std::nullopt;
            if(!aggregate || !aggregate->landMode || aggregate->allClass50)continue;
            group.targetCandidates.clear();group.targetRanking={};group.targetRange=aggregate->minRange;
            group.targetSearchUnsupported=false;
            std::vector<AiTargetCandidate> metrics;
            for(const auto &record:state.militaryKnownObjects) {
                const auto *target=findObject(uint32_t(record.id));
                if(!target || !target->active)continue;
                AiGroupTargetPriorityInputs priorities;
                // shortcut: Current match modes have no Terminate/victory-event controller; connect events109..115 before adding those modes.
                priorities.terminateMode=false;priorities.ownerFlag114=false;priorities.ownerFlag109=false;
                priorities.sn106=state.strategicNumbers.at(106);priorities.sn107=state.strategicNumbers.at(107);
                priorities.sn108=state.strategicNumbers.at(108);
                const auto priority=resolveAiGroupTargetPriority(target->unit->id,target->unit->cls,priorities);
                if(!priority){group.targetSearchUnsupported=true;continue;}
                const auto targetRegion=region(*first,*target);if(!targetRegion){group.targetSearchUnsupported=true;continue;}
                AiGroupTargetFilter filter;
                filter.recordId=record.id;filter.recordOwner=record.owner;filter.allied=allied(target->player);
                filter.hp=target->hitPoints;filter.unitType=target->unit->type;filter.prototypeId=target->unit->id;
                filter.unitClass=target->unit->cls;filter.trait=target->unit->trait;filter.visibility=visible(*target)?1:0;
                filter.region=*targetRegion;
                // shortcut: Native active-owner/target admission substitutes for original virtual300 and owner-state callbacks.
                filter.excludedByVirtual300=target->hidden || target->garrisonedInId>=0;
                const auto admission=filterAiGroupTarget(*aggregate,filter);
                if(admission==AiGroupTargetAdmission::Unsupported)group.targetSearchUnsupported=true;
                if(admission!=AiGroupTargetAdmission::Accepted)continue;
                std::vector<const Object*> threats;
                for(const auto &known:state.militaryKnownObjects) {
                    const auto *threat=findObject(uint32_t(known.id));
                    const bool enemy=known.owner>0 && size_t(known.owner)<players_[size_t(player-1)].diplomacy.size() &&
                        players_[size_t(player-1)].diplomacy[size_t(known.owner)]==3;
                    if(threat && threat->unit && aiGroupThreatInRange(enemy,threat->active,2,threat->hitPoints,
                        distance(*threat,float(mapSize_)-target->y,target->x),range(*threat)))threats.push_back(threat);
                }
                std::vector<AiGroupCombatMember> combat;
                for(int32_t id:group.members) {
                    const auto *object=findObject(uint32_t(id));
                    AiGroupCombatMember member;member.hp=object->hitPoints;member.attack=damage(*object,*target);
                    for(const auto *threat:threats)member.threats.push_back(damage(*threat,*object));
                    combat.push_back(std::move(member));
                }
                const auto metric=buildAiGroupTargetMetric(combat,threats.empty(),target->hitPoints,
                    distance(*target,aggregate->centroid[0],aggregate->centroid[2]),*targetRegion==aggregate->region,
                    target->unit->id,target->unit->cls,*priority);
                if(!metric){group.targetSearchUnsupported=true;continue;}
                group.targetCandidates.push_back(record.id);metrics.push_back(*metric);
                if(metrics.size()==1024)break;
            }
            const auto ranking=rankAiTargets(metrics);
            if(!ranking){group.targetCandidates.clear();continue;}
            group.targetRanking=*ranking;
        }
        const auto selected=selectAiGroupTarget(group.targetRanking,group.targetContinuation,started,budget,clock,[&](size_t index) {
            const auto *target=findObject(uint32_t(group.targetCandidates[index]));
            if(!target || !target->active)return AiGroupTargetAdmission::Rejected;
            std::vector<std::array<float,2>> path;
            // shortcut: Native direct target-path solver replaces4bd560; blocker/transport fallback remains unsupported.
            if(findPath(*first,target->x,target->y,path,target,group.targetRange))return AiGroupTargetAdmission::Accepted;
            const auto own=region(*first,*first),other=region(*first,*target);
            return own && other && *own!=*other?AiGroupTargetAdmission::Rejected:AiGroupTargetAdmission::Unsupported;
        });
        if(group.targetContinuation.active)group.flags|=64u;else group.flags&=~64u;
        if(selected.status==AiGroupTargetSelectionStatus::Selected) {
            group.targetId=group.targetCandidates[selected.index];group.flags|=32u;group.initialized=true;
            if(const auto *target=findObject(uint32_t(group.targetId)))queueAiMilitaryAttack(player,group,*target);
            group.targetCandidates.clear();group.targetRanking={};
        } else if(selected.status==AiGroupTargetSelectionStatus::Exhausted && !group.targetSearchUnsupported) {
            beginAiGroupSearch(group.search,group.flags,group.state,group.initialized,simulationTime,[&] {
                const auto point=aiGroupSearchPoint(group.search,mapSize_,mapSize_,[&] {
                    return genieRandomDraw(legacyRandomState_);
                });
                if(!point)return;
                const auto members=group.members;
                const float x=float((*point)[1]),y=float(mapSize_)-float((*point)[0]);
                // shortcut: Original targetless opcode2 uses native group movement/path placement;
                // original formation attack-move ownership and movement-group callbacks remain unported.
                localCommands_.enqueue([this,player,members,x,y] {
                    std::vector<Object*> actors;
                    for(int32_t id:members) {
                        auto *actor=id>=0?findObject(uint32_t(id)):nullptr;
                        if(actor && actor->active && actor->player==player && !actor->hidden && actor->garrisonedInId<0) {
                            clearUnitCommandOrder(*actor);
                            actors.push_back(actor);
                        }
                    }
                    issueGroupMove(std::move(actors),x,y);
                });
            });
            group.targetId=-1;
        }
    }
}

void Game::updateAiMilitary(int player,uint32_t budget) {
    if(player<=0 || size_t(player)>=aiPlayers_.size())return;
    auto &state=aiPlayers_[size_t(player)];
    if(state.militaryCycle.phase==AiMilitaryPhase::Refresh)reconcileAiMilitaryGroups(player);
    if(state.militaryGroups.empty())return;
    for(int key:{34,41})if(!state.strategicNumbers.count(key))return;
    const auto number=[&](int key) {return state.strategicNumbers.at(key);};
    const auto lookup=[&](int32_t id,const std::array<float,3> &destination)->std::optional<AiMilitaryGatherMember> {
        const auto *object=findObject(uint32_t(id));
        if(!object || !object->active || object->player!=player || object->hidden || object->garrisonedInId>=0)return std::nullopt;
        // shortcut: Native head-state mapping replaces original controller task/command metadata.
        const int task=object->tacticalMovement.present || object->pointMovement.present?610:
            object->exploration?605:object->attackTargetId?600:object->state==State::Idle?-1:0;
        const double distance=geniePointFootprintDistance(float(mapSize_)-object->y,object->x,
            float(mapSize_)-destination[1],destination[0],object->unit->collisionSize[1],object->unit->collisionSize[0]);
        return AiMilitaryGatherMember{distance,task,task==-1?-1:0};
    };
    std::map<int,std::vector<uint8_t>> zones;
    const auto deficitRegions=[&](const AiMilitaryGatherGroup &group)->std::optional<bool> {
        const auto member=std::find_if(group.members.begin(),group.members.end(),[](int32_t id){return id>=0;});
        const auto *first=member==group.members.end()?nullptr:findObject(uint32_t(*member));
        const Object *home=nullptr;
        for(int32_t id:state.militaryHomes) {
            const auto *candidate=findObject(uint32_t(id));
            if(candidate && candidate->unit->id==109){home=candidate;break;}
        }
        if(!first || !home)return false;
        const int restriction=first->unit->terrainRestriction;
        auto found=zones.find(restriction);
        if(found==zones.end()) {
            std::vector<uint8_t> tiles;
            if(!buildAiMilitaryRegions(restriction,tiles))return std::nullopt;
            found=zones.emplace(restriction,std::move(tiles)).first;
        }
        const auto sample=[&](const Object &object)->std::optional<uint8_t> {
            const float x=float(mapSize_)-object.y,y=object.x;
            if(!std::isfinite(x) || !std::isfinite(y) || x<0 || y<0 || x>=mapSize_ || y>=mapSize_)return std::nullopt;
            return found->second[size_t(int(y))*mapSize_+int(x)];
        };
        const auto own=sample(*first),atHome=sample(*home);
        if(!own || !atHome)return std::nullopt;
        return *own!=*atHome;
    };
    const auto clock=[] {return uint32_t(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count());};
    // shortcut: Only recovered phase7/8 ordering is represented; cadence and phase9 policy remain native.
    state.militaryCycle.advance(budget,clock,[&] {
        // Original phase7 replenishes exploration groups before attack groups.
        for(auto &group:state.militaryGroups)if(group.type==101 && !group.initialized &&
            int64_t(group.members.size())<group.desiredCount) {
            if(group.flags&8u) {group.desiredCount=int(group.members.size());continue;}
            std::optional<int32_t> selected;
            if(group.desiredCount==1) {
                std::vector<int32_t> pool;
                for(int32_t id:state.militarySoldiers) {
                    const auto *object=findObject(uint32_t(id));
                    if(!object || object->hidden || object->garrisonedInId>=0)continue;
                    const bool assigned=std::any_of(state.militaryGroups.begin(),state.militaryGroups.end(),[&](const auto &g) {
                        return std::find(g.members.begin(),g.members.end(),id)!=g.members.end();});
                    if(!assigned)pool.push_back(id);
                }
                selected=firstAiPreferredExplorer(pool,[&](int32_t id)->std::optional<int> {
                    const auto *object=findObject(uint32_t(id));return object?std::optional<int>(object->unit->cls):std::nullopt;
                });
            }
            if(selected) {group.members.push_back(*selected);group.lastMemberAdded=uint32_t(std::llround(gameTime_*1000.0));}
            else refillAiMilitaryGroup(group,0,group.flags,uint32_t(std::llround(gameTime_*1000.0)),
                [&](int requested)->std::optional<std::vector<int32_t>> {
                    const auto groups=availableAiMilitaryGroups(player,requested,true);
                    if(!groups)return std::nullopt;
                    return groups->empty()?std::vector<int32_t>{}:groups->front();
                });
        }
        refreshAiGatherDestinations(state.militaryGroups,[&](int32_t id)->std::optional<std::array<float,3>> {
            const auto *object=findObject(uint32_t(id));if(!object)return std::nullopt;
            return std::array<float,3>{object->x,object->y,object->heightAboveGround};
        });
        for(auto &group:state.militaryGroups) {
            std::vector<AiGroupSearchMember> searchMembers;
            for(int32_t id:group.members) {
                const auto *object=id>=0?findObject(uint32_t(id)):nullptr;
                if(!object || !object->active || object->player!=player || object->hitPoints<=0)continue;
                // shortcut: Native action slots supply task/state; no original movement-group registry yet.
                const int task=object->tacticalMovement.present || object->pointMovement.present?610:
                    object->exploration?605:object->attackTargetId?600:object->state==State::Idle?-1:0;
                const int actionState=object->tacticalMovement.present?object->tacticalMovement.state:
                    object->pointMovement.present?(object->pointMovement.status==GenieMovementStatus::Active?4:1):2;
                searchMembers.push_back({{float(mapSize_)-object->y,object->heightAboveGround,object->x},
                    true,aiGroupSearchIdle(actionState,task,-1,false,true)});
            }
            refreshAiGroupSearch(group.search,group.flags,searchMembers,uint32_t(std::llround(gameTime_*1000.0)));
            if(group.type==101)continue;
            // Native groups currently originate from attack-now (flag8). Other origins/initialization remain unported.
            const int initialized=group.initialized?1:0;
            refillAiMilitaryGroup(group,initialized,group.flags,uint32_t(std::llround(gameTime_*1000.0)),
                [&](int requested)->std::optional<std::vector<int32_t>> {
                    const auto selected=availableAiMilitaryGroups(player,requested);
                    if(!selected)return std::nullopt;
                    return selected->empty()?std::vector<int32_t>{}:selected->front();
                });
        }
    },[&](uint32_t started,uint32_t remainingBudget) {
        const auto result=state.militaryReadiness.advance(state.militaryGroups,started,remainingBudget,clock,
            [&](const auto &group) {return aiMilitaryGatherReady(group,number(41),lookup);},
            [&](auto &group) {dispatchAiMilitaryGather(group,player,7,number(41),number(34),false,lookup,
                [&](int32_t id) {queueAiMilitaryStop(player,id);},
                [&](const auto &order) {if(order.range>=0)tacticalCommands_.enqueue(order);});},
            [&](int32_t id) {queueAiMilitaryStop(player,id);},[] {},deficitRegions);
        if(result==AiMilitaryReadinessResult::Unsupported) {
            // Invalid native region input stays unsupported; skip it without releasing the group.
            const auto it=std::find_if(state.militaryGroups.begin(),state.militaryGroups.end(),[&](const auto &group) {return group.id==state.militaryReadiness.nextGroupId;});
            state.militaryReadiness.nextGroupId=it!=state.militaryGroups.end() && it+1!=state.militaryGroups.end()?(it+1)->id:-1;
            return state.militaryReadiness.nextGroupId==-1;
        }
        return result==AiMilitaryReadinessResult::Complete;
    },[&] {updateAiMilitaryTargets(player,budget);updateSkirmishArmy(player);});
}
} // namespace swgb
