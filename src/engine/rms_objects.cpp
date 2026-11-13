// SPDX-License-Identifier: GPL-3.0-or-later
#include "rms_objects.h"
#include "../core/genie_random.h"
#include "../core/genie_dat.h"
#include <algorithm>
#include <cmath>

namespace swgb {
bool generateRmsObjectCandidates(uint32_t width,uint32_t height,uint32_t &randomState,
                                 std::vector<uint32_t> &out,std::string *err) {
    out.clear();
    if (err) err->clear();
    if (!width || !height || width>1024 || height>1024) {
        if (err) *err="RMS object candidates: dimensions must be 1..1024";
        return false;
    }
    const uint32_t count=width*height, sentinel=count;
    std::vector<uint32_t> next(count+1),previous(count+1);
    for (uint32_t i=0;i<count;++i) {
        next[i]=i+1;
        previous[i]=i?i-1:sentinel;
    }
    next[sentinel]=0;
    previous[sentinel]=count-1;
    uint32_t rng=randomState;
    if (width>3 && height>3) {
        const uint32_t dx=width-2,dy=height-2;
        for (uint32_t i=0;i<dx*dy/4;++i) {
            const uint32_t x=genieRandomDraw(rng)*dx/32767u;
            const uint32_t y=genieRandomDraw(rng)*dy/32767u;
            const uint32_t tile=y*width+x;
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
    for (uint32_t tile=next[sentinel];tile!=sentinel;tile=next[tile]) out.push_back(tile);
    randomState=rng;
    return true;
}

bool generateRmsTimberPlacements(uint32_t width, uint32_t height,
    const std::vector<uint8_t> &terrain, const dat::DatFile &data,
    uint32_t requested, uint32_t &randomState,
    std::vector<RmsTimberPlacement> &out, std::string *err,
    const std::vector<uint32_t> &occupiedTiles) {
    out.clear();
    if (err) err->clear();
    auto fail = [&](const char *reason) {
        if (err) *err = std::string("RMS timber: ") + reason;
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
    if (data.civs.empty() || data.civs[0].units.size() <= 348)
        return fail("missing Gaia timber prototype");
    const auto &u = data.civs[0].units[348];
    const auto *g = data.graphic(5282);
    if (!u.exists || u.id != 348 || u.type != 10 || u.cls != 31 ||
        u.standingGraphic[0] != 5282 || u.standingGraphic[1] != -1 || u.hitPoints != 50 ||
        u.collisionSize[0] != .5f ||
        u.collisionSize[1] != .5f || u.collisionSize[2] != 2.f ||
        u.clearanceSize[0] != .5f || u.clearanceSize[1] != .5f ||
        u.placementSideTerrain[0] != -1 || u.placementSideTerrain[1] != -1 ||
        u.placementTerrain[0] != -1 || u.placementTerrain[1] != -1 ||
        u.terrainRestriction != 11 || u.hillMode || u.flyMode || u.canBeBuiltOn ||
        u.recyclable || u.lineOfSight != 0 || u.trainSound != -1 ||
        u.obstructionType != 2 || u.obstructionClass != 1 || u.civilization ||
        u.enableAutoGather != 1 || !g || g->sequenceType != 6 || g->angleCount != 12)
        return fail("unsupported timber prototype or graphic");
    int carbonStores = 0;
    for (const auto &storage : u.resourceStorages) {
        if (storage.flag || (storage.type >= 0 && (storage.type != 1 || storage.amount != 100)))
            return fail("unsupported resource storage");
        carbonStores += storage.type == 1;
    }
    if (carbonStores != 1) return fail("expected one carbon storage");
    if (data.terrainRestrictions.size() <= 11) return fail("missing terrain restriction");
    const auto &restriction = data.terrainRestrictions[11].passableBuildableDmgMultiplier;
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
        const auto facet = static_cast<uint8_t>(std::min(11u,genieRandomDraw(rng)*12u/32767u));
        out.push_back({float(tile%width)+.5f,float(tile/width)+.5f,facet});
    }
    randomState = rng;
    return true;
}
} // namespace swgb
