// SPDX-License-Identifier: GPL-3.0-or-later
#include "game.h"
#include "ai_military.h"
#include "rms_zones.h"
#include "../core/genie_tactical_movement.h"
#include <chrono>
#include <limits>

namespace swgb {
void Game::refreshAiMilitaryMembers(int player) {
    auto &state=aiPlayers_[size_t(player)];
    // shortcut: Native object lifecycle supplies admission/removal, not recovered 596110 notifications.
    for(auto it=state.militarySoldiers.begin();it!=state.militarySoldiers.end();) {
        const auto *object=findObject(uint32_t(*it));
        if(!object || !object->active || object->player!=player || !object->unit ||
           !aiMilitarySoldierClass(object->unit->cls)) {
            removeAiMilitaryMember(state.militaryGroups,*it);
            it=state.militarySoldiers.erase(it);
        } else ++it;
    }
    for(const auto &object:objects_) {
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

bool Game::formAiMilitaryGroups(int player) {
    if(player<=0 || size_t(player)>=aiPlayers_.size() || mapSize_<=0)return false;
    auto &state=aiPlayers_[size_t(player)];
    refreshAiMilitaryMembers(player);
    // Do not invent missing constructor defaults: PER must provide these until the original profile is connected.
    for(int key:{34,41,227,230})if(!state.strategicNumbers.count(key))return false;
    const auto number=[&](int key) {return state.strategicNumbers.at(key);};
    const int percentage=number(227);
    if(percentage<=0)return true;
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
    if(available.empty())return true;
    std::vector<uint8_t> terrain(size_t(mapSize_)*mapSize_);
    for(int y=0;y<mapSize_;++y)for(int x=0;x<mapSize_;++x) {
        const int value=terrainAt(y,mapSize_-1-x);
        if(value<0 || value>255)return false;
        terrain[size_t(y)*mapSize_+x]=uint8_t(value);
    }
    // Recovered terrain-only connected regions, sampled in native world coordinates.
    // shortcut: Rebuilding on admission observes current terrain; original zone-cache lifetime is not wired here.
    std::map<int,RmsZones> zones;
    const auto &restrictions=assets_.dat().terrainRestrictions;
    const auto region=[&](const Object &center,const Object &sample)->std::optional<uint8_t> {
        const int restriction=center.unit->terrainRestriction;
        if(restriction<0 || size_t(restriction)>=restrictions.size() || !std::isfinite(sample.x) ||
           !std::isfinite(sample.y) || sample.x<0 || sample.y<0 || sample.x>=mapSize_ || sample.y>=mapSize_)return std::nullopt;
        auto it=zones.find(restriction);
        if(it==zones.end()) {
            RmsZones result;
            if(!generateRmsZones(uint32_t(mapSize_),terrain,restrictions[size_t(restriction)].passableBuildableDmgMultiplier,result))return std::nullopt;
            it=zones.emplace(restriction,std::move(result)).first;
        }
        const int originalX=int(float(mapSize_)-sample.y),originalY=int(sample.x);
        if(originalX<0 || originalX>=mapSize_ || originalY<0 || originalY>=mapSize_)return std::nullopt;
        return it->second.tiles[size_t(originalY)*mapSize_+originalX];
    };
    std::vector<AiMilitaryCandidate> candidates;
    for(const auto *object:available) {
        const auto own=region(*object,*object);if(!own)return false;
        candidates.push_back({int(object->spawnId),float(mapSize_)-object->y,object->x,*own});
    }
    const size_t n=std::min<size_t>(available.size(),40);
    std::vector<uint8_t> projected(n*n);
    for(size_t i=0;i<n;++i)for(size_t j=0;j<n;++j) {
        const auto value=region(*available[i],*available[j]);if(!value)return false;
        projected[i*n+j]=*value;
    }
    const auto groups=selectAiMilitaryGroups(candidates,projected,aiMilitaryAttackCount(candidates.size(),percentage),float(number(230)));
    if(!groups)return false;
    for(const auto &members:*groups) {
        if(state.nextMilitaryGroup==INT32_MAX)return false;
        AiMilitaryGatherGroup group{state.nextMilitaryGroup++};
        group.members.assign(members.begin(),members.end());group.desiredCount=int(members.size());
        for(int32_t id:group.members)queueAiMilitaryStop(player,id);
        state.militaryGroups.push_back(std::move(group));
    }
    return true;
}

bool Game::aiMilitaryCanUseNativePolicy(int player,uint32_t id) const {
    if(player<=0 || size_t(player)>=aiPlayers_.size())return false;
    const auto &groups=aiPlayers_[size_t(player)].militaryGroups;
    for(const auto &group:groups)if(std::find(group.members.begin(),group.members.end(),int32_t(id))!=group.members.end())
        return aiPlayers_[size_t(player)].militaryCycle.phase==AiMilitaryPhase::Planning &&
               (group.state==2 || group.desiredCount>int(group.members.size()));
    const auto *object=findObject(id);
    const bool unsupported=object && (!aiMilitarySoldierClass(object->unit->cls) || object->unit->cls==51);
    return !aiPlayers_[size_t(player)].scripted && (groups.empty() || unsupported);
}

void Game::updateAiMilitary(int player,uint32_t budget) {
    if(player<=0 || size_t(player)>=aiPlayers_.size())return;
    auto &state=aiPlayers_[size_t(player)];
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
    const auto clock=[] {return uint32_t(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count());};
    // shortcut: Only recovered phase7/8 ordering is represented; cadence and phase9 policy remain native.
    state.militaryCycle.advance(budget,clock,[&] {
        refreshAiGatherDestinations(state.militaryGroups,[&](int32_t id)->std::optional<std::array<float,3>> {
            const auto *object=findObject(uint32_t(id));if(!object)return std::nullopt;
            return std::array<float,3>{object->x,object->y,object->heightAboveGround};
        });
    },[&](uint32_t started,uint32_t remainingBudget) {
        const auto result=state.militaryReadiness.advance(state.militaryGroups,started,remainingBudget,clock,
            [&](const auto &group) {return aiMilitaryGatherReady(group,number(41),lookup);},
            [&](auto &group) {dispatchAiMilitaryGather(group,player,7,number(41),number(34),false,lookup,
                [&](int32_t id) {queueAiMilitaryStop(player,id);},
                [&](const auto &order) {if(order.range>=0)tacticalCommands_.enqueue(order);});},
            [&](int32_t id) {queueAiMilitaryStop(player,id);},[] {});
        if(result==AiMilitaryReadinessResult::Unsupported) {
            // shortcut: Skip deficits until original home/region callbacks exist; planning uses native policy for them.
            const auto it=std::find_if(state.militaryGroups.begin(),state.militaryGroups.end(),[&](const auto &group) {return group.id==state.militaryReadiness.nextGroupId;});
            state.militaryReadiness.nextGroupId=it!=state.militaryGroups.end() && it+1!=state.militaryGroups.end()?(it+1)->id:-1;
            return state.militaryReadiness.nextGroupId==-1;
        }
        return result==AiMilitaryReadinessResult::Complete;
    },[&] {updateSkirmishArmy(player);});
}
} // namespace swgb
