// SPDX-License-Identifier: GPL-3.0-or-later
#include "game.h"
#include "ai_conversion_eligibility.h"
#include "../core/genie_random.h"

namespace swgb {
void Game::queueConversionThreat(Object &target,const Object &source) {
    // 445110 ->419d50 ->421e30 queues actor500 without inflicting damage.
    target.pendingConversionThreats.push_back(source.spawnId);
    // Native frozen-building scheduling adapter: deliver on its next update.
    setBuildingFrozen(target,false);
}

void Game::drainConversionThreats(Object &target) {
    auto pending=std::move(target.pendingConversionThreats);
    target.pendingConversionThreats.clear();
    for(uint32_t attackerId:pending) {
        // 41f2e0 emits owner513 before target reselection. Reuse the same
        // owner admission/RNG/dedup path as damage, without damage statistics.
        Object *attacker=findObject(attackerId);
        notifyAiAttackMemory(target,attacker);
        // The same generic target response follows owner513, without damage.
        respondToAttackThreat(target,attacker,true);
    }
}

const dat::Task *Game::conversionTask(const Object &source) const {
    if (!source.unit || source.unit->id < 0 ||
        size_t(source.unit->id) >= assets_.dat().unitHeaders.size()) return nullptr;
    for (const auto &task:assets_.dat().unitHeaders[size_t(source.unit->id)].tasks)
        if (task.actionType==104) return &task;
    return nullptr;
}

bool Game::conversionEligible(const Object &source,const Object &target,bool ignoreCharge) const {
    if (!source.unit || !target.unit || !conversionTask(source)) return false;
    AiConversionEligibility input;
    input.sourceClass=source.unit->cls;
    input.sourceVirtual30=aiConversionSourceVirtual30(source.unit->id,source.unit->copyId);
    input.sourceVirtual34=aiConversionSourceVirtual34(source.unit->id,source.unit->copyId);
    input.charge=source.conversionCharge;input.ignoreCharge=ignoreCharge;
    input.targetOwner=target.player;input.targetClass=target.unit->cls;
    input.targetPrototype=target.unit->id;input.targetType=target.unit->type;
    input.targetImmunity=target.unit->heroMode;
    input.enabledResource67=playerAttribute(source.player,67);
    input.jediResource27=playerAttribute(source.player,27);
    input.advancedResource87=playerAttribute(source.player,87);
    return aiConversionEligible(input);
}

bool Game::canConvert(const Object &source,const Object &target,bool ignoreCharge) const {
    return source.active && !source.hidden && source.garrisonedInId<0 &&
        target.active && !target.hidden && !target.underConstruction && target.garrisonedInId<0 &&
        isEnemy(source,target) && combatTargetVisible(source,target) &&
        (canTransferConvertedObject(target,source.player) || playerAttribute(target.player,192)>0) &&
        conversionEligible(source,target,ignoreCharge);
}

void Game::clearConversion(Object &source) {
    setTaskReference(source.conversionTargetId,0);
    source.conversionPhase=Object::ConversionPhase::None;
    source.conversionClock={};
    if (source.state==State::Convert) source.state=State::Idle;
}

bool Game::issueConversionCommand(Object &source,Object &target) {
    if (!canConvert(source,target)) return false;
    stopUnit(source);
    setTaskReference(source.conversionTargetId,target.spawnId);
    source.conversionLastSeenX=target.x;source.conversionLastSeenY=target.y;
    const bool building=target.unit->cls==18 || target.unit->cls==9 || target.unit->cls==10;
    source.conversionRange=building && !(playerAttribute(source.player,28)>0)
        ? .5f:playerAttribute(source.player,5);
    source.conversionPhase=Object::ConversionPhase::Approach;
    return true;
}

void Game::updateConversion(Object &source,float dt) {
    if (source.unit->type==dat::UT_Creatable && source.unit->cls==50)
        source.conversionCharge=genieConversionCharge(source.conversionCharge,
            playerAttribute(source.player,35),dt);
    if (!source.conversionTargetId) return;
    auto *target=findObject(source.conversionTargetId);
    const auto *task=conversionTask(source);
    if (!task || !target || !target->active || target->hidden || target->underConstruction || target->garrisonedInId>=0) {
        stopUnit(source);return;
    }
    if (isFriendlyPlayer(source.player,target->player)) {
        if (!(playerAttribute(target->player,193)>0)) source.conversionCharge=0;
        stopUnit(source);return;
    }
    const float range=source.conversionRange;
    const bool inRange=withinInteractionRange(source,*target,range);
    if (combatTargetVisible(source,*target)) {
        source.conversionLastSeenX=target->x;source.conversionLastSeenY=target->y;
    } else if (!inRange && std::hypot(target->x-source.conversionLastSeenX,
                                     target->y-source.conversionLastSeenY)>source.unit->lineOfSight) {
        const float x=source.conversionLastSeenX,y=source.conversionLastSeenY;
        stopUnit(source);source.moveGoalActive=issueMove(source,x,y);return;
    }
    using Phase=Object::ConversionPhase;
    if (source.conversionPhase==Phase::Approach) {
        if (!inRange) {
            approach(source,*target,range);
            return;
        }
        source.path.clear();source.pathIndex=0;source.moveGoalActive=false;
        source.state=State::Convert;source.conversionPhase=Phase::Orient;
        constexpr float pi=3.1415927410125732f,tau=6.2831854820251465f;
        source.conversionOrientation.angle=std::fmod(source.facing+pi*.5f+tau,tau);
        return;
    }
    if (source.conversionPhase==Phase::Orient) {
        constexpr float pi=3.1415927410125732f;
        auto &orientation=source.conversionOrientation;
        const auto *graphic=assets_.dat().graphic(objectGraphic(source,*source.unit));
        const auto result=genieOrientToward(orientation,float(mapSize_)-source.y,source.x,
            float(mapSize_)-target->y,target->x,graphic?graphic->angleCount:0,
            source.unit->rotationSpeed,uint32_t(std::llround(gameTime_*1000.)));
        if (result.facetChanged) source.facing=orientation.angle-pi*.5f;
        if (!result.ready) return;
        source.conversionClock.beginPulse(modifiedUnitAttribute(source,10,source.unit->reloadTime));
        source.conversionPhase=Phase::Pulse;source.state=State::Convert;
        resetObjectGraphicAnimation(source,task->proceedingGraphic);
        playWorldUnitSound(source,graphicSound(task->proceedingGraphic));
        playWorldUnitSound(source,task->resourceGatheringSound);
        if(source.conversionClock.elapsed<=0)queueConversionThreat(*target,source);
        return;
    }
    if (source.conversionPhase!=Phase::Pulse) return;
    queueConversionThreat(*target,source);
    if(!source.conversionClock.advancePulse(dt))return;
    GenieConversionRollInput input;
    input.targetClass=target->unit->cls;input.targetInterfaceKind=target->unit->interfaceKind;
    input.targetTrait=target->unit->trait;input.targetResistance77=playerAttribute(target->player,77);
    input.elapsed=source.conversionClock.elapsed;input.taskMinimum=task->workValue1;
    input.taskMaximum=task->workValue2;input.unitChance=source.unit->accuracyPercent;
    input.sourceMinimum176=playerAttribute(source.player,176);
    input.sourceMaximum177=playerAttribute(source.player,177);
    input.targetMinimum178=playerAttribute(target->player,178);
    input.targetMaximum179=playerAttribute(target->player,179);
    input.buildingMinimum180=playerAttribute(source.player,180);
    input.buildingMaximum181=playerAttribute(source.player,181);
    input.buildingChance182=playerAttribute(source.player,182);
    const auto roll=genieConversionRoll(genieRandomDraw(legacyRandomState_),input);
    if (!roll) {stopUnit(source);return;}
    if (roll->succeeds) {
        const int oldOwner=target->player;
        const auto costs=unitCosts(*target);
        float value=0;
        for (const auto &cost:costs)
            if ((cost.flag&0xff)==1 && cost.amount>0) value+=cost.amount;
        if (playerAttribute(oldOwner,192)>0) killObject(*target,false);
        else if (!transferConvertedObject(*target,source.player)) {stopUnit(source);return;}
        addResource(oldOwner,154,1);
        resources_[size_t(oldOwner)][98]=std::max(0.f,resource(oldOwner,98)-value);
        addResource(source.player,41,1);addResource(source.player,170,value);
        source.conversionCharge=0;stopUnit(source);
    } else {
        source.conversionPhase=Phase::Approach;source.state=State::Idle;
    }
}
} // namespace swgb
