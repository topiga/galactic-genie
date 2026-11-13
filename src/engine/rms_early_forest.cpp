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
bool validForest(const dat::DatFile &data, std::string *err) {
    uint32_t rng=1;
    std::vector<RmsSceneryPlacement> probe;
    if (!generateRmsScenery(1,1,{10},data,rng,probe,err)) return false;
    return probe.size()==1 && probe[0].unitId==411 &&
        data.civs[0].units[411].createDoppelgangerOnDeath==2;
}
}
bool generateRmsEarlyForest(uint32_t width, uint32_t height,
    const std::vector<uint32_t> &paintedTiles, const dat::DatFile &data,
    uint32_t &randomState, std::vector<RmsSceneryPlacement> &out, std::string *err) {
    out.clear(); if (err) err->clear();
    if (!width || !height || width>1024 || height>1024)
        return fail(out,err,"invalid dimensions");
    if (!validForest(data,err)) return fail(out,err,"unsupported forest prototype");
    std::vector<bool> seen(size_t(width)*height);
    for (uint32_t tile:paintedTiles) {
        if (tile>=seen.size() || seen[tile]) return fail(out,err,"invalid or repeated paint event");
        seen[tile]=true;
    }
    uint32_t rng=randomState;
    for (uint32_t tile:paintedTiles) {
        const auto facet=static_cast<uint8_t>(std::min(11u,genieRandomDraw(rng)*12u/32767u));
        out.push_back({411,float(tile%width)+.5f,float(tile/width)+.5f,facet});
    }
    randomState=rng;
    return true;
}
bool finishRmsEarlyForest(uint32_t width, uint32_t height,
    const std::vector<uint8_t> &terrain, const dat::DatFile &data,
    const std::vector<RmsSceneryPlacement> &early, uint32_t &randomState,
    std::vector<RmsSceneryPlacement> &out, std::string *err) {
    if (err) err->clear();
    if (!width || !height || width>1024 || height>1024 || terrain.size()!=size_t(width)*height)
        return fail(out,err,"invalid dimensions or terrain size");
    if (!validForest(data,err)) return fail(out,err,"unsupported forest prototype");
    for (uint8_t id:terrain) {
        if (id>=data.terrainBlock.terrains.size()) return fail(out,err,"invalid terrain ID");
        const auto &t=data.terrainBlock.terrains[id];
        if (t.numTerrainUnitsUsed<0 || t.numTerrainUnitsUsed>30)
            return fail(out,err,"invalid scenery slot count");
        if (id!=10) for (int slot=0;slot<t.numTerrainUnitsUsed;++slot)
            if (t.terrainUnitId[slot]!=-1) return fail(out,err,"other active terrain is unsupported");
    }
    auto result=early;
    std::vector<bool> seen(terrain.size());
    for (auto &object:result) {
        const auto &p=object;
        if (!object.alive || p.unitId!=411 || !std::isfinite(p.x) || !std::isfinite(p.y) ||
            p.x<.5f || p.y<.5f || p.x>=width || p.y>=height ||
            p.x!=std::floor(p.x)+.5f || p.y!=std::floor(p.y)+.5f || p.facet>11 || p.headingRadians!=0)
            return fail(out,err,"invalid early object");
        const size_t tile=size_t(p.y)*width+size_t(p.x);
        if (seen[tile]) return fail(out,err,"duplicate early object");
        seen[tile]=true;
        if (terrain[tile]==10) object.alive=false;
    }
    uint32_t rng=randomState;
    std::vector<RmsSceneryPlacement> generated;
    if (!generateRmsScenery(width,height,terrain,data,rng,generated,err)) { out.clear(); return false; }
    for (const auto &p:generated) result.push_back(p);
    out=std::move(result); randomState=rng;
    return true;
}
}
