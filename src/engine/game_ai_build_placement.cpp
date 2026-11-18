// SPDX-License-Identifier: GPL-3.0-or-later
#include "game.h"
#include "../core/genie_random.h"
#include "ai_prefab_grid.h"
#include "ai_build_placement.h"
#include <algorithm>
#include <cmath>

namespace swgb {
//1=found,0=supported search found no location,-1=unsupported provider context.
int Game::chooseAiOriginalBuildPosition(int player,const dat::Unit &unit,float &x,float &y,
                                 std::string *unsupportedReason) {
    if(unsupportedReason)unsupportedReason->clear();
    const auto unsupported=[&](const char *reason) {
        if(unsupportedReason)*unsupportedReason=reason;
        return -1;
    };
    if((unit.id!=70 && unit.id!=50) || unit.type!=dat::UT_Building || player<=0 ||
       size_t(player)>=aiPlayers_.size() || mapSize_<=0 || mapSize_>255)
        return unsupported("normal Clone Campaigns prefab or farm profile required");
    auto &state=aiPlayers_[size_t(player)];
    const auto radius=state.strategicNumbers.find(74),randomness=state.strategicNumbers.find(168);
    // Actual596d10 profile followed by597300 installs SN74=20/SN168=50.
    const int searchRadius=radius==state.strategicNumbers.end()?20:radius->second;
    const int placementRandomness=randomness==state.strategicNumbers.end()?50:randomness->second;
    if(searchRadius<0 || searchRadius>mapSize_)
        return unsupported("placement strategic-number context unavailable");
    const float widthFloat=unit.collisionSize[0]*2.f;
    if(!std::isfinite(widthFloat) || widthFloat<1 || widthFloat>mapSize_ ||
       widthFloat!=std::floor(widthFloat) || unit.collisionSize[0]!=unit.collisionSize[1])
        return unsupported("non-square building footprint");
    //58f260's event rectangles and58e9f0's wall plans are not native registries
    // yet. Restrict this adapter to contexts where they are absent, before RNG.
    for(const auto &quarter:state.explorationQuarters)
        if(quarter.received || quarter.issued)
            return unsupported("attack-history placement rectangles are not represented");
    if(wallDragActive_ && localPlayer_==player)
        return unsupported("pending wall placement is not represented");
    refreshAiEconomyKnowledge(player);
    auto &economy=state.economy;
    for(int32_t id:economy.depotIds) {
        const auto *building=findObject(uint32_t(id));
        if(building && building->unit && (building->unit->cls==6 || building->unit->cls==8))
            return unsupported("wall-plan placement stage is not represented");
    }
    if(economy.civilianIds.empty())return 0;
    const Object *anchor=findObject(uint32_t(economy.civilianIds.front()));
    if(!anchor || !anchor->active || !anchor->unit)return 0;
    for(int32_t id:economy.depotIds) {
        const auto *building=findObject(uint32_t(id));
        if(building && building->unit && building->unit->id==109) {anchor=building;break;}
    }
    const int originX=int(float(mapSize_)-anchor->y),originY=int(anchor->x);
    if(originX<0 || originY<0 || originX>=mapSize_ || originY>=mapSize_)
        return unsupported("placement anchor outside original map");
    const float powerRadius=playerAttribute(player,18),specialRadius=playerAttribute(player,24);
    if(!std::isfinite(powerRadius) || !std::isfinite(specialRadius))
        return unsupported("invalid player placement attributes");
    AiBuildPlacementGrid grid(mapSize_,originalBuildingPlacementCategory(unit.id));
    //58b100 scans remembered resource buckets. Lookup uses the live object just
    // as413850 does; absence/record.invalid=1 skip it. No new hidden knowledge.
    for(size_t bucket=0;bucket<economy.resources.size();++bucket)
        for(const auto &record:economy.resources[bucket].records) {
            const auto *resource=findObject(uint32_t(record.id));
            if(!resource || !resource->active || !resource->unit)continue;
            grid.resource(int(bucket),resource->unit->cls,float(mapSize_)-resource->y,resource->x,
                          resource->unit->collisionSize[0],resource->unit->collisionSize[1],record.invalid==1);
        }
    // shortcut: Native static-obstruction tile admission stands in for original
    // path-map bits0x4003. The scoring operations/terrain cases are original;
    // native footprint registration is not claimed to reproduce those bits.
    for(int oy=0;oy<mapSize_;++oy)for(int ox=0;ox<mapSize_;++ox) {
        const int nx=oy,ny=mapSize_-1-ox;
        const size_t tile=size_t(ny)*mapSize_+nx;
        bool occupied=false;
        if(tile<staticObstructionCells_.size())for(uint32_t index:staticObstructionCells_[tile]) {
            if(index>=objects_.size())continue;
            const auto &object=objects_[index];
            if(object.active && !object.hidden && object.garrisonedInId<0) {occupied=true;break;}
        }
        grid.terrain(ox,oy,terrainAt(nx,ny),occupied?1u:0u);
    }
    // shortcut: The ordered native owned-building registry supplies original
    // InfoAI+3c8 admission. Foundations remain included; completion is no gate.
    for(int32_t id:economy.depotIds) {
        const auto *building=findObject(uint32_t(id));
        if(!building || !building->active || !building->unit)continue;
        grid.ownedBuilding(building->unit->id,float(mapSize_)-building->y,building->x,
            building->unit->collisionSize[0],modifiedUnitAttribute(*building,1,building->unit->lineOfSight),
            powerRadius,specialRadius);
    }
    //58e770 resolves only persistent known records, not all hidden actors.
    // shortcut: active/HP and native modified range provide original actor-state
    // and virtual194 callbacks; the range rectangle arithmetic is original.
    for(const auto &known:state.militaryKnownObjects) {
        const auto *building=findObject(uint32_t(known.id));
        if(!building || !building->active || building->hitPoints<=0 || !building->unit)continue;
        const int cls=building->unit->cls;
        if(!((cls>=6 && cls<=10) || cls==18))continue;
        const auto &diplomacy=players_[size_t(player-1)].diplomacy;
        const bool allied=known.owner==player ||
            (known.owner>=0 && size_t(known.owner)<diplomacy.size() && diplomacy[size_t(known.owner)]==0);
        grid.knownBuilding(building->unit->id,float(mapSize_)-building->y,building->x,
            building->unit->collisionSize[0],allied,
            modifiedUnitAttribute(*building,12,building->unit->maxRange),
            modifiedUnitAttribute(*building,1,building->unit->lineOfSight));
    }
    //58939e consumes this even though the remainder is only retained in the
    // timed-search continuation record, never used by candidate scoring.
    if(placementRandomness>0)(void)(genieRandomDraw(legacyRandomState_)%uint32_t(placementRandomness));
    const int width=int(widthFloat),extent=searchRadius;
    struct RestorePlacementUi {
        Game &game;int player,failure;
        ~RestorePlacementUi(){game.localPlayer_=player;game.placementFailureCode_=failure;}
    } restore{*this,localPlayer_,placementFailureCode_};
    localPlayer_=player;
    const auto result=aiScanPrefabPlacement(mapSize_,mapSize_,
        std::max(0,originX-extent),std::max(0,originY-extent),
        std::min(mapSize_-1,originX+extent),std::min(mapSize_-1,originY+extent),
        originX,originY,width,[&](int gx,int gy){return grid.at(gx,gy);},
        [&]{return genieRandomDraw(legacyRandomState_);},
        [&](int ox,int oy) {
            const float offset=width%2?.5f:0.f;
            return aiBuildPositionRejected(player,unit.id,float(oy)+offset,
                float(mapSize_)-(float(ox)+offset));
        },
        [&](float ox,float oy) {
            // shortcut: Native placement validation uses the final snapped
            // center; the original master callback receives(x+.5,y+.5).
            const float offset=width%2?.5f:0.f;
            const float nx=float(int(oy))+offset,ny=float(mapSize_)-(float(int(ox))+offset);
            return placementValid(unit,nx,ny);
        });
    if(result.score<0)return 0;
    const float offset=width%2?.5f:0.f;
    x=float(result.y)+offset;y=float(mapSize_)-(float(result.x)+offset);
    return 1;
}
} // namespace swgb
