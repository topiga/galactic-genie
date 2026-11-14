// SPDX-License-Identifier: GPL-3.0-or-later
#include "rms_zones.h"
#include <algorithm>
#include <cmath>

namespace swgb {
bool generateRmsZones(uint32_t size,const std::vector<uint8_t> &terrain,
    const std::vector<float> &passability,RmsZones &out,std::string *err) {
    out={};
    if (err) err->clear();
    auto fail=[&](const char *reason) { if(err) *err=reason; return false; };
    if (!size || size>1024 || terrain.size()!=size_t(size)*size)
        return fail("RMS zones require a square map of size1..1024");
    for (float value:passability) if (!std::isfinite(value))
        return fail("RMS zones require finite passability values");
    for (uint8_t tile:terrain) if (tile>=passability.size())
        return fail("RMS zone terrain is outside the passability table");
    RmsZones result;
    result.tiles.assign(terrain.size(),255);
    std::vector<uint32_t> pending;
    uint32_t components=0;
    for (uint32_t seed=0;seed<terrain.size();++seed) {
        if (result.tiles[seed]!=255) continue;
        const uint8_t zone=uint8_t(components++%255);
        const bool positive=passability[terrain[seed]]>0;
        result.tiles[seed]=zone;
        pending.push_back(seed);
        while (!pending.empty()) {
            const uint32_t tile=pending.back();pending.pop_back();
            const uint32_t x=tile%size,y=tile/size;
            auto visit=[&](uint32_t next) {
                if (result.tiles[next]==255 && (passability[terrain[next]]>0)==positive) {
                    result.tiles[next]=zone;pending.push_back(next);
                }
            };
            if (x) visit(tile-1);
            if (x+1<size) visit(tile+1);
            if (y) visit(tile-size);
            if (y+1<size) visit(tile+size);
        }
    }
    result.count=std::min(components,255u);
    out=std::move(result);
    return true;
}
}
