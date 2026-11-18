// SPDX-License-Identifier: GPL-3.0-or-later
#include "game.h"
#include <algorithm>
#include <cmath>

namespace swgb {
bool Game::queueTacticalMove(Object &object,float x,float y,float range) {
    if(!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(range) ||
       x<0 || y<0 || x>=mapSize_ || y>=mapSize_ || range<0)return false;
    TacticalCommandQueue::Order order{object.spawnId,object.player,x,y};
    order.command=724;order.z=1;order.range=range;
    return tacticalCommands_.enqueue(order);
}

bool Game::issueTacticalMove(Object &object,const TacticalCommandQueue::Order &order) {
    if(!object.active || object.hidden || object.garrisonedInId>=0 || !object.unit ||
       object.unit->speed<=0 || object.unit->cls==36 || order.target!=-1 || order.flagA!=1 || order.flagB!=0 ||
       !std::isfinite(order.x) || !std::isfinite(order.y) || !std::isfinite(order.range) ||
       order.x<0 || order.y<0 || order.x>=mapSize_ || order.y>=mapSize_ || order.range<0)return false;
    stopUnit(object);
    auto &action=object.tacticalMovement;
    action.present=true;action.state=3;action.priority=order.flagC;
    action.destinationX=order.x;action.destinationY=order.y;action.range=order.range;
    object.pointMovement.begin(order.x,order.y);
    object.moveAnchorX=order.x;object.moveAnchorY=order.y;
    object.targetX=order.x;object.targetY=order.y;
    return true;
}

// Returns true when this admitted owner pass must not run the native Walk provider.
bool Game::updateTacticalMovement(Object &object,int previousGraphic) {
    auto &action=object.tacticalMovement;
    if(!action.present)return false;
    const double distance=geniePointFootprintDistance(float(mapSize_)-object.y,object.x,float(mapSize_)-action.destinationY,action.destinationX,
                                                       object.unit->collisionSize[1],object.unit->collisionSize[0]);
    bool prepared=false;
    const int outcome=action.update(distance,[&] {
        object.moveGoalActive=true;
        issueMove(object,action.destinationX,action.destinationY,nullptr,action.range);
        object.pointMovement.status=GenieMovementStatus::Active;
        prepared=true;
        return true; // Original388 accepts the request before route solving, even for occupied endpoints.
    },[&] {
        // shortcut: Native solving is synchronous; expose its result after request acceptance until original path-service retries are ported.
        if(prepared)return 0;
        if(object.pointMovement.status==GenieMovementStatus::Failed)return 1;
        // shortcut: Native terminal point paths stand in for original pathdata/flags.
        // Replace this qualification when4bbf30/4bc1c0 path state is represented.
        const bool terminalPointPath=!object.pathGoalId && !object.path.empty() &&
            object.pathIndex+1>=object.path.size() &&
            object.path.back()[0]==action.destinationX && object.path.back()[1]==action.destinationY;
        if(geniePointPathSpacingArrival(false,terminalPointPath,0,0,action.range,distance))return 2;
        if(object.pointMovement.status==GenieMovementStatus::Completed ||
           (object.state==State::Idle && object.pathIndex>=object.path.size()))
            return distance<=action.range?2:1;
        return 0;
    },[&](int) {
        // shortcut: Native AI observes retained610/retired movement through its actor snapshot;
        // original virtual1e0(505/506,610) notification side effects are not yet represented.
    });
    if(action.state!=4) {
        object.path.clear();object.pathIndex=0;object.moveGoalActive=false;
        object.state=State::Idle;object.moveSpeedLimit=0;
    }
    const int graphic=objectGraphic(object,*object.unit);
    if(graphic!=previousGraphic)resetObjectGraphicAnimation(object,graphic);
    if(outcome==1 || outcome==3) {
        action.cancel();object.pointMovement.cancel();return true;
    }
    return action.state!=4;
}
} // namespace swgb
