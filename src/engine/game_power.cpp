// SPDX-License-Identifier: GPL-3.0-or-later
#include "game.h"
#include "ai_power_field.h"

namespace swgb {
void Game::refreshPowerFields() {
    powerTiles_.clear();
    if(mapSize_<=0)return;
    powerTiles_.assign(size_t(mapSize_)*size_t(mapSize_),0);
    //61ef05 clears the field once before the world's per-owner updates. This
    // snapshot must not be refreshed lazily by queries or while paused.
    for(const auto &source:objects_) {
        if(!source.unit || source.player<1 || source.player>8 ||
           source.updateOwner<0 || source.updateList!=0)continue;
        const int state=source.active && !source.underConstruction && !source.resourceDeathState?2:0;
        const bool contained=source.garrisonedInId>=0;
        const int radius=originalPowerRadius(source.unit->id,source.unit->cls,contained,state,
            playerAttribute(source.player,18),playerAttribute(source.player,30));
        if(!radius)continue;
        const Object *center=&source;
        if(contained) {
            center=findObject(uint32_t(source.garrisonedInId));
            if(!center)continue;
        }
        const uint16_t bit=uint16_t(1u<<(source.player-1));
        forEachOriginalPowerTile(mapSize_,mapSize_,float(mapSize_)-center->y,center->x,radius,
            [&](int x,int y){powerTiles_[size_t(mapSize_-1-x)*mapSize_+y]|=bit;});
    }
}

bool Game::requiresPower(const Object &building) const {
    if(!building.unit)return false;
    if(building.annexParentId) {
        const auto *parent=findObject(building.annexParentId);
        if(parent && parent!=&building)return requiresPower(*parent);
    }
    return originalBuildingRequiresPower(building.unit->id);
}

bool Game::isPowered(const Object &building) const {
    if(!building.unit)return false;
    //553150 delegates annex power to its parent before54bda0's prototype test.
    if(building.annexParentId) {
        const auto *parent=findObject(building.annexParentId);
        if(parent && parent!=&building)return isPowered(*parent);
    }
    if(originalPowerExempt(building.unit->id,building.unit->cls))return true;
    if(building.player<1 || building.player>8 || mapSize_<=0)return false;
    const Object *center=&building;
    if(building.garrisonedInId>=0) {
        center=findObject(uint32_t(building.garrisonedInId));
        if(!center)return false;
    }
    const float ox=float(mapSize_)-center->y,oy=center->x;
    if(!std::isfinite(ox) || !std::isfinite(oy) || ox<0 || oy<0 || ox>=mapSize_ || oy>=mapSize_)return false;
    const size_t tile=size_t(mapSize_-1-int(ox))*mapSize_+int(oy);
    // InformationAI calls virtual54(0): only the owning player's power bit.
    return tile<powerTiles_.size() && (powerTiles_[tile] & uint16_t(1u<<(building.player-1)))!=0;
}
} // namespace swgb
