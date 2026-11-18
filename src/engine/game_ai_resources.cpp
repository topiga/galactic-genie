// SPDX-License-Identifier: GPL-3.0-or-later
#include "game.h"
#include "ai_resource_knowledge.h"
#include <algorithm>
#include <cmath>
namespace swgb {
namespace {
//551b80/551ba0: collision-box edge distance, rounded tofloat by590d25.
float resourceEdgeDistance(float x,float y,float ax,float ay,float bx,float by) {
    const float dx=std::max(0.0f,float(std::abs(double(x))-double(ax)-double(bx)));
    const double dy=std::max(0.0,std::abs(double(y))-double(ay)-double(by));
    return float(std::sqrt(double(dx)*dx+dy*dy));
}
}
void Game::updateAiResourceKnowledge(int player) {
    if(player<=0 || size_t(player)>=aiPlayers_.size() || mapSize_<=0)return;
    auto &state=aiPlayers_[size_t(player)];auto &knowledge=state.resourceKnowledge;
    const auto sn=[&](int n){auto i=state.strategicNumbers.find(n);return i==state.strategicNumbers.end()?(n==86?25:100):i->second;};
    const Object *center=nullptr;
    // shortcut: First native-owned109 stands in for MainAI413870's object lookup order.
    for(const auto &o:objects_)if(o.active && o.player==player && o.unit && o.unit->id==109){center=&o;break;}
    const size_t cells=size_t(mapSize_)*mapSize_;
    if(knowledge.observed.size()!=cells)knowledge.observed.assign(cells,0);
    std::vector<uint8_t> trees(cells,0);
    for(const auto &o:objects_) {
        if(!o.active || !o.unit)continue;
        const int x=int(std::floor(o.x)),y=int(std::ceil(o.y))-1;
        if(o.unit->cls==31 && x>=0 && y>=0 && x<mapSize_ && y<mapSize_)trees[size_t(y)*mapSize_+x]=1;
        // shortcut: Native visible-index admission replaces original588340 notifications;
        // resource knowledge persists when sight is lost. Never grant unseen object knowledge.
        if(visibleTargets_.slot(size_t(player-1),o.spawnId)==size_t(-1))continue;
        const int cls=o.unit->cls;
        if(cls!=27 && cls!=29 && cls!=30)continue;
        if(!knowledge.objects.insert(o.spawnId).second)continue;
        const float distance=center?resourceEdgeDistance(center->y-o.y,center->x-o.x,
            o.unit->collisionSize[1],o.unit->collisionSize[0],center->unit->collisionSize[1],center->unit->collisionSize[0]):0;
        knowledge.admit(cls,center!=nullptr,distance,sn(86),sn(87));
    }
    const uint16_t mask=sharedSightMask(player);
    for(size_t i=0;i<cells;++i)if(!knowledge.observed[i] && i<observedTiles_.size() && (observedTiles_[i]&mask)) {
        knowledge.observed[i]=1;
        // Original585570 tests forest terrain plus an actual class31 on that tile.
        // shortcut: Current native tile occupants supply the first-observation callback.
        if(i<terrain_.size() && i<visibleTiles_.size() && (visibleTiles_[i]&mask))
            knowledge.forestTile(terrain_[i],trees[i]!=0);
    }
}
void Game::reevaluateAiResourceFound(int player) {
    if(player<=0 || size_t(player)>=aiPlayers_.size())return;
    auto &state=aiPlayers_[size_t(player)];auto &knowledge=state.resourceKnowledge;
    const Object *center=nullptr;
    for(const auto &o:objects_)if(o.active && o.player==player && o.unit && o.unit->id==109){center=&o;break;}
    const auto sn=[&](int n){auto i=state.strategicNumbers.find(n);return i==state.strategicNumbers.end()?(n==86?25:100):i->second;};
    //5920e0 is called by the86/87 setter, not periodically and not by fact evaluation.
    // shortcut: Native absent/inactive resource invalidation replaces original record+c/+d flags;
    // economic assignment flags are not yet represented. Sticky found bits are never cleared.
    for(uint32_t id:knowledge.objects) {
        const auto *o=findObject(id);if(!o || !o->active || !o->unit)continue;
        const float distance=center?resourceEdgeDistance(center->y-o->y,center->x-o->x,
            o->unit->collisionSize[1],o->unit->collisionSize[0],center->unit->collisionSize[1],center->unit->collisionSize[0]):0;
        knowledge.admit(o->unit->cls,center!=nullptr,distance,sn(86),sn(87));
    }
}
} // namespace swgb
