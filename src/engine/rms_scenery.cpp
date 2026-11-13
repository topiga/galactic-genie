// SPDX-License-Identifier: GPL-3.0-or-later
#include "rms_scenery.h"
#include "../core/genie_dat.h"
#include "../core/genie_random.h"
#include <algorithm>
#include <cmath>

namespace swgb {
bool generateRmsScenery(uint32_t width, uint32_t height,
    const std::vector<uint8_t> &terrain, const dat::DatFile &data,
    uint32_t &randomState, std::vector<RmsSceneryPlacement> &out, std::string *err) {
    out.clear();
    if (err) err->clear();
    auto fail = [&](const char *reason) {
        if (err) *err = std::string("RMS scenery: ") + reason;
        return false;
    };
    if (!width || !height || width>1024 || height>1024 ||
        terrain.size()!=static_cast<size_t>(width)*height)
        return fail("invalid dimensions or terrain size");
    struct TreeSpec { int terrain, alias, unit, graphic, restriction, angles, carbon; };
    static constexpr TreeSpec trees[]={
        {10,-1,411,5292,4,12,100}, {13,11,351,5295,11,12,100},
        {17,11,414,5286,11,12,100}, {18,10,348,5282,11,12,100},
        {19,11,350,5298,11,12,100}, {20,10,349,5289,11,11,100}, {21,10,413,5301,11,12,125}};
    bool checked[256]={};
    int unitIds[256], angleCounts[256]={};
    std::fill(std::begin(unitIds),std::end(unitIds),-1);
    for (uint8_t tile:terrain) {
        if (checked[tile]) continue;
        checked[tile]=true;
        if (tile>55 || tile>=data.terrainBlock.terrains.size()) return fail("unsupported terrain ID");
        const auto &t=data.terrainBlock.terrains[tile];
        if (t.enabled!=1 || t.numTerrainUnitsUsed<0 || t.numTerrainUnitsUsed>30)
            return fail("unsupported terrain metadata");
        if (t.terrainToDraw!=-1) {
            const int alias=t.terrainToDraw;
            const bool known=(tile==5 && alias==10) || (tile==13 && alias==11) ||
                (tile==17 && alias==11) || (tile==18 && alias==10) ||
                (tile==19 && alias==11) || (tile==20 && alias==10) ||
                (tile==21 && alias==10) || (tile==28 && alias==1) || (tile==36 && alias==33) ||
                (tile==37 && alias==35) || (tile==40 && alias==38);
            if (!known || alias<0 || static_cast<size_t>(alias)>=data.terrainBlock.terrains.size())
                return fail("unsupported terrain alias");
            const auto &target=data.terrainBlock.terrains[alias];
            if (target.enabled!=1 || target.terrainToDraw!=-1)
                return fail("unsupported terrain alias target");
            // Automatic scenery reads SOURCE slots; the visual alias target has no effect.
        }
        for (int slot=0;slot<t.numTerrainUnitsUsed;++slot) {
            const int id=t.terrainUnitId[slot];
            if (id==-1) continue;
            if (id<0) return fail("invalid terrain scenery unit ID");
            if (data.civs.empty() || static_cast<size_t>(id)>=data.civs[0].units.size())
                return fail("invalid terrain scenery prototype lookup");
            if (!data.civs[0].units[id].exists || t.terrainUnitDensity[slot]<=0) continue;
            const TreeSpec *match=nullptr;
            for (const auto &candidate:trees) if (candidate.terrain==tile) match=&candidate;
            if (!match || t.terrainToDraw!=match->alias || slot!=0 || t.numTerrainUnitsUsed!=1 ||
                id!=match->unit || t.terrainUnitDensity[slot]!=1000 || t.terrainUnitCentering[slot]!=1)
                return fail("unsupported automatic scenery slot");
            const auto &spec=*match;
            const auto &u=data.civs[0].units[id];
            const auto *g=data.graphic(spec.graphic);
            if (!u.exists || u.id!=id || u.type!=10 || u.cls!=31 || u.hitPoints!=50 ||
                u.standingGraphic[0]!=spec.graphic || u.standingGraphic[1]!=-1 ||
                u.collisionSize[0]!=.5f || u.collisionSize[1]!=.5f || u.collisionSize[2]!=2.f ||
                u.clearanceSize[0]!=.5f || u.clearanceSize[1]!=.5f ||
                u.placementSideTerrain[0]!=-1 || u.placementSideTerrain[1]!=-1 ||
                u.placementTerrain[0]!=-1 || u.placementTerrain[1]!=-1 ||
                u.terrainRestriction!=spec.restriction || u.hillMode || u.flyMode || u.canBeBuiltOn ||
                u.recyclable || u.lineOfSight!=0 || u.trainSound!=-1 ||
                u.obstructionType!=2 || u.obstructionClass!=1 || u.civilization ||
                u.enableAutoGather!=1 || !g || g->sequenceType!=6 || g->angleCount!=spec.angles)
                return fail("unsupported tree prototype or graphic");
            int carbonStores=0;
            for (const auto &storage:u.resourceStorages) {
                if (storage.flag || (storage.type>=0 && (storage.type!=1 || storage.amount!=spec.carbon)))
                    return fail("unsupported tree resource storage");
                carbonStores+=storage.type==1;
            }
            if (carbonStores!=1) return fail("expected one carbon storage");
            if (data.terrainRestrictions.size()<=static_cast<size_t>(spec.restriction) ||
                data.terrainRestrictions[spec.restriction].passableBuildableDmgMultiplier.size()<=tile)
                return fail("missing tree terrain restriction");
            const float passability=data.terrainRestrictions[spec.restriction].passableBuildableDmgMultiplier[tile];
            if (!std::isfinite(passability)) return fail("invalid tree terrain restriction");
            if (passability>.05f) { unitIds[tile]=id; angleCounts[tile]=spec.angles; }
        }
    }
    uint32_t rng=randomState;
    for (size_t tile=0;tile<terrain.size();++tile) {
        const int id=unitIds[terrain[tile]], angles=angleCounts[terrain[tile]];
        if (id<0) continue;
        // Density1000/centering1 consumes no placement draws; only the sprite facet draws.
        const auto facet=static_cast<uint8_t>(std::min(static_cast<uint32_t>(angles-1),genieRandomDraw(rng)*static_cast<uint32_t>(angles)/32767u));
        out.push_back({static_cast<int16_t>(id),float(tile%width)+.5f,float(tile/width)+.5f,facet});
    }
    randomState=rng;
    return true;
}
} // namespace swgb
