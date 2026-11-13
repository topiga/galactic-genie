// SPDX-License-Identifier: GPL-3.0-or-later
#include "rms_objects.h"
#include "rms_scenery_appearance.h"
#include "../core/genie_random.h"
#include "../core/genie_dat.h"
#include <algorithm>
#include <cmath>

namespace swgb {
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
    struct Spec { int id, graphic, type, cls, hp, restriction, angles, resource, amount, doppel; float height; };
    static constexpr Spec specs[]={
        {348,5282,10,31,50,11,12,1,100,2,2.f},
        {66,5134,30,29,0,8,10,3,800,1,1.f},
        {102,5232,30,30,0,8,10,2,350,1,1.f}};
    const Spec *match=nullptr;
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
} // namespace swgb
