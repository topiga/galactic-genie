// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cmath>
#include <cstdint>
namespace swgb {
struct AiBuildPlacementResult {
    int x=-1, y=-1, score=-1;
};
// Original588b30 exhaustive scans for farm8, prefab10 and resource depots1/2/11/13.
// The caller must
// supply its prepared original scoring grid and master placement validator.
// This does not substitute a native walkability grid, choose a builder, derive
// search bounds or resume a timed scan. Other placement categories are rejected.
// Validator receives half-tile coordinates even for an even-sized footprint;
// final result allocation converts even-sized footprints to integer positions.
template<class Grid, class Random, class Blacklisted, class Validator, class PlannedNearby>
AiBuildPlacementResult aiScanBuildingPlacement(int category,int mapWidth,int mapHeight,
    int minX,int minY,int maxX,int maxY,int originX,int originY,int footprint,
    Grid grid,Random random,Blacklisted blacklisted,Validator valid,PlannedNearby plannedNearby) {
    AiBuildPlacementResult best;
    if(footprint<=0 || mapWidth<=0 || mapHeight<=0 ||
       (category!=1 && category!=2 && category!=8 && category!=10 && category!=11 && category!=13))return best;
    const bool resourceDepot=category==1 || category==2 || category==11 || category==13;
    const int margin=resourceDepot?1:0;
    const int half=footprint/2;
    for(int x=minX;x<=maxX;++x)for(int y=minY;y<=maxY;++y) {
        const int left=x-half-margin,top=y-half-margin;
        const int right=x+half-(footprint%2==0)+margin,bottom=y+half-(footprint%2==0)+margin;
        if(left<0 || top<0 || right>=mapWidth || bottom>=mapHeight)continue;
        int score=0;bool blocked=false;
        for(int gx=left;gx<=right && !blocked;++gx)for(int gy=top;gy<=bottom;++gy) {
            const auto value=grid(gx,gy);
            if(value==255){blocked=true;break;}
            score+=value;
        }
        if(blocked)continue;
        if(resourceDepot) {
            //589b5c/58f190: same-prototype status1 planned-position records
            // within an inclusive10-tile square reduce preference. Registration
            // also creates these records; they are distinct from failed/status2
            // positions and must not be inferred only from live buildings.
            if(plannedNearby(x,y,10))score/=10;
            const int64_t dx=int64_t(originX)-x,dy=int64_t(originY)-y;
            const int distance=int(std::sqrt(double(dx*dx+dy*dy)));
            if(distance>10) {
                const int reduced=score-(distance-10)/5;
                //588ceb computes this from the actual, unexpanded footprint.
                if(reduced>int64_t(footprint)*footprint*60)score=reduced;
            }
        }
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
template<class Grid, class Random, class Blacklisted, class Validator>
AiBuildPlacementResult aiScanPrefabPlacement(int mapWidth,int mapHeight,
    int minX,int minY,int maxX,int maxY,int originX,int originY,int footprint,
    Grid grid,Random random,Blacklisted blacklisted,Validator valid) {
    return aiScanBuildingPlacement(10,mapWidth,mapHeight,minX,minY,maxX,maxY,
        originX,originY,footprint,grid,random,blacklisted,valid,
        [](int,int,int){return false;});
}
} // namespace swgb
