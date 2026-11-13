// SPDX-License-Identifier: GPL-3.0-or-later
#include "rms_early_forest.h"
#include "../core/genie_dat.h"
#include "../core/genie_random.h"
#include <algorithm>
#include <cmath>

namespace swgb {
namespace {
bool fail(std::vector<RmsSceneryPlacement> &out, std::string *err, const char *message) {
    out.clear();
    if (err) *err=std::string("RMS early forest: ")+message;
    return false;
}
bool validForest(const dat::DatFile &data, uint8_t base, int16_t &unit, uint32_t &angles, std::string *err) {
    if (base!=10 && base!=13 && (base<17 || base>21)) return false;
    uint32_t rng=1;
    std::vector<RmsSceneryPlacement> probe;
    if (!generateRmsScenery(1,1,{base},data,rng,probe,err)) return false;
    if (probe.size()!=1) return false;
    unit=probe[0].unitId;
    const auto &prototype=data.civs[0].units[unit];
    angles=data.graphic(prototype.standingGraphic[0])->angleCount;
    return prototype.type==10 && prototype.createDoppelgangerOnDeath==2;
}
}
bool generateRmsEarlyForest(uint32_t width, uint32_t height,
    const std::vector<uint32_t> &paintedTiles, const dat::DatFile &data,
    uint32_t &randomState, std::vector<RmsSceneryPlacement> &out, std::string *err, uint8_t baseTerrain) {
    out.clear(); if (err) err->clear();
    if (!width || !height || width>1024 || height>1024)
        return fail(out,err,"invalid dimensions");
    int16_t unit=-1; uint32_t angles=0;
    if (!validForest(data,baseTerrain,unit,angles,err)) return fail(out,err,"unsupported forest prototype");
    std::vector<bool> seen(size_t(width)*height);
    for (uint32_t tile:paintedTiles) {
        if (tile>=seen.size() || seen[tile]) return fail(out,err,"invalid or repeated paint event");
        seen[tile]=true;
    }
    uint32_t rng=randomState;
    for (uint32_t tile:paintedTiles) {
        const auto facet=static_cast<uint8_t>(std::min(angles-1,genieRandomDraw(rng)*angles/32767u));
        out.push_back({unit,float(tile%width)+.5f,float(tile/width)+.5f,facet});
    }
    randomState=rng;
    return true;
}
bool finishRmsEarlyForest(uint32_t width, uint32_t height,
    const std::vector<uint8_t> &terrain, const dat::DatFile &data,
    const std::vector<RmsSceneryPlacement> &early, uint32_t &randomState,
    std::vector<RmsSceneryPlacement> &out, std::string *err, uint8_t baseTerrain) {
    if (err) err->clear();
    if (!width || !height || width>1024 || height>1024 || terrain.size()!=size_t(width)*height)
        return fail(out,err,"invalid dimensions or terrain size");
    int16_t unit=-1; uint32_t angles=0;
    if (!validForest(data,baseTerrain,unit,angles,err)) return fail(out,err,"unsupported forest prototype");
    for (uint8_t id:terrain) {
        if (id>=data.terrainBlock.terrains.size()) return fail(out,err,"invalid terrain ID");
        const auto &t=data.terrainBlock.terrains[id];
        if (t.numTerrainUnitsUsed<0 || t.numTerrainUnitsUsed>30)
            return fail(out,err,"invalid scenery slot count");
        if (id!=baseTerrain) for (int slot=0;slot<t.numTerrainUnitsUsed;++slot)
            if (t.terrainUnitId[slot]!=-1) return fail(out,err,"other active terrain is unsupported");
    }
    auto result=early;
    std::vector<bool> seen(terrain.size());
    for (auto &object:result) {
        const auto &p=object;
        if (!object.alive || p.unitId!=unit || !std::isfinite(p.x) || !std::isfinite(p.y) ||
            p.x<.5f || p.y<.5f || p.x>=width || p.y>=height ||
            p.x!=std::floor(p.x)+.5f || p.y!=std::floor(p.y)+.5f || p.facet>=angles || p.headingRadians!=0)
            return fail(out,err,"invalid early object");
        const size_t tile=size_t(p.y)*width+size_t(p.x);
        if (seen[tile]) return fail(out,err,"duplicate early object");
        seen[tile]=true;
        if (terrain[tile]==baseTerrain) object.alive=false;
    }
    uint32_t rng=randomState;
    std::vector<RmsSceneryPlacement> generated;
    if (!generateRmsScenery(width,height,terrain,data,rng,generated,err)) { out.clear(); return false; }
    for (const auto &p:generated) result.push_back(p);
    out=std::move(result); randomState=rng;
    return true;
}
}
