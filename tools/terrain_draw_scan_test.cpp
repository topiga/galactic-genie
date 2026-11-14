// SPDX-License-Identifier: GPL-3.0-or-later
#include "engine/terrain_draw_scan.h"
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <algorithm>
#include <vector>
#include <array>
int main() {
    // Captured original 60dbe0 visits on24x24; all pixel rectangles were clipped.
    const struct { int x,y,columns,rows; uint32_t hash,orderHash; } cases[]={
        {9,11,1,1,226802224u,3835727641u},
        {17,19,1,1,3846980733u,1362288911u},
        {1,3,1,1,994946954u,445081256u},
        {9,13,3,2,1391382113u,2576578549u},
    };
    for (const auto &f:cases) {
        uint32_t hash=2166136261u;
        for(int y=0;y<24;++y) for(int x=0;x<24;++x)
            hash=(hash^uint32_t(swgb::terrainDrawScanContains(f.x,f.y,f.columns,f.rows,x,y)))*16777619u;
        assert(hash==f.hash);
        std::vector<std::array<int,2>> visits;
        for(int y=0;y<24;++y) for(int x=0;x<24;++x)
            if(swgb::terrainDrawScanContains(f.x,f.y,f.columns,f.rows,x,y)) visits.push_back({x,y});
        std::sort(visits.begin(),visits.end(),[](const auto &a,const auto &b) {
            return swgb::terrainDrawTileBefore(a[0],a[1],b[0],b[1]);
        });
        uint32_t orderHash=2166136261u;
        for(const auto &v:visits) orderHash=(orderHash^uint32_t(v[1]*24+v[0]))*16777619u;
        assert(orderHash==f.orderHash);
    }
    std::puts("Original padded terrain-dispatch viewport fixtures passed");
}
