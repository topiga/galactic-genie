// SPDX-License-Identifier: GPL-3.0-or-later
#include "rms_terrain_cleanup.h"
#include <algorithm>

namespace swgb {
bool cleanupRmsTerrain(uint32_t width, uint32_t height,
    std::vector<uint8_t> &terrain, std::string *err) {
    if (err) err->clear();
    if (!width || !height || width>1024 || height>1024 ||
        terrain.size()!=static_cast<size_t>(width)*height) {
        if (err) *err="RMS terrain cleanup: invalid dimensions or raster size";
        return false;
    }
    auto water=[](uint8_t terrain) { return terrain==1 || terrain==4 || terrain==22 || terrain==23; };
    const int w=static_cast<int>(width), h=static_cast<int>(height);
    for (int y=0;y<h;++y) for (int x=0;x<w;++x) {
        auto &tile=terrain[y*w+x];
        if (water(tile) || tile==2) continue;
        bool adjacent=false;
        for (int ny=std::max(0,y-1);ny<=std::min(h-1,y+1) && !adjacent;++ny)
            for (int nx=std::max(0,x-1);nx<=std::min(w-1,x+1);++nx)
                if (water(terrain[ny*w+nx])) { adjacent=true; break; }
        if (adjacent) tile=((tile>=32 && tile<=35) || tile==37 || tile==47) ? 37 : 2;
    }
    return true;
}
} // namespace swgb
