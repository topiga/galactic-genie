// SPDX-License-Identifier: GPL-3.0-or-later
#include "rms_objects.h"
#include "rms_zones.h"
#include "rms_scenery_appearance.h"
#include "../core/genie_random.h"
#include "../core/genie_dat.h"
#include <algorithm>
#include <cmath>

namespace swgb {
namespace {
bool isTree(int unit) { return unit==348 || unit==349 || unit==413; }
bool isPredator(int unit) { return unit==126 || unit==812 || unit==1197; }
struct ResourceSpec { int id, graphic, type, cls, hp, restriction, angles, resource, amount, doppel; float height; };
const ResourceSpec *validateResource(const dat::DatFile &data,int16_t unitId,
                                    const std::vector<uint8_t> &terrain,std::string *err) {
    auto fail=[&](const char *reason)->const ResourceSpec* {
        if(err) *err=std::string("RMS resource: ")+reason;
        return nullptr;
    };
    static constexpr ResourceSpec specs[]={
        {348,5282,10,31,50,11,12,1,100,2,2.f},
        {349,5289,10,31,50,11,11,1,100,2,2.f},
        {413,5301,10,31,50,11,12,1,125,2,2.f},
        {59,5192,10,27,0,8,6,0,125,1,1.f},
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
const ResourceSpec *validateHerd(const dat::DatFile &data,int16_t unitId,const std::vector<uint8_t> &terrain,std::string *err) {
    static constexpr ResourceSpec nerf{594,61,70,1,7,7,8,0,200,0,2.f};
    static constexpr ResourceSpec kaadu{1363,46,70,4,5,1,8,0,100,0,2.f};
    static constexpr ResourceSpec eopie{1365,22,70,4,5,1,8,0,140,0,2.f};
    static constexpr ResourceSpec rancor{126,70,70,5,150,26,8,-1,0,0,2.f};
    const bool bigHunt=unitId==822 || unitId==1249;
    const bool predator=isPredator(unitId);
    static constexpr ResourceSpec predator812{812,84,70,5,40,1,8,-1,0,0,2.f};
    static constexpr ResourceSpec predator1197{1197,36,70,5,75,1,8,-1,0,0,2.f};
    static constexpr ResourceSpec hunt822{822,31,70,60,50,1,8,0,300,0,2.f};
    static constexpr ResourceSpec hunt1249{1249,17,70,60,75,26,8,0,340,0,2.f};
    const auto &spec=unitId==822?hunt822:unitId==1249?hunt1249:unitId==812?predator812:unitId==1197?predator1197:predator?rancor:unitId==1363?kaadu:unitId==1365?eopie:nerf;
    const bool hunt=unitId==1363 || unitId==1365;
    auto fail=[&]()->const ResourceSpec* {if(err)*err="RMS herd: unsupported original animal metadata";return nullptr;};
    if(data.civs.empty() || data.civs[0].units.size()<=size_t(unitId) || data.terrainRestrictions.size()<=size_t(spec.restriction)) return fail();
    const auto &u=data.civs[0].units[unitId];const auto *g=data.graphic(spec.graphic);
    if(!u.exists || u.id!=unitId || u.type!=70 || u.cls!=spec.cls || u.hitPoints!=spec.hp || u.standingGraphic[0]!=spec.graphic || u.standingGraphic[1]!=-1 ||
       u.collisionSize[0]!=(bigHunt?.5f:predator?.25f:.3f) || u.collisionSize[1]!=(bigHunt?.5f:predator?.25f:.3f) || u.collisionSize[2]!=2.f || u.clearanceSize[0]!=(bigHunt?1.f:predator?.5f:.3f) || u.clearanceSize[1]!=(bigHunt?1.f:predator?.5f:.3f) ||
       u.lineOfSight!=(bigHunt?4.f:unitId==812?6.f:unitId==1197?10.f:predator?7.f:2.f) || u.speed!=(bigHunt?.9f:unitId==812?.9f:unitId==1197?.85f:hunt?.74f:.7f) || u.terrainRestriction!=spec.restriction || u.obstructionType!=5 || u.obstructionClass!=2 ||
       u.hillMode || u.flyMode || u.canBeBuiltOn || u.recyclable || u.civilization || u.enableAutoGather || u.createDoppelgangerOnDeath ||
       u.placementSideTerrain[0]!=-1 || u.placementSideTerrain[1]!=-1 || u.placementTerrain[0]!=-1 || u.placementTerrain[1]!=-1 ||
       u.trainSound!=(unitId==822?612:unitId==1249?562:unitId==812?642:unitId==1197?639:predator?37:unitId==1363?421:unitId==1365?609:630) || !g || g->sequenceType!=7 || g->frameCount!=(bigHunt?10:unitId==1363?11:unitId==1365?12:8) || g->angleCount!=8 || g->frameDuration!=((unitId==812 || unitId==1197)?.1f:(hunt || predator || bigHunt)?.12f:.275f) ||
       g->replayDelay!=((unitId==812 || unitId==1197)?1.f:(hunt || predator || bigHunt)?.5f:0.f) || g->soundId!=-1 || !g->angleSounds.empty()) return fail();
    int stores=0;for(const auto &storage:u.resourceStorages) {
        if(storage.flag || (storage.type>=0 && (predator || storage.type!=0 || storage.amount!=spec.amount))) return fail();
        stores+=storage.type==0;
    }
    if(stores!=(predator?0:1)) return fail();
    const auto &restriction=data.terrainRestrictions[spec.restriction].passableBuildableDmgMultiplier;
    for(auto t:terrain) if(t>=restriction.size() || !std::isfinite(restriction[t])) return fail();
    return &spec;
}
const ResourceSpec *validateBird(const dat::DatFile &data,const std::vector<uint8_t> &terrain,std::string *err) {
    static constexpr ResourceSpec bird{816,6,70,20,5,23,8,-1,0,0,0};
    auto fail=[&]()->const ResourceSpec* {if(err)*err="RMS bird: unsupported original metadata";return nullptr;};
    if(data.civs.empty() || data.civs[0].units.size()<=816 || data.terrainRestrictions.size()<=23) return fail();
    const auto &u=data.civs[0].units[816];const auto *g=data.graphic(6);
    if(!u.exists || u.id!=816 || u.type!=70 || u.cls!=20 || u.hitPoints!=5 || u.standingGraphic[0]!=6 || u.standingGraphic[1]!=-1 ||
       u.collisionSize[0]!=.1f || u.collisionSize[1]!=.1f || u.collisionSize[2]!=0 || u.clearanceSize[0]!=0 || u.clearanceSize[1]!=0 ||
       u.lineOfSight!=5 || u.speed!=1.32f || u.terrainRestriction!=23 || u.obstructionType!=5 || u.obstructionClass!=2 ||
       u.hillMode || u.flyMode!=1 || u.canBeBuiltOn || u.recyclable || u.civilization || u.enableAutoGather || u.createDoppelgangerOnDeath ||
       u.placementSideTerrain[0]!=-1 || u.placementSideTerrain[1]!=-1 || u.placementTerrain[0]!=-1 || u.placementTerrain[1]!=-1 ||
       u.trainSound!=618 || !g || g->sequenceType!=3 || g->frameCount!=8 || g->angleCount!=8 || g->frameDuration!=.1265f ||
       g->replayDelay!=0 || g->soundId!=-1 || !g->angleSounds.empty()) return fail();
    for(const auto &storage:u.resourceStorages) if(storage.flag || storage.type>=0) return fail();
    const auto &restriction=data.terrainRestrictions[23].passableBuildableDmgMultiplier;
    for(auto t:terrain) if(t>=restriction.size() || restriction[t]!=1.f) return fail();
    return &bird;
}
const ResourceSpec *validateAquatic(const dat::DatFile &data,int16_t unitId,const std::vector<uint8_t> &terrain,std::string *err) {
    static constexpr ResourceSpec fish{69,5268,30,25,0,9,1,17,200,1,0};
    static constexpr ResourceSpec wave{1585,10295,30,42,1000,31,1,-1,0,0,0};
    const auto &spec=unitId==69?fish:wave;
    auto fail=[&]()->const ResourceSpec* {if(err)*err="RMS aquatic: unsupported original profile";return nullptr;};
    if(data.civs.empty() || data.civs[0].units.size()<=size_t(unitId) || data.terrainRestrictions.size()<=size_t(spec.restriction))return fail();
    const auto &u=data.civs[0].units[unitId];const auto *g=data.graphic(spec.graphic);
    if(!u.exists || u.id!=unitId || u.type!=30 || u.cls!=spec.cls || u.hitPoints!=spec.hp || u.standingGraphic[0]!=spec.graphic || u.standingGraphic[1]!=-1 ||
       u.collisionSize[0]!=.5f || u.collisionSize[1]!=.5f || u.collisionSize[2]!=0 || u.clearanceSize[0]!=.5f || u.clearanceSize[1]!=.5f ||
       u.lineOfSight!=0 || u.speed!=(unitId==69?1.32f:0.f) || u.terrainRestriction!=spec.restriction || u.obstructionType || u.obstructionClass ||
       u.hillMode || u.flyMode || u.canBeBuiltOn || u.recyclable || u.civilization || u.enableAutoGather!=(unitId==69) || u.createDoppelgangerOnDeath!=spec.doppel ||
       u.placementSideTerrain[0]!=(unitId==69?2:-1) || u.placementSideTerrain[1]!=(unitId==69?35:-1) || u.placementTerrain[0]!=-1 || u.placementTerrain[1]!=-1 ||
       u.trainSound!=-1 || !g || g->sequenceType!=7 || g->frameCount!=(unitId==69?12:48) || g->angleCount!=1 || g->frameDuration!=.13f ||
       g->replayDelay!=0 || g->soundId!=-1 || !g->angleSounds.empty())return fail();
    int stores=0;for(const auto &storage:u.resourceStorages){if(storage.flag || (storage.type>=0 && (storage.type!=spec.resource || storage.amount!=spec.amount)))return fail();stores+=storage.type>=0;}
    if(stores!=(unitId==69?1:0))return fail();
    const auto &restriction=data.terrainRestrictions[spec.restriction].passableBuildableDmgMultiplier;
    for(auto t:terrain)if(t>=restriction.size() || !std::isfinite(restriction[t]))return fail();
    return &spec;
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
    if(unitId==59 || unitId==349 || unitId==413) return fail("resource requires the verified grouped row");
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
            const auto facet=static_cast<uint8_t>(std::min(unsigned(spec.angles-1),genieRandomDraw(rng)*unsigned(spec.angles)/32767u));
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
    const std::vector<uint32_t> &occupiedTiles,size_t maxWork,
    const std::vector<RmsPlayerStart> &novaPlayerStarts,RmsResourceDistance distance,
    std::vector<uint8_t> *rowEligibility,const RmsGlobalGroupPlacement *globalPlacement) {
    out.clear(); if(err) err->clear();
    auto fail=[&](const char *reason) {
        if(err) *err=std::string("RMS grouped resource: ")+reason;
        return false;
    };
    const size_t count=size_t(width)*height;
    if(!width || !height || width>1024 || height>1024 || terrain.size()!=count ||
       (group.mode!=1 && group.mode!=2 && !((isPredator(unitId) || unitId==822 || unitId==1249 || isTree(unitId) || unitId==816 || unitId==69 || unitId==1585) && group.mode==0)) || group.groups>count || group.perGroup>count ||
       group.variance>1024 || group.radius>1024)
        return fail("unsupported dimensions or grouping parameters");
    const bool hunt=unitId==1363 || unitId==1365;
    const bool predator=isPredator(unitId);
    if((unitId==349 || unitId==413) && group.mode!=0)return fail("tree species requires verified mode0 rows");
    const bool tree=isTree(unitId) && group.mode==0,bird=unitId==816,aquatic=unitId==69 || unitId==1585;
    const bool bigHunt=unitId==822 || unitId==1249;
    const bool herd=unitId==594 || hunt || predator || bigHunt;
    const auto *spec=aquatic?validateAquatic(data,unitId,terrain,err):bird?validateBird(data,terrain,err):herd?validateHerd(data,unitId,terrain,err):validateResource(data,unitId,terrain,err);
    if(!spec) return false;
    const auto &restriction=data.terrainRestrictions[spec->restriction].passableBuildableDmgMultiplier;
    const bool perPlayer=!novaPlayerStarts.empty();
    if(aquatic && (width!=120 || height!=120 || perPlayer || globalPlacement || distance!=RmsResourceDistance::Near ||
       group.mode!=0 || group.groups!=(unitId==69?4u:8u) || group.perGroup!=1 || group.variance || group.radius!=3))
        return fail("unsupported stock aquatic row");
    if(bird && (width!=120 || height!=120 || perPlayer || globalPlacement || distance!=RmsResourceDistance::Near ||
       group.mode!=0 || group.groups!=8 || group.perGroup!=1 || group.variance || group.radius!=3))
        return fail("unsupported stock bird row");
    if(globalPlacement) {
        if(perPlayer || (!predator && !tree && ((unitId!=66 && unitId!=102) || group.mode!=2)) ||
           (predator && (width!=120 || height!=120 || group.mode!=1 || group.groups!=4 || (group.perGroup!=1 && !((unitId==126 || unitId==1197) && group.perGroup==2)) || group.variance || group.radius!=3 ||
            globalPlacement->landCenters.size()!=2 || globalPlacement->minDistance!=40 || globalPlacement->spacing!=12)) ||
           (tree && (width!=120 || height!=120 || group.mode!=0 || group.groups!=100 || group.perGroup!=1 || group.variance || group.radius!=3 ||
            globalPlacement->landCenters.size()!=2 || globalPlacement->minDistance!=6 || globalPlacement->spacing!=2)) ||
           distance!=RmsResourceDistance::Near || globalPlacement->minDistance<0 ||
           globalPlacement->minDistance>1024 || globalPlacement->spacing<0 || globalPlacement->spacing>1024 ||
           globalPlacement->landCenters.size()>1024)
            return fail("unsupported global group placement");
        for(const auto &land:globalPlacement->landCenters)
            if(land.x<0 || land.y<0 || uint32_t(land.x)>=width || uint32_t(land.y)>=height)
                return fail("global land center outside map");
    }
    const bool forage=unitId==59,ore=unitId==102 && perPlayer;
    const bool medium=distance==RmsResourceDistance::Medium,far=distance==RmsResourceDistance::Far;
    if(distance!=RmsResourceDistance::Near && !medium && !far) return fail("invalid resource distance profile");
    if((medium || far) && (!perPlayer || (unitId!=66 && !((ore || herd || tree) && far)) || width!=120 || height!=120 || novaPlayerStarts.size()!=2))
        return fail("distance profile requires verified120-square two-player resource row");
    if(bigHunt && (medium || (far && group.mode!=1) || (group.mode!=0 && group.mode!=1) ||
       (group.mode==0 && (group.groups<1 || group.groups>(unitId==822?1u:2u))))) return fail("unsupported large-hunt row");
    if((hunt || predator) && !far && !globalPlacement) return fail("hunting animals require the verified far row");
    const int playerMin=globalPlacement?globalPlacement->minDistance:tree?(far?6:4):predator?30:bigHunt?(far?35:16):herd?(far?14:10):forage?10:ore?(far?20:14):medium?18:far?25:12;
    const int playerMax=tree?(far?8:5):predator?40:bigHunt?(far?-1:22):herd?(far?30:12):forage?12:ore?(far?26:18):medium?26:far?35:16,playerSpacing=globalPlacement?globalPlacement->spacing:perPlayer?(tree?2:bigHunt && far?5:herd?0:forage?6:7):0;
    const auto &exclusionCenters=globalPlacement?globalPlacement->landCenters:novaPlayerStarts;
    if((forage || ore || herd || tree) && !globalPlacement && (!perPlayer || width!=120 || height!=120 || novaPlayerStarts.size()!=2))
        return fail("resource row requires verified120-square two-player profile");
    if((forage || ore || herd || tree || medium || far) && std::any_of(terrain.begin(),terrain.end(),[&](uint8_t t) {
        const bool forest=width==120 && (t==1 || t==2 || t==5 || t==12 || t==18 || t==20 || t==21 || t==22);
        return !forest && t!=0 && t!=6 && t!=9 && t!=10 && t!=27 && t!=41;
    })) return fail("resource row requires verified grass/forest terrain");
    if(perPlayer) {
        const uint32_t expectedGroups=bigHunt?(group.mode==0?group.groups:1u):tree?(far?3u:2u):herd && far && !hunt?2u:1u;
        const uint32_t expectedCount=bigHunt?(far?3u:group.mode==0?1u:unitId==822?4u:2u):(tree || predator)?1u:hunt?4u:herd?(far?2u:4u):forage?6u:ore?(far?4u:5u):(medium || far)?4u:7u;
        if((unitId!=66 && !forage && !ore && !herd && !tree) || (!bigHunt && group.mode!=((tree || predator)?0:herd?1:2)) ||
           group.groups!=expectedGroups || (group.perGroup!=expectedCount && !(forage && (group.perGroup==3 || group.perGroup==12)) && !(unitId==822 && group.mode==1 && ((!far && (group.perGroup==1 || group.perGroup==2)) || (far && group.perGroup==6))) && !(hunt && group.perGroup==8) && !(hunt && group.perGroup==3) && !(unitId==1365 && group.perGroup==2) && !(unitId==1249 && group.perGroup==1) && !(unitId==594 && (group.perGroup==expectedCount/2 || group.perGroup==expectedCount*2))) ||
           group.variance!=((hunt || (bigHunt && far))?1u:0u) || group.radius!=(ore?2u:3u) || novaPlayerStarts.size()>8)
            return fail("unsupported per-player resource row");
        for(auto t:terrain) if(restriction[t]!=1.f && (width!=120 || (t!=1 && t!=2 && t!=5 && t!=6 && t!=12 && t!=18 && t!=20 && t!=21 && t!=22)))
            return fail("resource row has unverified nonuniform passability");
        for(const auto &start:novaPlayerStarts)
            if(start.x<0 || start.y<0 || uint32_t(start.x)>=width || uint32_t(start.y)>=height)
                return fail("resource player start outside map");
    }
    std::vector<bool> occupied(count),eligible(count,true);
    if(rowEligibility && !rowEligibility->empty()) {
        if(rowEligibility->size()!=count || std::any_of(rowEligibility->begin(),rowEligibility->end(),[](uint8_t v){return v>1;}))
            return fail("invalid persistent row eligibility");
        for(size_t i=0;i<count;++i) eligible[i]=(*rowEligibility)[i]!=0;
    }
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
    auto pass=[&](int tile) {
        if(unitId==1585 && terrain[tile]!=22)return false;
        if(unitId==69) {
            const int x=tile%width,y=tile/width;
            auto shore=[&](int at){return terrain[at]==2 || terrain[at]==35;};
            if(!((x>0 && shore(tile-1)) || (x+1<int(width) && shore(tile+1)) ||
                 (y>0 && shore(tile-int(width))) || (y+1<int(height) && shore(tile+int(width)))))return false;
        }
        if(bigHunt) {
            const int x=tile%width,y=tile/width;
            if(x==0 || y==0 || x+1==int(width) || y+1==int(height))return false;
            for(int yy=y-1;yy<=y+1;++yy)
                for(int xx=x-1;xx<=x+1;++xx)
                    if(occupied[yy*width+xx] || restriction[terrain[yy*width+xx]]<=.05f)return false;
        }
        return (bird || !occupied[tile]) && restriction[terrain[tile]]>.05f;
    };
    auto create=[&](int tile) {
        RmsTimberPlacement placement{float(tile%width)+.5f,float(tile/width)+.5f,0,0,unitId};
        if(bird) {
            const uint32_t heading=genieRandomDraw(rng)*8u/32767u;
            placement.headingRadians=float(double(heading)*3.14159265358979323846/4.);
            placement.facet=uint8_t((heading+7)%8);
        } else if(herd || aquatic) {
            // Base facet, animated initial frame/phase, heading, then three action timers.
            genieRandomDraw(rng);genieRandomDraw(rng);
            const auto &g=*data.graphic(spec->graphic);
            const float cycle=g.frameCount*g.frameDuration+g.replayDelay;
            const double elapsed=double(genieRandomDraw(rng))*cycle*(1.f/32767.f);
            const int frame=int(elapsed/g.frameDuration);
            placement.animationFrame=uint8_t(std::min(int(g.frameCount)-1,frame));
            if(frame>=g.frameCount) placement.initialIdleDelay=std::max(0.f,float(elapsed-double(g.frameCount-1)*g.frameDuration)-g.frameDuration);
            const uint32_t heading=aquatic?0:genieRandomDraw(rng)*8u/32767u;
            placement.headingRadians=float(double(heading)*3.14159265358979323846/4.);
            placement.facet=uint8_t((heading+7)%8);
            if(!aquatic)for(int timer=0;timer<((hunt || predator)?4:3);++timer)genieRandomDraw(rng);
        } else if(spec->type==30) {
            const auto appearance=drawRmsCarbonAppearance(rng,static_cast<uint8_t>(spec->angles));
            placement.facet=appearance.facet;placement.headingRadians=appearance.headingRadians;
        } else placement.facet=static_cast<uint8_t>(std::min(unsigned(spec->angles-1),genieRandomDraw(rng)*unsigned(spec->angles)/32767u));
        result.push_back(placement);if(!herd && !bird) occupied[tile]=true;
    };
    RmsZoneCache resourceZones;
    for(size_t player=0;player<(perPlayer?novaPlayerStarts.size():1);++player) {
        const int startX=perPlayer?novaPlayerStarts[player].x:-1;
        const int startY=perPlayer?novaPlayerStarts[player].y:-1;
        if(!build(outer,perPlayer && playerMax>=0?startX:-1,perPlayer && playerMax>=0?startY:-1,perPlayer?playerMax:-1)) return fail("work limit exceeded");
        const RmsZones *zones=nullptr;
        uint8_t centerZone=0;
        if(perPlayer) {
            auto connectivity=restriction;connectivity[terrain[startY*width+startX]]=1;
            uint32_t index;
            if(!resourceZones.lookup(width,terrain,connectivity,index,err)) return false;
            zones=&resourceZones.entry(index)->zones;centerZone=zones->tiles[startY*width+startX];
        }
        auto sameZone=[&](int x,int y) {
            x=std::max(0,std::min(int(width)-1,x));y=std::max(0,std::min(int(height)-1,y));
            return zones->tiles[y*width+x]==centerZone;
        };
        uint32_t groupsLeft=group.groups;
        while(next[outer]>=0 && groupsLeft) {
            if(!spend(1)) return fail("work limit exceeded");
            const int center=pop(outer);
            const int x=center%width,y=center/width;
            if((perPlayer || globalPlacement) && std::any_of(exclusionCenters.begin(),exclusionCenters.end(),[&](const auto &start) {
                return std::abs(x-start.x)<playerMin && std::abs(y-start.y)<playerMin;
            })) continue;
            if(perPlayer) {
                if(!sameZone(x,y)) continue;
                // Original zone7 clearance samples cardinal7 and diagonal5, clamped to the map.
                if(!herd && !tree && (!sameZone(x,y-7) || !sameZone(x+5,y-5) || !sameZone(x+7,y) || !sameZone(x+5,y+5) ||
                    !sameZone(x,y+7) || !sameZone(x-5,y+5) || !sameZone(x-7,y) || !sameZone(x-5,y-5))) continue;
            }
            if(((predator || bigHunt) && group.mode==0) || tree || bird || aquatic) {
                // Original mode0 has no inner queue/variance draw. A failed final gate
                // forces the last candidate (4dec9c), after distance/zone exclusions.
                const bool forced=!pass(center);
                if(forced && (globalPlacement || aquatic || next[outer]>=0)) continue;
                if(tree) {
                    const int x0=std::max(0,x-playerSpacing),x1=std::min(int(width)-1,x+playerSpacing);
                    const int y0=std::max(0,y-playerSpacing),y1=std::min(int(height)-1,y+playerSpacing);
                    if(!spend(size_t(x1-x0+1)*size_t(y1-y0+1)*2)) return fail("work limit exceeded");
                    for(int sy=y0;sy<=y1;++sy) for(int sx=x0;sx<=x1;++sx) unlink(sy*width+sx);
                    create(center);
                    for(int sy=y0;sy<=y1;++sy) for(int sx=x0;sx<=x1;++sx)
                        if(restriction[terrain[sy*width+sx]]>0) eligible[sy*width+sx]=false;
                } else create(center);
                if(!forced) --groupsLeft;
                continue;
            }
            if(!pass(center)) continue;
            if(playerSpacing) {
                // Original4de1b0 unlinks the spacing square before the group uses shared nodes.
                for(int sy=std::max(0,y-playerSpacing);sy<=std::min(int(height)-1,y+playerSpacing);++sy)
                    for(int sx=std::max(0,x-playerSpacing);sx<=std::min(int(width)-1,x+playerSpacing);++sx) unlink(sy*width+sx);
            }
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
            if(playerSpacing) {
                // Original4de090 persists spacing only on passable tiles, across later rows.
                for(int sy=std::max(0,y-playerSpacing);sy<=std::min(int(height)-1,y+playerSpacing);++sy)
                    for(int sx=std::max(0,x-playerSpacing);sx<=std::min(int(width)-1,x+playerSpacing);++sx)
                        if(restriction[terrain[sy*width+sx]]>0) eligible[sy*width+sx]=false;
            }
            --groupsLeft;
        }
        while(next[outer]>=0) pop(outer);
    }
    if(rowEligibility) rowEligibility->assign(eligible.begin(),eligible.end());
    out=std::move(result);randomState=rng;
    return true;
}
bool generateRmsHolocronPlacements(uint32_t width,uint32_t height,
    const std::vector<uint8_t> &terrain,const dat::DatFile &data,
    const std::vector<RmsPlayerStart> &playerStarts,uint32_t &randomState,
    std::vector<RmsTimberPlacement> &out,std::string *err,
    const std::vector<uint32_t> &occupiedTiles) {
    out.clear();if(err) err->clear();
    auto fail=[&](const char *reason) {
        if(err) *err=std::string("RMS holocron: ")+reason;
        return false;
    };
    if((width!=64 && width!=120) || height!=width || terrain.size()!=size_t(width)*height || playerStarts.size()!=2)
        return fail("requires64/120-square terrain and two player starts");
    for(const auto &start:playerStarts)
        if(start.x<2 || start.y<2 || start.x>int(width)-2 || start.y>int(height)-2) return fail("invalid player start");
    if(std::any_of(terrain.begin(),terrain.end(),[&](uint8_t t) {
        return t!=0 && t!=27 && (width!=120 || (t!=1 && t!=2 && t!=5 && t!=6 && t!=9 && t!=10 && t!=12 && t!=18 && t!=20 && t!=21 && t!=22 && t!=41));
    }) || (width==64 && !occupiedTiles.empty()))
        return fail("requires verified grass or120-square forest terrain");
    std::vector<bool> occupied(terrain.size());
    for(auto tile:occupiedTiles) {
        if(tile>=terrain.size()) return fail("occupied tile outside map");
        occupied[tile]=true;
    }
    if(data.civs.empty() || data.civs[0].units.size()<=285 || data.terrainRestrictions.size()<=7)
        return fail("missing original Gaia holocron metadata");
    const auto &u=data.civs[0].units[285];const auto *g=data.graphic(5200);
    if(!u.exists || u.id!=285 || u.type!=dat::UT_Creatable || u.cls!=28 || u.hitPoints!=30 ||
       u.collisionSize[0]!=.3f || u.collisionSize[1]!=.3f || u.collisionSize[2]!=2 ||
       u.clearanceSize[0]!=.3f || u.clearanceSize[1]!=.3f || u.terrainRestriction!=7 ||
       u.placementSideTerrain[0]!=-1 || u.placementSideTerrain[1]!=-1 ||
       u.placementTerrain[0]!=-1 || u.placementTerrain[1]!=-1 ||
       u.hillMode || u.flyMode || u.canBeBuiltOn || u.recyclable || u.lineOfSight!=7 ||
       u.obstructionType!=5 || u.obstructionClass!=2 || u.speed!=0 || u.trait ||
       u.trackingUnit!=-1 || u.trackingUnitMode || u.trackingUnitDensity!=0 ||
       u.headUnit!=-1 || u.trainSound!=542 || u.convertTerrain || u.enableAutoGather ||
       u.createDoppelgangerOnDeath || u.standingGraphic[0]!=5200 || u.standingGraphic[1]!=-1 ||
       !g || g->sequenceType!=4 || g->frameCount!=1 || g->angleCount!=1 ||
       g->soundId!=-1 || !g->angleSounds.empty() || !g->deltas.empty())
        return fail("unsupported holocron prototype or graphics");
    const auto &pass=data.terrainRestrictions[7].passableBuildableDmgMultiplier;
    for(auto t:terrain)
        if(t>=pass.size() || !std::isfinite(pass[t])) return fail("invalid holocron terrain restriction");
    uint32_t rng=randomState;std::vector<uint32_t> candidates;
    if(!generateRmsObjectCandidates(width,height,rng,candidates,err)) return false;
    std::vector<bool> removed(terrain.size());std::vector<RmsTimberPlacement> placements;
    for(auto tile:candidates) {
        if(removed[tile] || occupied[tile] || pass[terrain[tile]]<=.05f) continue;
        const int x=tile%width,y=tile/width;
        if(std::any_of(playerStarts.begin(),playerStarts.end(),[&](const auto &start) {
            return std::abs(x-start.x)<25 && std::abs(y-start.y)<25;
        })) continue;
        // Original4de1b0 removes temporary spacing nodes from this row's queue.
        for(int sy=std::max(0,y-20);sy<=std::min(int(height)-1,y+20);++sy)
            for(int sx=std::max(0,x-20);sx<=std::min(int(width)-1,x+20);++sx) removed[sy*width+sx]=true;
        // Static type70: base facet, sprite frame, three action timers, military timer.
        for(int draw=0;draw<6;++draw) genieRandomDraw(rng);
        placements.push_back({x+.5f,y+.5f,7,0,285});
        if(placements.size()==5) break;
    }
    out=std::move(placements);randomState=rng;return true;
}
} // namespace swgb
