// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>
namespace swgb {
struct AiBuildPlacementResult {
    int x=-1, y=-1, score=-1;
};
// The exhaustive category10 (prefab) scan in original588b30. The caller must
// supply its prepared original scoring grid and master placement validator.
// This does not substitute a native walkability grid, choose a builder, derive
// search bounds, resume a timed scan, or implement other placement categories.
// Validator receives half-tile coordinates even for an even-sized footprint;
// final result allocation converts even-sized footprints to integer positions.
template<class Grid, class Random, class Blacklisted, class Validator>
AiBuildPlacementResult aiScanPrefabPlacement(int mapWidth,int mapHeight,
    int minX,int minY,int maxX,int maxY,int originX,int originY,int footprint,
    Grid grid,Random random,Blacklisted blacklisted,Validator valid) {
    AiBuildPlacementResult best;
    if(footprint<=0 || mapWidth<=0 || mapHeight<=0)return best;
    const int half=footprint/2;
    for(int x=minX;x<=maxX;++x)for(int y=minY;y<=maxY;++y) {
        const int left=x-half,top=y-half;
        const int right=x+half-(footprint%2==0),bottom=y+half-(footprint%2==0);
        if(left<0 || top<0 || right>=mapWidth || bottom>=mapHeight)continue;
        int score=0;bool blocked=false;
        for(int gx=left;gx<=right && !blocked;++gx)for(int gy=top;gy<=bottom;++gy) {
            const auto value=grid(gx,gy);
            if(value==255){blocked=true;break;}
            score+=value;
        }
        if(blocked)continue;
        bool replace=best.score<0 || score>best.score;
        if(score==best.score) {
            const auto distance=[&](int px,int py) {
                const int64_t dx=int64_t(originX)-px,dy=int64_t(originY)-py;
                return dx*dx+dy*dy;
            };
            const auto difference=distance(x,y)-distance(best.x,best.y);
            replace=difference>-2 && difference<2 ? random()%2!=0 : difference<0;
        }
        // The original consumes tie RNG before blacklist/placement rejection.
        if(replace && !blacklisted(x,y) && valid(float(x)+0.5f,float(y)+0.5f))
            best={x,y,score};
    }
    return best;
}
} // namespace swgb
