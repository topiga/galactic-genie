// SPDX-License-Identifier: GPL-3.0-or-later
#include "game.h"
#include "ai_unit_line.h"
#include "../core/genie_random.h"
#include "ai_prefab_grid.h"
#include "ai_build_placement.h"
#include <algorithm>
#include <cmath>

namespace swgb {
void Game::initializeAiWallPlans(const Scenario &scenario) {
    if (mapSize_!=120 || assets_.dat().terrainRestrictions.size()<=25) return;
    RmsWallInput input;
    input.width=input.height=mapSize_;
    input.restrictions=assets_.dat().terrainRestrictions[25].passableBuildableDmgMultiplier;
    for (const auto &tile:scenario.map.tiles) input.terrain.push_back(tile.terrain);
    for (const auto &object:objects_) {
        if (!object.active || !object.unit) continue;
        const auto &unit=*object.unit;
        const float x=float(mapSize_)-object.y,y=object.x;
        const float rx=unit.collisionSize[1],ry=unit.collisionSize[0];
        input.objects.push_back({object.player,x,y,rx,ry,unit.obstructionType,unit.obstructionClass,unit.cls});
        if (unit.id==109) input.commandCenters.push_back({object.player,x,y,rx,ry});
    }
    // 5813f0 requests inner and outer plans. Only the first distance pair of each search is verified.
    for (int plan=0;plan<2;++plan) {
        input.minimumDistance=plan==0?10:18;
        input.maximumDistance=plan==0?20:30;
        RmsWallBoundary boundary;
        if (!generateRmsWallBoundary(input,boundary)) continue;
        for (auto &segments:boundary.segments) {
            if (std::none_of(segments.begin(),segments.end(),[](const auto &s){return s.active && s.length>3;}))
                segments.clear();
        }
        auto &owners=aiWallPlans_[plan];
        owners.resize(boundary.segments.size());
        for(size_t owner=0;owner<owners.size();++owner)
            for(const auto &geometry:boundary.segments[owner])
                owners[owner].push_back({geometry,-1,geometry.length,0,false});
    }
}

//1=found,0=supported search found no location,-1=unsupported provider context.
int Game::chooseAiOriginalBuildPosition(int player,const dat::Unit &unit,float &x,float &y,
                                 std::string *unsupportedReason) {
    if(unsupportedReason)unsupportedReason->clear();
    const auto unsupported=[&](const char *reason) {
        if(unsupportedReason)*unsupportedReason=reason;
        return -1;
    };
    const int category=originalBuildingPlacementCategory(unit.id);
    const bool depot=category==1 || category==2 || category==11 || category==13;
    if((category!=8 && category!=10 && !depot) || unit.type!=dat::UT_Building || player<=0 ||
       size_t(player)>=aiPlayers_.size() || mapSize_<=0 || mapSize_>255)
        return unsupported("normal Clone Campaigns prefab, farm or resource depot profile required");
    auto &state=aiPlayers_[size_t(player)];
    const auto radius=state.strategicNumbers.find(category==2?87:depot?86:74),randomness=state.strategicNumbers.find(168);
    // Actual596d10/597300 defaults: SN74=20, SN86=25, SN87=100, SN168=50.
    const int searchRadius=radius==state.strategicNumbers.end()?(category==2?100:depot?25:20):radius->second;
    const int placementRandomness=randomness==state.strategicNumbers.end()?50:randomness->second;
    if(searchRadius<0)
        return unsupported("placement strategic-number context unavailable");
    const float widthFloat=unit.collisionSize[0]*2.f;
    if(!std::isfinite(widthFloat) || widthFloat<1 || widthFloat>mapSize_ ||
       widthFloat!=std::floor(widthFloat) || unit.collisionSize[0]!=unit.collisionSize[1])
        return unsupported("non-square building footprint");
    // Recent attack rectangles still require the placement fallback before RNG.
    for(const auto &quarter:state.explorationQuarters)
        if(quarter.received || quarter.issued)
            return unsupported("attack-history placement rectangles are not represented");
    if(wallDragActive_ && localPlayer_==player)
        return unsupported("pending wall placement is not represented");
    refreshAiEconomyKnowledge(player);
    auto &economy=state.economy;
    for(int32_t id:economy.depotIds) {
        const auto *building=findObject(uint32_t(id));
        if(aiWallPlans_[0].empty() && aiWallPlans_[1].empty() && building && building->unit && (building->unit->cls==6 || building->unit->cls==8))
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
    if(const auto preset=state.buildPositions.consumePreset(unit.id)) {
        // Original stored coordinates return at+.5; use the same explicitly
        // native final-centre snapping as the fresh scanner below.
        const float offset=int(widthFloat)%2?.5f:0.f;
        x=float(preset->y)+offset;y=float(mapSize_)-(float(preset->x)+offset);
        return 1;
    }
    AiBuildPlacementGrid grid(mapSize_,category);
    const bool hasCompletedFoodDepot=category==2 && aiPrototypeCount(player,68,false)>0;
    //58b100 scans remembered resource buckets. Lookup uses the live object just
    // as413850 does; absence/record.invalid=1 skip it. No new hidden knowledge.
    for(size_t bucket=0;bucket<economy.resources.size();++bucket)
        for(const auto &record:economy.resources[bucket].records) {
            const auto *resource=findObject(uint32_t(record.id));
            if(!resource || !resource->active || !resource->unit)continue;
            grid.resource(int(bucket),resource->unit->cls,float(mapSize_)-resource->y,resource->x,
                          resource->unit->collisionSize[0],resource->unit->collisionSize[1],record.invalid==1,hasCompletedFoodDepot);
        }
    // 4b0ce0 sets bit 0x4000 on the registered static footprint.
    for(uint32_t index:staticObstructionIndices_) {
        const auto &object=objects_[index];
        if(!object.active || object.hidden || object.garrisonedInId>=0)continue;
        const auto &master=*object.unit;
        if(master.obstructionType!=1 && master.obstructionType!=2)continue;
        if(master.obstructionClass==0)continue;
        const auto bounds=pathMapFootprint(float(mapSize_)-object.y,object.x,
                                           master.collisionSize[1],master.collisionSize[0]);
        if(bounds.left<bounds.right && bounds.top<bounds.bottom && bounds.right>0 &&
           bounds.bottom>0 && bounds.left<mapSize_ && bounds.top<mapSize_)
            grid.forbid({bounds.left,bounds.top,bounds.right-1,bounds.bottom-1});
    }
    for(int oy=0;oy<mapSize_;++oy)for(int ox=0;ox<mapSize_;++ox) {
        const int nx=oy,ny=mapSize_-1-ox;
        const size_t tile=size_t(ny)*mapSize_+nx;
        bool occupied=false;
        // shortcut: retain native coverage for terrain-changing obstructions until their callbacks are ported.
        if(tile<staticObstructionCells_.size())for(uint32_t index:staticObstructionCells_[tile]) {
            const auto &object=objects_[index];
            if(object.active && !object.hidden && object.garrisonedInId<0 && object.unit->obstructionType>2 &&
               object.unit->obstructionType!=5) {occupied=true;break;}
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
    for (size_t plan=0;plan<aiWallPlans_.size();++plan) {
        if (!aiWallPlanEnabled_[player][plan]) continue;
        for (const auto &segment:aiWallPlans_[plan][player])
            grid.wall(segment.fromX,segment.fromY,segment.toX,segment.toY);
    }
    //58939e consumes this even though the remainder is only retained in the
    // timed-search continuation record, never used by candidate scoring.
    if(placementRandomness>0)(void)(genieRandomDraw(legacyRandomState_)%uint32_t(placementRandomness));
    const int width=int(widthFloat),extent=std::min(searchRadius,mapSize_);
    struct RestorePlacementUi {
        Game &game;int player,failure;
        ~RestorePlacementUi(){game.localPlayer_=player;game.placementFailureCode_=failure;}
    } restore{*this,localPlayer_,placementFailureCode_};
    localPlayer_=player;
    const auto result=aiScanBuildingPlacement(category,mapSize_,mapSize_,
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
        },[&](int ox,int oy,int radius) {
            return state.buildPositions.plannedNearby(unit.id,ox,oy,radius);
        });
    if(result.score<0)return 0;
    const float offset=width%2?.5f:0.f;
    x=float(result.y)+offset;y=float(mapSize_)-(float(result.x)+offset);
    return 1;
}

std::vector<Game::AiWallSegment> *Game::aiWallSegments(int player,int plan) {
    if(plan<1 || plan>2 || player<=0 || size_t(player)>=aiWallPlanEnabled_.size() ||
       !aiWallPlanEnabled_[player][plan-1])return nullptr;
    auto &owners=aiWallPlans_[plan-1];
    return size_t(player)<owners.size()?&owners[player]:nullptr;
}

void Game::surveyAiWallSegment(int player) {
    auto &cursor=aiPlayers_[player].wallSurveyIndex;
    auto *inner=aiWallSegments(player,1),*outer=aiWallSegments(player,2);
    const size_t n=inner?inner->size():0,total=n+(outer?outer->size():0);
    if(!total)return;
    cursor%=total;
    auto &s=cursor<n?(*inner)[cursor]:(*outer)[cursor-n];
    cursor=(cursor+1)%total;
    const auto *unit=findUnit(civilizationForPlayer(player),117);
    if(!unit)return;
    const int dx=(s.toX>s.fromX)-(s.toX<s.fromX),dy=(s.toY>s.fromY)-(s.toY<s.fromY);
    s.unexplored=s.buildable=0;
    Object probe;probe.player=player;probe.unit=unit;
    const auto sight=sharedSightMask(player);
    for(int i=0;i<s.length;++i) {
        const int x=s.fromY+i*dy,y=mapSize_-1-(s.fromX+i*dx);
        if(x<0 || y<0 || x>=mapSize_ || y>=mapSize_)continue;
        const size_t tile=size_t(y)*mapSize_+x;
        // shortcut: Native exploration and obstruction supply the original master placement query.
        if(tile>=observedTiles_.size() || !(observedTiles_[tile]&sight))++s.unexplored;
        else if(positionPassable(probe,x+.5f,y+.5f,true))++s.buildable;
    }
    if(s.buildable)s.gateReserved=false; // 58a39d releases a gate request on a newly buildable segment.
}

const dat::Unit *Game::aiWallUnit(int player,const std::string &symbol) const {
    const auto &program=aiPlayers_[player].program;
    if(program.constant(symbol)!=-2)return aiUnit(player,symbol);
    const auto &lines=assets_.dat().unitLines;
    if(lines.size()<=2)return nullptr;
    const int id=aiUnitLineSelection(lines[2].unitIds,true,[&](int member){return aiBaseUnitEnabled(player,member);});
    return effectiveUnitForPlayer(player,findUnit(civilizationForPlayer(player),id));
}

bool Game::aiWallRequest(int player,int plan,const dat::Unit *unit,AiWallRequest request,bool escrow,bool execute) {
    auto *segments=aiWallSegments(player,plan);
    if(!unit || plan<1 || plan>2 || !aiBaseUnitEnabled(player,unit->id))return false;
    const bool gate=request==AiWallRequest::Gate;
    if(!gate && unit->id!=72 && unit->id!=117 && unit->id!=155 && unit->id!=195)return false;
    if(request==AiWallRequest::CompletePlan) {
        int tiles=0;
        if(segments)for(const auto &s:*segments)tiles+=s.unexplored+s.buildable;
        return aiCanAfford(player,*unit,escrow,tiles);
    }
    if(!segments)return false;
    const int longSegments=gate?std::count_if(segments->begin(),segments->end(),[](const auto &s) {
        return s.active && s.length>3;
    }):0;
    for(auto &s:*segments) {
        if(gate) {
            if(s.wallMaster<0 || s.gateReserved || s.length<(longSegments>3?4:3))continue;
        } else if(s.unexplored || !s.buildable)continue;
        // 575e40 dispatches a wall line without repeating the fact's whole-segment credit check.
        if((!execute || gate) && !aiCanAfford(player,*unit,escrow,gate?1:s.buildable))return false;
        if(execute) {
            if(gate)s.gateReserved=true;
            else {s.wallMaster=unit->id;s.buildable=0;}
            localCommands_.enqueue([this,player,s,id=unit->id,gate]{executeAiWall(player,s,id,gate);});
        }
        return true;
    }
    return false;
}

void Game::executeAiWall(int player,const AiWallSegment &s,int prototype,bool gate) {
    const auto *price=findUnit(civilizationForPlayer(player),prototype);
    if(!price)return;
    int id=prototype;
    float x=s.fromX+.5f,y=s.fromY+.5f;
    const int dx=(s.toX>s.fromX)-(s.toX<s.fromX),dy=(s.toY>s.fromY)-(s.toY<s.fromY);
    if(gate) {
        const bool even=s.length%2==0;
        x=(s.fromX+s.toX)*.5f;y=(s.fromY+s.toY)*.5f;
        if(!dx){id=490;x+=.5f;if(even)y+=.5f;}
        else if(!dy){id=487;y+=.5f;if(even)x+=.5f;}
        else {id=dy>0?665:673;if(even){x+=.5f;y+=.5f;}else if(dy<0)x+=1.f;}
    }
    const auto *unit=findUnit(civilizationForPlayer(player),id);
    if(!unit)return;
    struct Restore {Game &game;int owner,failure;~Restore(){game.localPlayer_=owner;game.placementFailureCode_=failure;}}
        restore{*this,localPlayer_,placementFailureCode_};
    localPlayer_=player;
    const auto costs=unitCosts(player,*price);
    for(int i=0;i<(gate?1:s.length);++i) {
        const float wx=y+i*dy,wy=mapSize_-(x+i*dx);
        std::vector<Object *> replaced;
        if(!placementValid(*unit,wx,wy,&replaced))continue;
        for(const auto &cost:costs)
            if(cost.flag && cost.type>=0 && cost.amount>resource(player,cost.type))return;
        std::vector<uint32_t> wallIds;
        for(const auto *wall:replaced)wallIds.push_back(wall->spawnId);
        auto *building=createFoundation(*unit,player,wx,wy);
        if(!building)continue;
        for(auto wallId:wallIds)if(auto *wall=findObject(wallId)) {
            wall->active=false;wall->selected=false;
        }
        for(const auto &cost:costs)if(cost.flag && cost.type>=0 && cost.amount>0)
            resources_[player][cost.type]-=cost.amount;
        rebuildAdjacency();
    }
}
} // namespace swgb
