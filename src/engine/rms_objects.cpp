// SPDX-License-Identifier: GPL-3.0-or-later
#include "rms_objects.h"
#include "rms_scenery_appearance.h"
#include "../core/genie_random.h"
#include "../core/genie_dat.h"
#include <algorithm>
#include <cmath>

namespace swgb {
namespace {
struct ResourceSpec { int id, graphic, type, cls, hp, restriction, angles, resource, amount, doppel; float height; };
const ResourceSpec *validateResource(const dat::DatFile &data,int16_t unitId,
                                    const std::vector<uint8_t> &terrain,std::string *err) {
    auto fail=[&](const char *reason)->const ResourceSpec* {
        if(err) *err=std::string("RMS resource: ")+reason;
        return nullptr;
    };
    static constexpr ResourceSpec specs[]={
        {348,5282,10,31,50,11,12,1,100,2,2.f},
        {66,5134,30,29,0,8,10,3,800,1,1.f},
        {102,5232,30,30,0,8,10,2,350,1,1.f}};
    const ResourceSpec *match=nullptr;
    for (const auto &candidate:specs) if (candidate.id==unitId) match=&candidate;
    if (!match) return fail("unsupported resource unit");
    const auto &spec=*match;
    if (data.civs.empty() || data.civs[0].units.size()<=static_cast<size_t>(unitId))
        return fail("missing Gaia resource prototype");
    const auto &u=data.civs[0].units[unitId];
    const auto *g=data.graphic(spec.graphic);
    if (!u.exists || u.id!=unitId || u.type!=spec.type || u.cls!=spec.cls ||
        u.standingGraphic[0]!=spec.graphic || u.standingGraphic[1]!=-1 || u.hitPoints!=spec.hp ||
        u.collisionSize[0]!=.5f || u.collisionSize[1]!=.5f || u.collisionSize[2]!=spec.height ||
        u.clearanceSize[0]!=.5f || u.clearanceSize[1]!=.5f ||
        u.placementSideTerrain[0]!=-1 || u.placementSideTerrain[1]!=-1 ||
        u.placementTerrain[0]!=-1 || u.placementTerrain[1]!=-1 ||
        u.terrainRestriction!=spec.restriction || u.hillMode || u.flyMode || u.canBeBuiltOn ||
        u.recyclable || u.lineOfSight!=0 || u.trainSound!=-1 ||
        u.obstructionType!=2 || u.obstructionClass!=1 || u.civilization ||
        u.enableAutoGather!=1 || !g || g->sequenceType!=6 || g->angleCount!=spec.angles)
        return fail("unsupported resource prototype or graphic");
    if (spec.type==30 && (u.createDoppelgangerOnDeath!=spec.doppel || u.speed!=0 ||
        u.trackingUnit!=-1 || u.trackingUnitMode!=0 || u.trackingUnitDensity!=0 ||
        g->frameCount!=1 || g->soundId!=-1 || !g->angleSounds.empty()))
        return fail("unsupported resource movement or sprite metadata");
    int resourceStores=0;
    for (const auto &storage:u.resourceStorages) {
        if (storage.flag || (storage.type>=0 && (storage.type!=spec.resource || storage.amount!=spec.amount)))
            return fail("unsupported resource storage");
        resourceStores+=storage.type==spec.resource;
    }
    if (resourceStores!=1) return fail("expected one resource storage");
    if (data.terrainRestrictions.size()<=static_cast<size_t>(spec.restriction))
        return fail("missing terrain restriction");
    const auto &restriction=data.terrainRestrictions[spec.restriction].passableBuildableDmgMultiplier;
    for (uint8_t tile : terrain)
        if (tile >= restriction.size() || !std::isfinite(restriction[tile]))
            return fail("invalid terrain restriction value");
    return match;
}
} // namespace

bool generateRmsObjectCandidates(uint32_t width,uint32_t height,uint32_t &randomState,
                                 std::vector<uint32_t> &out,std::string *err) {
    return generateRmsObjectCandidatesInArea(width,height,-1,-1,-1,randomState,out,err);
}

bool generateRmsObjectCandidatesInArea(uint32_t width,uint32_t height,
    int32_t centerX,int32_t centerY,int32_t outerRadius,uint32_t &randomState,
    std::vector<uint32_t> &out,std::string *err) {
    out.clear();
    if (err) err->clear();
    if (!width || !height || width>1024 || height>1024) {
        if (err) *err="RMS object candidates: dimensions must be 1..1024";
        return false;
    }
    uint32_t xmin=0,ymin=0,xmax=width-1,ymax=height-1;
    if (centerX>=0 && centerY>=0 && outerRadius>=0) {
        if (static_cast<uint32_t>(centerX)>=width || static_cast<uint32_t>(centerY)>=height ||
            outerRadius>1024) {
            if (err) *err="RMS object candidates: unsupported center or radius";
            return false;
        }
        xmin=static_cast<uint32_t>(std::max(0,centerX-outerRadius));
        ymin=static_cast<uint32_t>(std::max(0,centerY-outerRadius));
        xmax=static_cast<uint32_t>(std::min(static_cast<int32_t>(width)-1,centerX+outerRadius));
        ymax=static_cast<uint32_t>(std::min(static_cast<int32_t>(height)-1,centerY+outerRadius));
    }
    const uint32_t areaWidth=xmax-xmin+1,areaHeight=ymax-ymin+1;
    const uint32_t count=areaWidth*areaHeight, sentinel=count;
    std::vector<uint32_t> next(count+1),previous(count+1);
    for (uint32_t i=0;i<count;++i) {
        next[i]=i+1;
        previous[i]=i?i-1:sentinel;
    }
    next[sentinel]=0;
    previous[sentinel]=count-1;
    uint32_t rng=randomState;
    if (areaWidth>3 && areaHeight>3) {
        const uint32_t dx=areaWidth-2,dy=areaHeight-2;
        for (uint32_t i=0;i<dx*dy/4;++i) {
            const uint32_t x=genieRandomDraw(rng)*dx/32767u;
            const uint32_t y=genieRandomDraw(rng)*dy/32767u;
            const uint32_t tile=y*areaWidth+x;
            // Original unlink/insert-after-sentinel moves nodes; repeated draws do not duplicate them.
            next[previous[tile]]=next[tile];
            previous[next[tile]]=previous[tile];
            next[tile]=next[sentinel];
            previous[tile]=sentinel;
            previous[next[sentinel]]=tile;
            next[sentinel]=tile;
        }
    }
    out.reserve(count);
    for (uint32_t tile=next[sentinel];tile!=sentinel;tile=next[tile])
        out.push_back((ymin+tile/areaWidth)*width+xmin+tile%areaWidth);
    randomState=rng;
    return true;
}

bool generateRmsTimberPlacements(uint32_t width, uint32_t height,
    const std::vector<uint8_t> &terrain, const dat::DatFile &data,
    uint32_t requested, uint32_t &randomState,
    std::vector<RmsTimberPlacement> &out, std::string *err,
    const std::vector<uint32_t> &occupiedTiles) {
    return generateRmsResourcePlacements(width,height,terrain,data,348,requested,randomState,out,err,occupiedTiles);
}

bool generateRmsResourcePlacements(uint32_t width, uint32_t height,
    const std::vector<uint8_t> &terrain, const dat::DatFile &data, int16_t unitId,
    uint32_t requested, uint32_t &randomState,
    std::vector<RmsTimberPlacement> &out, std::string *err,
    const std::vector<uint32_t> &occupiedTiles) {
    out.clear();
    if (err) err->clear();
    auto fail = [&](const char *reason) {
        if (err) *err = std::string("RMS resource: ") + reason;
        return false;
    };
    if (!width || !height || width > 1024 || height > 1024 ||
        terrain.size() != static_cast<size_t>(width)*height || requested > terrain.size())
        return fail("invalid dimensions, terrain size or object count");
    std::vector<uint8_t> occupied(terrain.size(),0);
    for (uint32_t tile:occupiedTiles) {
        if (tile>=terrain.size()) return fail("occupied tile outside map");
        occupied[tile]=1;
    }
    const auto *match=validateResource(data,unitId,terrain,err);
    if(!match) return false;
    const auto &spec=*match;
    const auto &restriction=data.terrainRestrictions[spec.restriction].passableBuildableDmgMultiplier;
    uint32_t rng = randomState;
    std::vector<uint32_t> candidates;
    // The original builds its queue even when the requested count is zero.
    if (!generateRmsObjectCandidates(width,height,rng,candidates,err)) return false;
    out.reserve(requested);
    for (uint32_t tile : candidates) {
        if (out.size() == requested) break;
        if (occupied[tile]) continue;
        if (restriction[terrain[tile]] <= .05f) continue;
        // Distinct tile centers cannot strictly overlap these half-tile radii.
        if (spec.type==30) {
            const auto appearance=drawRmsCarbonAppearance(rng,static_cast<uint8_t>(spec.angles));
            out.push_back({float(tile%width)+.5f,float(tile/width)+.5f,appearance.facet,
                           appearance.headingRadians,unitId});
        } else {
            const auto facet=static_cast<uint8_t>(std::min(11u,genieRandomDraw(rng)*12u/32767u));
            out.push_back({float(tile%width)+.5f,float(tile/width)+.5f,facet,0,unitId});
        }
    }
    randomState = rng;
    return true;
}
bool generateRmsGroupedResourcePlacements(uint32_t width,uint32_t height,
    const std::vector<uint8_t> &terrain,const dat::DatFile &data,int16_t unitId,
    const RmsResourceGroups &group,uint32_t &randomState,
    std::vector<RmsTimberPlacement> &out,std::string *err,
    const std::vector<uint32_t> &occupiedTiles,size_t maxWork) {
    out.clear(); if(err) err->clear();
    auto fail=[&](const char *reason) {
        if(err) *err=std::string("RMS grouped resource: ")+reason;
        return false;
    };
    const size_t count=size_t(width)*height;
    if(!width || !height || width>1024 || height>1024 || terrain.size()!=count ||
       (group.mode!=1 && group.mode!=2) || group.groups>count || group.perGroup>count ||
       group.variance>1024 || group.radius>1024)
        return fail("unsupported dimensions or grouping parameters");
    const auto *spec=validateResource(data,unitId,terrain,err);
    if(!spec) return false;
    const auto &restriction=data.terrainRestrictions[spec->restriction].passableBuildableDmgMultiplier;
    std::vector<bool> occupied(count),eligible(count,true);
    for(uint32_t tile:occupiedTiles) {
        if(tile>=count) return fail("occupied tile outside map");
        occupied[tile]=true;
    }
    uint32_t rng=randomState;
    std::vector<RmsTimberPlacement> result;
    // Both original queues link the SAME map nodes. Moving a node into an inner
    // group removes it from the outer queue, including unused inner candidates.
    const int outer=static_cast<int>(count),inner=outer+1;
    std::vector<int> next(count+2,-1),previous(count+2,-1);
    std::vector<uint32_t> priority(count);
    auto unlink=[&](int tile) {
        if(previous[tile]>=0) next[previous[tile]]=next[tile];
        if(next[tile]>=0) previous[next[tile]]=previous[tile];
        previous[tile]=next[tile]=-1;
    };
    auto insertAfter=[&](int anchor,int tile) {
        unlink(tile);
        next[tile]=next[anchor];previous[tile]=anchor;
        if(next[anchor]>=0) previous[next[anchor]]=tile;
        next[anchor]=tile;
    };
    auto pop=[&](int queue) {
        const int tile=next[queue];
        if(tile>=0) unlink(tile);
        return tile;
    };
    auto spend=[&](size_t amount) {
        if(amount>maxWork) return false;
        maxWork-=amount;return true;
    };
    auto build=[&](int queue,int x,int y,int radius) {
        const size_t area=x<0 ? count :
            size_t(std::min(int(width)-1,x+radius)-std::max(0,x-radius)+1)*
            size_t(std::min(int(height)-1,y+radius)-std::max(0,y-radius)+1);
        if(!spend(area*2)) return false;
        std::vector<uint32_t> order;
        if(!generateRmsObjectCandidatesInArea(width,height,x,y,radius,rng,order,err)) return false;
        int tail=queue;
        // Removing ineligible nodes after the all-eligible shuffle is equivalent:
        // original draws still occur, but moves of an ineligible node are skipped.
        for(uint32_t tile:order) if(eligible[tile]) {
            insertAfter(tail,static_cast<int>(tile));tail=static_cast<int>(tile);
        }
        return true;
    };
    auto pass=[&](int tile) { return !occupied[tile] && restriction[terrain[tile]]>.05f; };
    auto create=[&](int tile) {
        RmsTimberPlacement placement{float(tile%width)+.5f,float(tile/width)+.5f,0,0,unitId};
        if(spec->type==30) {
            const auto appearance=drawRmsCarbonAppearance(rng,static_cast<uint8_t>(spec->angles));
            placement.facet=appearance.facet;placement.headingRadians=appearance.headingRadians;
        } else placement.facet=static_cast<uint8_t>(std::min(11u,genieRandomDraw(rng)*12u/32767u));
        result.push_back(placement);occupied[tile]=true;
    };
    if(!build(outer,-1,-1,-1)) return fail("work limit exceeded");
    uint32_t groupsLeft=group.groups;
    while(next[outer]>=0 && groupsLeft) {
        if(!spend(1)) return fail("work limit exceeded");
        const int center=pop(outer);
        if(!pass(center)) continue;
        const int x=center%width,y=center/width;
        const int xmin=std::max(0,x-int(group.radius)),xmax=std::min(int(width)-1,x+int(group.radius));
        const int ymin=std::max(0,y-int(group.radius)),ymax=std::min(int(height)-1,y+int(group.radius));
        if(group.mode==1) {
            if(!build(inner,x,y,static_cast<int>(group.radius))) return fail("work limit exceeded");
        } else insertAfter(inner,center);
        // Even zero variance consumes a draw; zero/negative group sizes become one.
        int remaining=std::max(1,int(group.perGroup)+int(genieRandomDraw(rng)*2u*group.variance/32767u)-int(group.variance));
        while(next[inner]>=0 && remaining>0) {
            if(!spend(1)) return fail("work limit exceeded");
            const int tile=pop(inner);
            if(group.mode==1 && !pass(tile)) continue;
            create(tile);--remaining;
            if(group.mode==2) {
                eligible[tile]=false;
                const int tx=tile%width,ty=tile/width;
                const int neighbors[]={tx>xmin?tile-1:-1,tx<xmax?tile+1:-1,
                                       ty>ymin?tile-int(width):-1,ty<ymax?tile+int(width):-1};
                // Original still tests/enqueues neighbors after the final creation.
                for(int neighbor:neighbors) if(neighbor>=0) {
                    if(!spend(1)) return fail("work limit exceeded");
                    if(!pass(neighbor)) continue;
                    const uint32_t score=genieRandomDraw(rng);
                    unlink(neighbor);
                    int anchor=inner;
                    while(next[anchor]>=0 && priority[next[anchor]]<score) {
                        if(!spend(1)) return fail("work limit exceeded");
                        anchor=next[anchor];
                    }
                    priority[neighbor]=score;insertAfter(anchor,neighbor);
                }
            }
        }
        while(next[inner]>=0) {
            if(!spend(1)) return fail("work limit exceeded");
            pop(inner);
        }
        --groupsLeft;
    }
    out=std::move(result);randomState=rng;
    return true;
}
} // namespace swgb
