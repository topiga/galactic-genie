// SPDX-License-Identifier: GPL-3.0-or-later
#include "rms_objects.h"
#include "rms_object_rows.h"
#include "rms_zones.h"
#include "rms_scenery_appearance.h"
#include "../core/genie_random.h"
#include "../core/genie_dat.h"
#include <algorithm>
#include <cmath>

namespace swgb {
namespace {
bool isTree(int unit) { return unit==348 || unit==349 || unit==350 || unit==351 || unit==411 || unit==413; }
bool isPredator(int unit) { return unit==1459 || unit==1461 || unit==1463 || unit==126 || unit==812 || unit==1197 || unit==1250 || unit==1129 || unit==1252 || unit==1251 || unit==1465 || unit==998 || unit==1033; }
bool isMountain(int unit) { return unit==589 || unit==1017 || unit==1018 || unit==1019 || unit==1020 || unit==1165 || unit==1166; }
bool isCrater(int unit) { return unit>=1329 && unit<=1334; }
bool isGaiaBuilding(int unit) { return unit==1295 || unit==1052 || unit==1307 || (unit>=1184 && unit<=1188) || unit==275 || unit==518 || unit==519 || unit==524 || unit==536 || unit==537; }
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
        {350,5298,10,31,50,11,12,1,100,2,2.f},
        {351,5295,10,31,50,11,12,1,100,2,2.f},
        {411,5292,10,31,50,4,12,1,100,2,2.f},
        {413,5301,10,31,50,11,12,1,125,2,2.f},
        {59,5192,10,27,0,8,6,0,125,1,1.f},
        {66,5134,30,29,0,8,10,3,800,1,1.f},
        {102,5232,30,30,0,8,10,2,350,1,1.f},
        {1160,5147,30,31,50,11,12,1,100,2,2.f},
        {1161,5158,20,31,100,8,10,1,100,2,2.f},
        {1170,5266,10,42,50,1,3,-1,0,0,1.f},
        {1171,5267,10,42,50,1,3,-1,0,0,1.f},
        {1123,5249,10,42,50,1,10,-1,0,0,0.f},
        {573,8960,10,42,50,29,1,-1,0,0,2.f},
        {582,8961,10,42,50,29,1,-1,0,0,2.f},
        {583,8962,10,42,50,29,1,-1,0,0,3.f},
        {616,8958,10,42,50,29,1,-1,0,0,0.f},
        {589,8963,10,42,5000,11,1,-1,0,0,3.f},
        {1159,5235,30,30,0,8,12,2,350,1,1.f},
        {1017,5238,10,42,5000,11,1,-1,0,0,3.f},
        {1018,5239,10,42,5000,11,1,-1,0,0,3.f},
        {1296,1958,10,42,2100,4,1,-1,0,0,3.f},
        {1122,5248,10,42,50,1,6,-1,0,0,0.f},
        {1120,5246,10,42,50,1,6,-1,0,0,0.f},
        {1172,5316,10,42,50,13,2,-1,0,0,1.f},
        {1173,5315,10,42,50,13,2,-1,0,0,1.f},
        {1295,1949,80,18,3000,4,1,-1,0,0,4.f},
        {1052,1958,80,18,2100,4,1,-1,0,0,3.f},
        {1019,5240,10,42,5000,11,1,-1,0,0,3.f},
        {1020,5241,10,42,5000,11,1,-1,0,0,3.f},
        {1165,5276,10,42,5000,11,1,-1,0,0,3.f},
        {1166,5277,10,42,5000,11,1,-1,0,0,3.f},
        {615,8956,20,27,0,8,12,0,250,1,2.f},
        {1117,5140,30,31,100,8,12,1,100,2,2.f},
        {1332,5202,10,42,50,28,1,-1,0,0,0.f},
        {1333,5203,10,42,50,28,1,-1,0,0,0.f},
        {1334,5204,10,42,50,28,1,-1,0,0,0.f},
        {1329,5227,10,42,50,29,1,-1,0,0,0.f},
        {1330,5226,10,42,50,29,1,-1,0,0,0.f},
        {1331,5225,10,42,50,29,1,-1,0,0,0.f},
        {1307,1911,80,18,800,4,1,-1,0,0,2.f},
        {1184,2854,80,18,800,4,1,-1,0,0,2.f},
        {1185,2860,80,18,800,4,1,-1,0,0,2.f},
        {1186,2847,80,18,2100,4,1,-1,0,0,2.f},
        {1187,2866,80,18,2000,4,1,-1,0,0,2.f},
        {1188,2873,80,18,2000,4,1,-1,0,0,2.f},
        {275,8865,80,18,2100,4,1,-1,0,0,2.f},
        {518,8865,80,18,2100,4,1,-1,0,0,2.f},
        {519,8866,80,18,800,4,1,-1,0,0,2.f},
        {524,8868,80,18,800,4,1,-1,0,0,2.f},
        {536,8889,80,18,800,4,1,-1,0,0,2.f},
        {537,8890,80,18,800,4,1,-1,0,0,2.f}};
    const ResourceSpec *match=nullptr;
    for (const auto &candidate:specs) if (candidate.id==unitId) match=&candidate;
    if (!match) return fail("unsupported resource unit");
    const auto &spec=*match;
    const bool mountain=isMountain(unitId),crater=isCrater(unitId),mine=unitId==1307,building=isGaiaBuilding(unitId),stalagmite=unitId==573 || unitId==582 || unitId==583,vent=unitId==616;
    const float radius=unitId==573?1.f:(unitId==582 || vent)?1.5f:unitId==583?2.f:unitId==1295?4.f:(mountain || unitId==1296 || unitId==1052 || unitId==275 || unitId==518)?3.f:(unitId==1184 || unitId==1185)?1.5f:(unitId==1186 || unitId==519 || unitId==524)?2.f:(mine || unitId==1187 || unitId==536 || unitId==537)?1.f:crater?float(unitId>=1332?1336-unitId:unitId-1327)*.5f:.5f;
    const float clearance=unitId==1188?0.f:radius;
    if (data.civs.empty() || data.civs[0].units.size()<=static_cast<size_t>(unitId))
        return fail("missing Gaia resource prototype");
    const auto &u=data.civs[0].units[unitId];
    const auto *g=data.graphic(spec.graphic);
    if (!u.exists || u.id!=unitId || u.type!=spec.type || u.cls!=spec.cls ||
        u.standingGraphic[0]!=spec.graphic || u.standingGraphic[1]!=-1 || u.hitPoints!=spec.hp ||
        u.collisionSize[0]!=radius || u.collisionSize[1]!=radius || u.collisionSize[2]!=spec.height ||
        u.clearanceSize[0]!=clearance || u.clearanceSize[1]!=clearance ||
        u.placementSideTerrain[0]!=-1 || u.placementSideTerrain[1]!=-1 ||
        u.placementTerrain[0]!=-1 || u.placementTerrain[1]!=-1 ||
        u.terrainRestriction!=spec.restriction || u.hillMode!=(unitId==1295?3:0) || u.flyMode || u.canBeBuiltOn!=int((crater && unitId!=1332) || stalagmite || vent) ||
        u.recyclable || u.lineOfSight!=(unitId==1295?6.f:unitId==1296?1.f:(unitId==1184 || unitId==1185 || unitId==519 || unitId==524 || unitId==537)?2.f:building?1.f:0.f) || u.trainSound!=-1 ||
        u.obstructionType!=(mountain?10:(unitId==1120 || unitId==1122 || unitId==1123 || crater || vent)?0:2) || u.obstructionClass!=((unitId==1120 || unitId==1122 || unitId==1123 || crater || vent)?0:(unitId==1170 || unitId==1171 || unitId==1172 || unitId==1173 || unitId==1296 || stalagmite || building)?3:1) || u.civilization ||
        u.enableAutoGather!=(spec.resource<0?0:1) || !g || g->sequenceType!=(mine?5:(mountain || crater || stalagmite || vent || unitId==1295)?0:6) || g->angleCount!=spec.angles)
        return fail("unsupported resource prototype or graphic");
    if ((spec.type==20 || spec.type==30 || mountain || crater || stalagmite || vent || unitId==1296 || building) && (u.createDoppelgangerOnDeath!=spec.doppel || u.speed!=0 ||
        u.trackingUnit!=-1 || u.trackingUnitMode!=0 || u.trackingUnitDensity!=0 ||
        g->frameCount!=(mine?8:1) || g->soundId!=-1 || !g->angleSounds.empty()))
        return fail("unsupported resource movement or sprite metadata");
    if(building) {
        const int construction=unitId==1295?373:unitId==1052?367:(unitId==275 || unitId==518)?373:unitId==1186?367:unitId==1188?342:348;
        const int sound=unitId==1295?602:unitId==1052?920:mine?919:(unitId==519 || unitId==537)?930:(unitId==1184 || unitId==1185 || unitId==524)?591:2;
        if(((unitId==519 || unitId==537) && u.fogVisibility!=1) || u.foundationTerrainId!=((unitId==1295 || unitId==519 || unitId==537)?27:-1) || u.oldOverlayId!=-1 || u.techId!=-1 || u.transformUnit!=-1 ||
           u.headUnit!=-1 || u.stackUnitId!=-1 || u.adjacentMode || u.graphicsAngle || u.disappearsWhenBuilt ||
           u.trait!=(unitId==1295?2:0) || u.convertTerrain || u.constructionGraphic!=construction || u.constructionSound!=sound ||
           u.trainTime!=(unitId==1295?4000:20) || g->frameDuration!=(mine?.3f:0.f) || g->replayDelay!=0)
            return fail("unsupported Gaia building lifecycle metadata");
        for(const auto &annex:u.annexes)if(annex.unitId!=-1)return fail("Gaia building annexes are unsupported");
        if(unitId==1295 || unitId==1052) {
            for(size_t n=0;n<u.costs.size();++n) {
                const auto &cost=u.costs[n];const int type=unitId==1295?(n==0?1:n==1?3:2):(n==0?1:-1);
                if(cost.type!=type || cost.amount!=(unitId==1295?1500:n==0?25:0) || cost.flag!=(type<0?0:1))
                    return fail("unsupported temple completion costs");
            }
        }
    }
    int resourceStores=0;
    for (const auto &storage:u.resourceStorages) {
        if (storage.flag || (storage.type>=0 && (storage.type!=spec.resource || storage.amount!=spec.amount)))
            return fail("unsupported resource storage");
        resourceStores+=storage.type>=0;
    }
    if (resourceStores!=(spec.resource>=0?1:0)) return fail("unexpected resource storage count");
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
    static constexpr ResourceSpec bantha{833,1,70,1,7,19,8,0,200,0,2.f};
    static constexpr ResourceSpec kaadu{1363,46,70,4,5,1,8,0,100,0,2.f};
    static constexpr ResourceSpec eopie{1365,22,70,4,5,1,8,0,140,0,2.f};
    static constexpr ResourceSpec mynock{1203,56,70,4,5,1,8,0,140,0,2.f};
    static constexpr ResourceSpec orray{1471,8709,70,4,29,26,8,0,340,0,2.f};
    static constexpr ResourceSpec acklay{1459,8695,70,5,150,26,8,-1,0,0,2.f};
    static constexpr ResourceSpec nexu{1461,8705,70,5,150,19,8,-1,0,0,2.f};
    static constexpr ResourceSpec reek{1463,8714,70,5,150,26,8,-1,0,0,2.f};
    static constexpr ResourceSpec cupa{1367,80,70,4,10,26,8,0,170,0,2.f};
    static constexpr ResourceSpec womprat{1475,8731,70,4,15,26,8,0,170,0,2.f};
    static constexpr ResourceSpec marine998{998,65,70,5,80,27,8,-1,0,0,2.f};
    static constexpr ResourceSpec marine1033{1033,12,70,5,80,27,8,-1,0,0,2.f};
    static constexpr ResourceSpec sarlacc{1465,8718,70,5,500,26,1,-1,0,0,2.f};
    static constexpr ResourceSpec rancor{126,70,70,5,150,26,8,-1,0,0,2.f};
    const bool bigHunt=unitId==822 || unitId==1249 || unitId==1364;
    const bool predator=isPredator(unitId);
    static constexpr ResourceSpec predator1251{1251,89,70,5,50,1,8,-1,0,0,2.f};
    static constexpr ResourceSpec predator1252{1252,94,70,5,150,19,8,-1,0,0,2.f};
    static constexpr ResourceSpec predator1250{1250,41,70,5,30,1,8,-1,0,0,2.f};
    static constexpr ResourceSpec predator1129{1129,51,70,5,100,26,8,-1,0,0,2.f};
    static constexpr ResourceSpec predator812{812,84,70,5,40,1,8,-1,0,0,2.f};
    static constexpr ResourceSpec predator1197{1197,36,70,5,75,1,8,-1,0,0,2.f};
    static constexpr ResourceSpec hunt822{822,31,70,60,50,1,8,0,300,0,2.f};
    static constexpr ResourceSpec hunt1249{1249,17,70,60,75,26,8,0,340,0,2.f};
    static constexpr ResourceSpec ronto{1364,75,70,60,100,19,8,0,400,0,2.f};
    const auto &spec=unitId==1203?mynock:unitId==1471?orray:unitId==1459?acklay:unitId==1461?nexu:unitId==1463?reek:unitId==998?marine998:unitId==1033?marine1033:unitId==1465?sarlacc:unitId==1367?cupa:unitId==1475?womprat:unitId==1251?predator1251:unitId==1252?predator1252:unitId==1250?predator1250:unitId==1129?predator1129:unitId==1364?ronto:unitId==833?bantha:unitId==822?hunt822:unitId==1249?hunt1249:unitId==812?predator812:unitId==1197?predator1197:predator?rancor:unitId==1363?kaadu:unitId==1365?eopie:nerf;
    const bool hunt=unitId==1203 || unitId==1363 || unitId==1365 || unitId==1367 || unitId==1471 || unitId==1475;
    auto fail=[&]()->const ResourceSpec* {if(err)*err="RMS herd: unsupported original animal metadata";return nullptr;};
    if(data.civs.empty() || data.civs[0].units.size()<=size_t(unitId) || data.terrainRestrictions.size()<=size_t(spec.restriction)) return fail();
    const auto &u=data.civs[0].units[unitId];const auto *g=data.graphic(spec.graphic);
    const float radius=(unitId==1203 || unitId==1471)?.5f:unitId==833?.35f:unitId==1364?.3f:bigHunt?.5f:predator?.25f:.3f;
    const float clearance=(unitId==1203 || unitId==1471)?1.f:unitId==1364?.3f:bigHunt?1.f:predator?.5f:radius;
    if(!u.exists || u.id!=unitId || u.type!=70 || u.cls!=spec.cls || u.hitPoints!=spec.hp || u.standingGraphic[0]!=spec.graphic || u.standingGraphic[1]!=-1 ||
       u.collisionSize[0]!=radius || u.collisionSize[1]!=radius || u.collisionSize[2]!=2.f || u.clearanceSize[0]!=clearance || u.clearanceSize[1]!=clearance ||
       u.lineOfSight!=(unitId==1471?4.f:(unitId==1250 || unitId==1251 || unitId==1252)?5.f:(unitId==1129 || unitId==998 || unitId==1033)?8.f:bigHunt?4.f:unitId==812?6.f:unitId==1197?10.f:predator?7.f:2.f) || u.speed!=(unitId==1471?.75f:unitId==998?.5f:unitId==1465?0.f:(unitId==1475 || unitId==1367)?1.2f:unitId==1251?.8f:unitId==1250?.7f:unitId==1129?.95f:unitId==1364?.85f:bigHunt?.9f:unitId==812?.9f:unitId==1197?.85f:hunt?.74f:.7f) || u.terrainRestriction!=spec.restriction || u.obstructionType!=5 || u.obstructionClass!=2 ||
       u.hillMode || u.flyMode || ((unitId==998 || unitId==1033) && u.trait!=2) || u.canBeBuiltOn || u.recyclable || u.civilization || u.enableAutoGather || u.createDoppelgangerOnDeath ||
       u.placementSideTerrain[0]!=-1 || u.placementSideTerrain[1]!=-1 || u.placementTerrain[0]!=((unitId==998 || unitId==1033)?23:-1) || u.placementTerrain[1]!=-1 ||
       u.trainSound!=(unitId==1203?633:unitId==1471?1036:unitId==1459?1024:unitId==1461?1032:unitId==1463?1040:unitId==998?627:unitId==1033?942:unitId==1465?1044:unitId==1367?442:unitId==1475?1056:unitId==1251?622:unitId==1252?619:unitId==1250?636:unitId==1129?836:unitId==1364?624:unitId==833?615:unitId==822?612:unitId==1249?562:unitId==812?642:unitId==1197?639:predator?37:unitId==1363?421:unitId==1365?609:630) || !g || g->sequenceType!=7 || g->frameCount!=(unitId==833?11:unitId==1364?8:bigHunt?10:unitId==1363?11:unitId==1365?12:unitId==1367?9:8) || g->angleCount!=spec.angles || g->frameDuration!=((unitId==812 || unitId==1197 || unitId==1251)?.1f:(hunt || predator || bigHunt)?.12f:.275f) ||
       g->replayDelay!=((unitId==812 || unitId==1197 || unitId==1251)?1.f:(hunt || predator || bigHunt)?.5f:0.f) || g->soundId!=-1 || !g->angleSounds.empty()) return fail();
    int stores=0;for(const auto &storage:u.resourceStorages) {
        if(storage.flag || (storage.type>=0 && (predator || storage.type!=0 || storage.amount!=spec.amount))) return fail();
        stores+=storage.type==0;
    }
    if(stores!=(predator?0:1)) return fail();
    const auto &restriction=data.terrainRestrictions[spec.restriction].passableBuildableDmgMultiplier;
    for(auto t:terrain) if(t>=restriction.size() || !std::isfinite(restriction[t])) return fail();
    return &spec;
}
const ResourceSpec *validateSandcrawler(const dat::DatFile &data,const std::vector<uint8_t> &terrain,std::string *err) {
    static constexpr ResourceSpec spec{938,6338,70,54,185,25,8,-1,0,0,2.f};
    auto fail=[&]()->const ResourceSpec* {if(err)*err="RMS Sandcrawler: unsupported original metadata";return nullptr;};
    if(data.civs.empty() || data.civs[0].units.size()<=938 || data.terrainRestrictions.size()<=25)return fail();
    const auto &u=data.civs[0].units[938];const auto *g=data.graphic(6338);
    if(!u.exists || u.id!=938 || u.type!=70 || u.cls!=54 || u.hitPoints!=185 || u.standingGraphic[0]!=6338 || u.standingGraphic[1]!=-1 ||
       u.collisionSize[0]!=.5f || u.collisionSize[1]!=.5f || u.collisionSize[2]!=2 || u.clearanceSize[0]!=.5f || u.clearanceSize[1]!=.5f ||
       u.lineOfSight!=9 || u.speed!=1 || u.terrainRestriction!=25 || u.obstructionType!=5 || u.obstructionClass!=2 ||
       u.hillMode || u.flyMode || u.canBeBuiltOn || u.recyclable || u.civilization || u.enableAutoGather || u.createDoppelgangerOnDeath ||
       u.placementSideTerrain[0]!=-1 || u.placementSideTerrain[1]!=-1 || u.placementTerrain[0]!=-1 || u.placementTerrain[1]!=-1 ||
       u.trainSound!=406 || !g || g->sequenceType!=6 || g->frameCount!=1 || g->angleCount!=8 || g->frameDuration!=.2f ||
       g->replayDelay!=1 || g->soundId!=-1 || !g->angleSounds.empty())return fail();
    int population=0,units=0;
    for(const auto &storage:u.resourceStorages) {
        if(storage.type==4 && storage.amount==-1 && storage.flag==2)++population;
        else if(storage.type==11 && storage.amount==1 && storage.flag==2)++units;
        else if(storage.type>=0 || storage.flag)return fail();
    }
    if(population!=1 || units!=1)return fail();
    const auto &restriction=data.terrainRestrictions[25].passableBuildableDmgMultiplier;
    for(auto t:terrain)if(t>=restriction.size() || !std::isfinite(restriction[t]))return fail();
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
    static constexpr ResourceSpec fish450{450,5269,30,23,0,9,1,17,350,1,0};
    static constexpr ResourceSpec fish451{451,5270,30,23,0,9,1,17,350,1,0};
    static constexpr ResourceSpec fish455{455,5271,30,23,0,9,1,17,225,1,0};
    static constexpr ResourceSpec fish458{458,5274,30,23,0,9,1,17,225,1,0};
    static constexpr ResourceSpec deepFish{456,5272,30,23,0,9,1,17,225,1,0};
    static constexpr ResourceSpec fish{69,5268,30,25,0,9,1,17,200,1,0};
    static constexpr ResourceSpec wave{1585,10295,30,42,1000,31,1,-1,0,0,0};
    const auto &spec=unitId==458?fish458:unitId==450?fish450:unitId==451?fish451:unitId==455?fish455:unitId==456?deepFish:unitId==69?fish:wave;
    const bool largeFish=unitId==450 || unitId==451 || unitId==455 || unitId==456;
    const float radius=largeFish?1.f:.5f;
    auto fail=[&]()->const ResourceSpec* {if(err)*err="RMS aquatic: unsupported original profile";return nullptr;};
    if(data.civs.empty() || data.civs[0].units.size()<=size_t(unitId) || data.terrainRestrictions.size()<=size_t(spec.restriction))return fail();
    const auto &u=data.civs[0].units[unitId];const auto *g=data.graphic(spec.graphic);
    if(!u.exists || u.id!=unitId || u.type!=30 || u.cls!=spec.cls || u.hitPoints!=spec.hp || u.standingGraphic[0]!=spec.graphic || u.standingGraphic[1]!=-1 ||
       u.collisionSize[0]!=radius || u.collisionSize[1]!=radius || u.collisionSize[2]!=0 || u.clearanceSize[0]!=radius || u.clearanceSize[1]!=radius ||
       u.lineOfSight!=0 || u.speed!=(unitId==69?1.32f:0.f) || u.terrainRestriction!=spec.restriction || u.obstructionType || u.obstructionClass ||
       u.hillMode || u.flyMode || u.canBeBuiltOn || u.recyclable || u.civilization || u.enableAutoGather!=(unitId!=1585) || u.createDoppelgangerOnDeath!=spec.doppel ||
       u.placementSideTerrain[0]!=(unitId==69?2:-1) || u.placementSideTerrain[1]!=(unitId==69?35:-1) || u.placementTerrain[0]!=-1 || u.placementTerrain[1]!=-1 ||
       u.trainSound!=-1 || !g || g->sequenceType!=7 || g->frameCount!=((unitId==450 || unitId==451)?35:largeFish?30:(unitId==69 || unitId==458)?12:48) || g->angleCount!=1 || g->frameDuration!=((unitId==450 || unitId==451)?.1f:.13f) ||
       g->replayDelay!=0 || g->soundId!=-1 || !g->angleSounds.empty())return fail();
    int stores=0;for(const auto &storage:u.resourceStorages){if(storage.flag || (storage.type>=0 && (storage.type!=spec.resource || storage.amount!=spec.amount)))return fail();stores+=storage.type>=0;}
    if(stores!=(unitId==1585?0:1))return fail();
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
    if(isMountain(unitId) || unitId==573 || unitId==582 || unitId==583 || unitId==616 || unitId==1296 || isCrater(unitId) || isGaiaBuilding(unitId)) {if(err)*err="RMS large scenery requires a verified object row";return false;}
    if (err) err->clear();
    auto fail = [&](const char *reason) {
        if (err) *err = std::string("RMS resource: ") + reason;
        return false;
    };
    if(unitId==59 || unitId==349 || unitId==350 || unitId==351 || unitId==411 || unitId==413) return fail("resource requires the verified grouped row");
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
        if (occupied[tile] && unitId!=1123) continue;
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
    std::vector<uint8_t> *rowEligibility,const RmsGlobalGroupPlacement *globalPlacement,
    const RmsResourcePlayerRange *playerRange,const RmsObjectRow *objectRow,const std::vector<uint8_t> *elevation) {
    out.clear(); if(err) err->clear();
    auto fail=[&](const char *reason) {
        if(err) *err=std::string("RMS grouped resource: ")+reason;
        return false;
    };
    const size_t count=size_t(width)*height;
    if(!width || !height || width>1024 || height>1024 || terrain.size()!=count ||
       (group.mode!=1 && group.mode!=2 && !((objectRow || isPredator(unitId) || unitId==822 || unitId==1249 || unitId==1364 || isTree(unitId) || unitId==816 || unitId==69 || unitId==1585) && group.mode==0)) || group.groups>count || group.perGroup>count ||
       group.variance>1024 || group.radius>1024)
        return fail("unsupported dimensions or grouping parameters");
    const bool mountain=isMountain(unitId),crater=isCrater(unitId),mine=unitId==1307,building=isGaiaBuilding(unitId),stalagmite=unitId==573 || unitId==582 || unitId==583,vent=unitId==616;
    const bool settlement=building && unitId<1000,temple=unitId==1295 || unitId==1052;
    if(unitId==1295 && (!elevation || elevation->size()!=count))return fail("temple hill placement requires an elevation raster");
    if(building && (!objectRow || width!=120 || height!=120 || objectRow->landId!=-2 ||
        objectRow->minDistance!=(settlement?((unitId==275 || unitId==537)?12:unitId==536?8:10):35) || objectRow->maxDistance!=-1 || objectRow->terrain!=-1 ||
        objectRow->spacing!=((settlement || temple)?0:15) || objectRow->temporarySpacing || objectRow->zoneDistance ||
        group.mode!=((temple || unitId==1184 || unitId==1185)?1:0) || group.groups!=(temple?(unitId==1295?1u:2u):settlement?(unitId==275?2u:(unitId==536 || unitId==537)?4u:unitId==524?5u:1u):mine?20u:unitId==1186?1u:unitId==1188?5u:10u) || group.perGroup!=1 || group.variance || group.radius!=3))
        return fail("unsupported Gaia building row");
    if(mountain && (!objectRow || width!=120 || height!=120 || objectRow->landId!=-2 ||
        objectRow->minDistance!=(unitId==589?25:40) || objectRow->maxDistance!=-1 || objectRow->terrain!=-1 ||
        objectRow->spacing || objectRow->temporarySpacing || objectRow->zoneDistance ||
        group.mode || group.groups!=(unitId==589?2u:1u) || group.perGroup!=1 || group.variance || group.radius!=3))
        return fail("unsupported mountain row");
    const bool crawler=unitId==938;
    if(crawler && (!objectRow || width!=120 || height!=120 || objectRow->landId!=-2 ||
       (objectRow->minDistance!=10 && objectRow->minDistance!=20) || objectRow->maxDistance!=-1 || objectRow->terrain!=-1 ||
       objectRow->spacing || objectRow->temporarySpacing || objectRow->zoneDistance ||
       group.mode!=1 || group.groups!=2 || group.perGroup!=1 || group.variance || group.radius!=3))
        return fail("unsupported Sandcrawler row");
    const bool hunt=unitId==1203 || unitId==1363 || unitId==1365 || unitId==1367 || unitId==1471 || unitId==1475;
    const bool predator=isPredator(unitId);
    if(!objectRow && (unitId==349 || unitId==350 || unitId==351 || unitId==411 || unitId==413) && group.mode!=0)return fail("tree species requires verified mode0 rows");
    const bool tree=isTree(unitId) && group.mode==0,bird=unitId==816,largeFish=unitId==450 || unitId==451 || unitId==455 || unitId==456,
        aquatic=unitId==458 || unitId==69 || unitId==1585 || largeFish;
    const bool bigHunt=unitId==822 || unitId==1249 || unitId==1364;
    const bool herd=crawler || unitId==594 || unitId==833 || hunt || predator || bigHunt;
    const auto *spec=crawler?validateSandcrawler(data,terrain,err):aquatic?validateAquatic(data,unitId,terrain,err):bird?validateBird(data,terrain,err):herd?validateHerd(data,unitId,terrain,err):validateResource(data,unitId,terrain,err);
    if(!spec) return false;
    const auto &restriction=data.terrainRestrictions[spec->restriction].passableBuildableDmgMultiplier;
    const bool perPlayer=!novaPlayerStarts.empty();
    const bool forage=unitId==59,ore=unitId==102 && perPlayer;
    const bool medium=distance==RmsResourceDistance::Medium,far=distance==RmsResourceDistance::Far;
    if(objectRow) {
        if(objectRow->unitId!=unitId || objectRow->mode!=group.mode ||
            objectRow->groups<0 || uint32_t(objectRow->groups)!=group.groups ||
            objectRow->perGroup<0 || uint32_t(objectRow->perGroup)!=group.perGroup ||
            objectRow->variance<0 || uint32_t(objectRow->variance)!=group.variance ||
            objectRow->radius<0 || uint32_t(objectRow->radius)!=group.radius || playerRange)
            return fail("object row and grouping parameters disagree");
        if(objectRow->owner < -1 || objectRow->owner>0 || objectRow->matchPlayerCiv ||
            (objectRow->landId>=0 && objectRow->owner!=0) || objectRow->landId < -2)
            return fail("object row requires supported Gaia land selection");
        if((objectRow->landId>=0)!=perPlayer || novaPlayerStarts.size()>8 ||
            (objectRow->landId==-2 && !globalPlacement) ||
            (objectRow->landId==-1 && globalPlacement))
            return fail("object row land selection lacks matching centers");
        if(objectRow->minDistance < -1 || objectRow->minDistance>1024 ||
            objectRow->maxDistance < -1 || objectRow->maxDistance>1024 ||
            objectRow->spacing<0 || objectRow->spacing>1024 ||
            objectRow->temporarySpacing<0 || objectRow->temporarySpacing>1024 ||
            objectRow->zoneDistance<0 || objectRow->zoneDistance>1024 ||
            objectRow->terrain < -1 || objectRow->terrain>255)
            return fail("object row placement parameters exceed supported bounds");
    } else {
        // Reytha uses an unbounded outer queue for its late Kaadu groups.
        if(playerRange && ((unitId!=1363 && unitId!=1365) || !perPlayer || globalPlacement || width!=120 || height!=120 ||
            novaPlayerStarts.size()!=2 || distance!=RmsResourceDistance::Far ||
            playerRange->minimum!=19 || playerRange->maximum!=-1 || group.mode!=1 || group.groups!=1 ||
            (group.perGroup!=4 && group.perGroup!=8) || group.variance!=1 || group.radius!=3))
            return fail("unsupported explicit player resource range");
        if(aquatic && (width!=120 || height!=120 || perPlayer || globalPlacement || distance!=RmsResourceDistance::Near ||
           group.mode!=0 || group.groups!=(unitId==69?4u:8u) || group.perGroup!=1 || group.variance || group.radius!=3))
            return fail("unsupported stock aquatic row");
        if(bird && (width!=120 || height!=120 || perPlayer || globalPlacement || distance!=RmsResourceDistance::Near ||
           group.mode!=0 || group.groups!=8 || group.perGroup!=1 || group.variance || group.radius!=3))
            return fail("unsupported stock bird row");
        if(globalPlacement) {
            if(perPlayer || (!predator && !tree && ((unitId!=66 && unitId!=102) || group.mode!=2)) ||
               (predator && (width!=120 || height!=120 || group.mode!=1 || group.groups!=4 || (group.perGroup!=1 && group.perGroup!=2) || ((unitId==1250 || unitId==1129) && group.perGroup!=1) || group.variance || group.radius!=3 ||
                globalPlacement->landCenters.size()!=2 || globalPlacement->minDistance!=40 || globalPlacement->spacing!=12)) ||
               (tree && (width!=120 || height!=120 || group.mode!=0 || group.perGroup!=1 || group.variance || group.radius!=3 ||
                globalPlacement->landCenters.size()!=2 ||
                !((unitId!=350 && group.groups==100 && globalPlacement->minDistance==6 && globalPlacement->spacing==2) ||
                  ((unitId==411 || unitId==350) && group.groups==43 && globalPlacement->minDistance==8 && globalPlacement->spacing==0)))) ||
               distance!=RmsResourceDistance::Near || globalPlacement->minDistance<0 ||
               globalPlacement->minDistance>1024 || globalPlacement->spacing<0 || globalPlacement->spacing>1024 ||
               globalPlacement->landCenters.size()>1024)
                return fail("unsupported global group placement");
            for(const auto &land:globalPlacement->landCenters)
                if(land.x<0 || land.y<0 || uint32_t(land.x)>=width || uint32_t(land.y)>=height)
                    return fail("global land center outside map");
        }
        if(distance!=RmsResourceDistance::Near && !medium && !far) return fail("invalid resource distance profile");
        if((medium || far) && (!perPlayer || (unitId!=66 && !((ore || herd || tree) && far)) || width!=120 || height!=120 || novaPlayerStarts.size()!=2))
            return fail("distance profile requires verified120-square two-player resource row");
        if(bigHunt && (medium || (far && group.mode!=1) || (group.mode!=0 && group.mode!=1) ||
           (group.mode==0 && (group.groups<1 || group.groups>2u)))) return fail("unsupported large-hunt row");
        if((hunt || predator) && !far && !globalPlacement) return fail("hunting animals require the verified far row");
        if((forage || ore || herd || tree) && !globalPlacement && (!perPlayer || width!=120 || height!=120 || novaPlayerStarts.size()!=2))
            return fail("resource row requires verified120-square two-player profile");
        if((forage || ore || herd || tree || medium || far) && std::any_of(terrain.begin(),terrain.end(),[&](uint8_t t) {
            const bool forest=width==120 && (t==1 || t==2 || t==4 || t==5 || t==12 || t==18 || t==20 || t==21 || t==22);
            return !forest && t!=0 && t!=6 && t!=9 && t!=10 && t!=27 && t!=41 && t!=44 && t!=14 && t!=45 && t!=46 && t!=50 && t!=19 && t!=26;
        })) return fail("resource row requires verified grass/forest terrain");
        if(perPlayer) {
            const uint32_t expectedGroups=bigHunt?(group.mode==0?group.groups:1u):tree?(far?3u:2u):herd && far && !hunt?2u:1u;
            const uint32_t expectedCount=bigHunt?(far?3u:group.mode==0?1u:unitId==822?4u:2u):(tree || predator)?1u:hunt?4u:herd?(far?2u:4u):forage?6u:ore?(far?4u:5u):(medium || far)?4u:7u;
            if((unitId!=66 && !forage && !ore && !herd && !tree) || (!bigHunt && group.mode!=((tree || predator)?0:herd?1:2)) ||
               group.groups!=expectedGroups || (group.perGroup!=expectedCount && !(forage && (group.perGroup==3 || group.perGroup==12)) && !(unitId==822 && group.mode==1 && ((!far && (group.perGroup==1 || group.perGroup==2)) || (far && (group.perGroup==1 || group.perGroup==6)))) && !(hunt && group.perGroup==8) && !(hunt && group.perGroup==3) && !(hunt && group.perGroup==2) && !((unitId==1249 || unitId==1364) && (group.perGroup==1 || (!far && group.perGroup==4) || (far && group.perGroup==6))) && !((unitId==594 || unitId==833) && (group.perGroup==expectedCount/2 || group.perGroup==expectedCount*2))) ||
               group.variance!=((hunt || (bigHunt && far))?1u:0u) || group.radius!=(ore?2u:3u) || novaPlayerStarts.size()>8)
                return fail("unsupported per-player resource row");
            for(auto t:terrain) if(restriction[t]!=1.f && (width!=120 || (t!=1 && t!=2 && t!=4 && t!=5 && t!=6 && t!=10 && t!=12 && t!=18 && t!=20 && t!=21 && t!=22 && t!=50 && t!=19)))
                return fail("resource row has unverified nonuniform passability");
        }
    }
    for(const auto &start:novaPlayerStarts)
        if(start.x<0 || start.y<0 || uint32_t(start.x)>=width || uint32_t(start.y)>=height)
            return fail("resource player start outside map");
    if(globalPlacement) {
        if(globalPlacement->landCenters.size()>1024) return fail("too many exclusion centers");
        for(const auto &land:globalPlacement->landCenters)
            if(land.x<0 || land.y<0 || uint32_t(land.x)>=width || uint32_t(land.y)>=height)
                return fail("global land center outside map");
    }
    const int playerMin=objectRow?objectRow->minDistance:playerRange?playerRange->minimum:globalPlacement?globalPlacement->minDistance:tree?(far?6:4):predator?30:bigHunt?(far?35:16):herd?(far?14:10):forage?10:ore?(far?20:14):medium?18:far?25:12;
    const int playerMax=objectRow?objectRow->maxDistance:playerRange?playerRange->maximum:tree?(far?8:5):predator?40:bigHunt?(far?-1:22):herd?(far?30:12):forage?12:ore?(far?26:18):medium?26:far?35:16,playerSpacing=objectRow?objectRow->spacing:globalPlacement?globalPlacement->spacing:perPlayer?(tree?2:bigHunt && far?5:herd?0:forage?6:7):0;
    const auto &exclusionCenters=globalPlacement?globalPlacement->landCenters:novaPlayerStarts;
    const int temporarySpacing=objectRow?objectRow->temporarySpacing:0;
    const int zoneDistance=objectRow?objectRow->zoneDistance:(!herd && !tree?7:0);
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
    auto removeSpacing=[&](int x,int y,int radius,bool persistent) {
        if(!radius) return true;
        const int x0=std::max(0,x-radius),x1=std::min(int(width)-1,x+radius);
        const int y0=std::max(0,y-radius),y1=std::min(int(height)-1,y+radius);
        if(!spend(size_t(x1-x0+1)*size_t(y1-y0+1))) return false;
        for(int sy=y0;sy<=y1;++sy) for(int sx=x0;sx<=x1;++sx) {
            const int tile=sy*width+sx;
            if(persistent) {
                if(restriction[terrain[tile]]>0) eligible[tile]=false;
            } else unlink(tile);
        }
        return true;
    };
    auto pass=[&](int tile) {
        const auto &placementTerrain=data.civs[0].units[unitId].placementTerrain;
        if(placementTerrain[0]>=0 && terrain[tile]!=placementTerrain[0] && terrain[tile]!=placementTerrain[1]) return false;
        if(mountain || crater || stalagmite || vent || building || unitId==1296 || largeFish) {
            const auto &prototype=data.civs[0].units[unitId];
            const float radius=prototype.collisionSize[0],clearance=prototype.clearanceSize[0];
            const float offset=radius>int(radius)?.5f:0.f;
            const int x=tile%width,y=tile/width,low=int(std::floor(offset-clearance)),high=int(std::floor(offset+clearance-.001f))+1;
            if(x+low<0 || y+low<0 || x+high>int(width) || y+high>int(height))return false;
            for(int yy=y+low;yy<y+high;++yy)for(int xx=x+low;xx<x+high;++xx)
                if((!crater && !vent && !aquatic && occupied[yy*width+xx]) || restriction[terrain[yy*width+xx]]<=.05f)return false;
            if(unitId==1295) {
                // Original48e697 checks only the left column and last row of the footprint, within one height of their shared corner.
                const int reference=(*elevation)[(y+high-1)*width+x+low];
                for(int yy=y+low;yy<y+high;++yy)for(int xx=x+low;xx<x+high;++xx)
                    if((xx==x+low || yy==y+high-1) && std::abs(int((*elevation)[yy*width+xx])-reference)>1)return false;
            }
            return true;
        }
        if(!objectRow && unitId==1585 && terrain[tile]!=22)return false;
        if(unitId==69) {
            const int x=tile%width,y=tile/width;
            // Original48e352 also accepts ice beach37 when a side terrain is ice35.
            auto shore=[&](int at){return terrain[at]==2 || terrain[at]==35 || terrain[at]==37;};
            if(!((x>0 && shore(tile-1)) || (x+1<int(width) && shore(tile+1)) ||
                 (y>0 && shore(tile-int(width))) || (y+1<int(height) && shore(tile+int(width)))))return false;
        }
        if((bigHunt || unitId==1471 || unitId==1203) && data.civs[0].units[unitId].clearanceSize[0]>=1.f) {
            const int x=tile%width,y=tile/width;
            if(x==0 || y==0 || x+1==int(width) || y+1==int(height))return false;
            for(int yy=y-1;yy<=y+1;++yy)
                for(int xx=x-1;xx<=x+1;++xx)
                    if(occupied[yy*width+xx] || restriction[terrain[yy*width+xx]]<=.05f)return false;
        }
        return (bird || aquatic || unitId==1120 || unitId==1122 || unitId==1123 || !occupied[tile]) && restriction[terrain[tile]]>.05f;
    };
    auto create=[&](int tile) {
        RmsTimberPlacement placement{float(tile%width)+(largeFish?0.f:.5f),float(tile/width)+(largeFish?0.f:.5f),0,0,unitId};
        if(mountain || crater || stalagmite || vent || building || unitId==1296) {
            // Original snaps by fractional radius; static sequence0 graphics consume no draws.
            const float radius=data.civs[0].units[unitId].collisionSize[0],offset=radius>int(radius)?.5f:0.f;
            placement.x=float(tile%width)+offset;placement.y=float(tile/width)+offset;
            if(unitId==1296)genieRandomDraw(rng);
            if(building) {
                // Original5cfad0 reseeds after completion; sequence0 temple skips sprite draws.
                for(int draw=0;draw<(unitId==1295?3:mine?8:6);++draw)genieRandomDraw(rng);
                rng=genieRandomDraw(rng);
            }
        } else if(bird) {
            const uint32_t heading=genieRandomDraw(rng)*8u/32767u;
            placement.headingRadians=float(double(heading)*3.14159265358979323846/4.);
            placement.facet=uint8_t((heading+7)%8);
        } else if(herd || aquatic) {
            const auto &prototype=data.civs[0].units[unitId];
            if(prototype.speed>1.e-7f) {
                // Original4a1440 leaves moving objects .01 tiles inside their collision bounds.
                placement.x=std::clamp(placement.x,prototype.collisionSize[0]+.01f,float(width)-prototype.collisionSize[0]-.01f);
                placement.y=std::clamp(placement.y,prototype.collisionSize[1]+.01f,float(height)-prototype.collisionSize[1]-.01f);
            }
            // Base facet, animated initial frame/phase, heading, then three action timers.
            genieRandomDraw(rng);genieRandomDraw(rng);
            const auto &g=*data.graphic(spec->graphic);
            if(g.sequenceType&1) {
                const float cycle=g.frameCount*g.frameDuration+g.replayDelay;
                const double elapsed=double(genieRandomDraw(rng))*cycle*(1.f/32767.f);
                const int frame=int(elapsed/g.frameDuration);
                placement.animationFrame=uint8_t(std::min(int(g.frameCount)-1,frame));
                if(frame>=g.frameCount)placement.initialIdleDelay=std::max(0.f,float(elapsed-double(g.frameCount-1)*g.frameDuration)-g.frameDuration);
            }
            const uint32_t heading=(aquatic || unitId==1465)?0:genieRandomDraw(rng)*8u/32767u;
            placement.headingRadians=float(double(heading)*3.14159265358979323846/4.);
            placement.facet=uint8_t((heading+7)%8);
            if(!aquatic)for(int timer=0;timer<((hunt || predator) && unitId!=1465?4:3);++timer)genieRandomDraw(rng);
        } else if(spec->type==20) {
            // Original54fe56 clears the base facet after sprite creation consumes its frame draw.
            genieRandomDraw(rng);genieRandomDraw(rng);
        } else if(spec->type==30) {
            const auto appearance=drawRmsCarbonAppearance(rng,static_cast<uint8_t>(spec->angles));
            placement.facet=appearance.facet;placement.headingRadians=appearance.headingRadians;
        } else placement.facet=static_cast<uint8_t>(std::min(unsigned(spec->angles-1),genieRandomDraw(rng)*unsigned(spec->angles)/32767u));
        result.push_back(placement);
        if(building || unitId==1296) {
            const float radius=data.civs[0].units[unitId].collisionSize[0];
            for(int yy=int(std::floor(placement.y-radius));yy<int(std::ceil(placement.y+radius));++yy)
                for(int xx=int(std::floor(placement.x-radius));xx<int(std::ceil(placement.x+radius));++xx)
                    if(xx>=0 && yy>=0 && xx<int(width) && yy<int(height))occupied[yy*width+xx]=true;
        } else if(!data.civs[0].units[unitId].canBeBuiltOn && !herd && !bird && !aquatic && unitId!=1120 && unitId!=1122 && unitId!=1123 && !crater)occupied[tile]=true;
    };
    RmsZoneCache resourceZones;
    for(size_t player=0;player<(perPlayer?novaPlayerStarts.size():1);++player) {
        const int startX=perPlayer?novaPlayerStarts[player].x:-1;
        const int startY=perPlayer?novaPlayerStarts[player].y:-1;
        if(!build(outer,perPlayer && playerMax>=0?startX:-1,perPlayer && playerMax>=0?startY:-1,perPlayer?playerMax:-1)) return fail("work limit exceeded");
        const RmsZones *zones=nullptr;
        uint8_t centerZone=0;
        if(perPlayer || (objectRow && zoneDistance>0)) {
            auto connectivity=restriction;
            if(perPlayer)connectivity[terrain[startY*width+startX]]=1;
            uint32_t index;
            if(!resourceZones.lookup(width,terrain,connectivity,index,err)) return false;
            zones=&resourceZones.entry(index)->zones;
            if(perPlayer)centerZone=zones->tiles[startY*width+startX];
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
            if(perPlayer && !sameZone(x,y)) continue;
            if(zones) {
                // Global rows compare neighbors to the candidate's own zone.
                if(!perPlayer)centerZone=zones->tiles[center];
                // Original4df2f0 samples cardinal A and diagonal trunc(A*10/14).
                const int diagonal=zoneDistance*10/14;
                if(zoneDistance>0 && (!sameZone(x,y-zoneDistance) || !sameZone(x+diagonal,y-diagonal) ||
                    !sameZone(x+zoneDistance,y) || !sameZone(x+diagonal,y+diagonal) ||
                    !sameZone(x,y+zoneDistance) || !sameZone(x-diagonal,y+diagonal) ||
                    !sameZone(x-zoneDistance,y) || !sameZone(x-diagonal,y-diagonal))) continue;
            }
            if(objectRow && objectRow->terrain>=0 && terrain[center]!=objectRow->terrain) continue;
            if(objectRow?group.mode==0:(((predator || bigHunt) && group.mode==0) || tree || bird || aquatic)) {
                // Original mode0 has no inner queue/variance draw. A failed final gate
                // forces the last candidate (4dec9c), after distance/zone exclusions.
                const bool forced=!pass(center);
                if(forced && ((objectRow?!perPlayer:bool(globalPlacement) || aquatic) || next[outer]>=0)) continue;
                if(objectRow) {
                    if(!removeSpacing(x,y,playerSpacing,false) ||
                       (!forced && !removeSpacing(x,y,temporarySpacing,false))) return fail("work limit exceeded");
                    create(center);
                    if(playerSpacing && !removeSpacing(x,y,playerSpacing,true)) return fail("work limit exceeded");
                } else if(tree) {
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
            // Temporary spacing removes current queue nodes, not future-row eligibility.
            if(!removeSpacing(x,y,playerSpacing,false) ||
               !removeSpacing(x,y,temporarySpacing,false)) return fail("work limit exceeded");
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
                if(group.mode==1 && ((objectRow && objectRow->terrain>=0 && terrain[tile]!=objectRow->terrain) || !pass(tile))) continue;
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
            if(playerSpacing && !removeSpacing(x,y,playerSpacing,true)) return fail("work limit exceeded");
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
    const std::vector<RmsPlayerStart> &landCenters,uint32_t &randomState,
    std::vector<RmsTimberPlacement> &out,std::string *err,
    const std::vector<uint32_t> &occupiedTiles,std::vector<uint8_t> *rowEligibility,
    const RmsPlayerStart *selectedLand,const RmsObjectRow *objectRow) {
    out.clear();if(err) err->clear();
    auto fail=[&](const char *reason) {
        if(err) *err=std::string("RMS holocron: ")+reason;
        return false;
    };
    if((width!=64 && width!=120) || height!=width || terrain.size()!=size_t(width)*height || (landCenters.size()!=2 && (width!=120 || (landCenters.size()<3 || landCenters.size()>5))))
        return fail("requires64/120-square terrain and two land centers (or three to five at120)");
    for(const auto &start:landCenters)
        if(start.x<2 || start.y<2 || start.x>int(width)-2 || start.y>int(height)-2) return fail("invalid land center");
    if(std::any_of(terrain.begin(),terrain.end(),[&](uint8_t t) {
        return t!=0 && t!=27 && (width!=120 || (t!=1 && t!=2 && t!=4 && t!=5 && t!=6 && t!=9 && t!=10 && t!=12 && t!=18 && t!=20 && t!=21 && t!=22 && t!=23 && t!=24 && t!=25 && t!=38 && t!=41 && t!=44 && t!=14 && t!=45 && t!=46 && t!=50 && t!=19 && t!=26 && t!=32 && t!=33 && t!=34 && t!=36 && t!=49 && t!=13 && t!=35 && t!=37 && t!=47 && t!=16 && t!=42));
    }) || (width==64 && !occupiedTiles.empty()))
        return fail("requires verified grass or120-square forest terrain");
    if(rowEligibility && !rowEligibility->empty() &&
       (rowEligibility->size()!=terrain.size() ||
        std::any_of(rowEligibility->begin(),rowEligibility->end(),[](uint8_t value){return value>1;})))
        return fail("invalid persistent row eligibility");
    const bool arena=objectRow!=nullptr;
    if(arena && (width!=120 || landCenters.size()!=3 || selectedLand || objectRow->unitId!=285 ||
       (objectRow->terrain!=6 && objectRow->terrain!=9 && objectRow->terrain!=12) || objectRow->mode || objectRow->scaling ||
       objectRow->groups!=5 || objectRow->perGroup!=1 || objectRow->variance || objectRow->radius!=3 || objectRow->owner!=-1 ||
       objectRow->landId!=-2 || objectRow->minDistance!=30 || objectRow->maxDistance!=-1 || objectRow->spacing!=5 ||
       objectRow->temporarySpacing || objectRow->zoneDistance || objectRow->matchPlayerCiv))return fail("unsupported Arena holocron row");
    std::vector<uint8_t> eligible;
    if(arena)eligible=(rowEligibility && !rowEligibility->empty())?*rowEligibility:std::vector<uint8_t>(terrain.size(),1);
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
    RmsZoneCache zones;const RmsZones *selectedZones=nullptr;uint8_t selectedZone=0;
    if(selectedLand) {
        if(width!=120 || selectedLand->x<0 || selectedLand->y<0 || selectedLand->x>=int(width) || selectedLand->y>=int(height))
            return fail("selected land outside verified map");
        auto connectivity=pass;connectivity[terrain[selectedLand->y*width+selectedLand->x]]=1;
        uint32_t index;
        if(!zones.lookup(width,terrain,connectivity,index,err))return false;
        selectedZones=&zones.entry(index)->zones;selectedZone=selectedZones->tiles[selectedLand->y*width+selectedLand->x];
    }
    uint32_t rng=randomState;std::vector<uint32_t> candidates;
    if(!generateRmsObjectCandidates(width,height,rng,candidates,err)) return false;
    std::vector<bool> removed(terrain.size());std::vector<RmsTimberPlacement> placements;
    size_t lastQueued=candidates.size();
    for(size_t index=0;index<candidates.size();++index) {
        const auto tile=candidates[index];
        if((rowEligibility && !rowEligibility->empty() && !(*rowEligibility)[tile]) || removed[tile] ||
           (selectedZones && selectedZones->tiles[tile]!=selectedZone)) continue;
        const int x=tile%width,y=tile/width;
        if(std::any_of(landCenters.begin(),landCenters.end(),[&](const auto &start) {
            return std::abs(x-start.x)<(arena?30:25) && std::abs(y-start.y)<(arena?30:25);
        })) continue;
        if(arena && terrain[tile]!=objectRow->terrain)continue;
        if(occupied[tile] || pass[terrain[tile]]<=.05f) {
            if(!selectedLand)continue;
            // Original4dec9c forces the last queued selected-land candidate after collision rejection.
            while(lastQueued>index+1 && (removed[candidates[lastQueued-1]] ||
                  (rowEligibility && !rowEligibility->empty() && !(*rowEligibility)[candidates[lastQueued-1]])))--lastQueued;
            if(index+1!=lastQueued)continue;
        }
        // Original4de1b0 removes queue nodes; permanent spacing also masks passable future-row cells.
        const int spacing=arena?5:20;
        for(int sy=std::max(0,y-spacing);sy<=std::min(int(height)-1,y+spacing);++sy)
            for(int sx=std::max(0,x-spacing);sx<=std::min(int(width)-1,x+spacing);++sx) {
                removed[sy*width+sx]=true;
                if(arena && pass[terrain[sy*width+sx]]>0)eligible[sy*width+sx]=0;
            }
        // Static type70: base facet, sprite frame, three action timers, military timer.
        for(int draw=0;draw<6;++draw) genieRandomDraw(rng);
        placements.push_back({x+.5f,y+.5f,7,0,285});
        if(placements.size()==5) break;
    }
    if(arena && rowEligibility)*rowEligibility=std::move(eligible);
    out=std::move(placements);randomState=rng;return true;
}
} // namespace swgb
