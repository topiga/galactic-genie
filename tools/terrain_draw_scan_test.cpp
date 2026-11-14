// SPDX-License-Identifier: GPL-3.0-or-later
#include "engine/terrain_draw_scan.h"
#include <cassert>
#include <cstdint>
#include <cstdio>
int main() {
    // Captured original 60dbe0 visits on24x24; all pixel rectangles were clipped.
    const struct { int x,y,columns,rows; uint32_t hash; } cases[]={
        {9,11,1,1,226802224u},
        {17,19,1,1,3846980733u},
        {1,3,1,1,994946954u},
        {9,13,3,2,1391382113u},
    };
    for (const auto &f:cases) {
        uint32_t hash=2166136261u;
        for(int y=0;y<24;++y) for(int x=0;x<24;++x)
            hash=(hash^uint32_t(swgb::terrainDrawScanContains(f.x,f.y,f.columns,f.rows,x,y)))*16777619u;
        assert(hash==f.hash);
    }
    std::puts("Original padded terrain-dispatch viewport fixtures passed");
}
