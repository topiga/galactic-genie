// SPDX-License-Identifier: GPL-3.0-or-later
#include "rms_land_placement.h"
#include "../core/genie_random.h"
#include <algorithm>
#include <cstdlib>
namespace swgb {
bool resolveRmsLandPositions(uint32_t width,uint32_t height,
    std::vector<RmsLandPlacement> &lands,uint32_t &randomState,
    std::string *err,size_t maxAttempts) {
    if(err)err->clear();
    auto fail=[&](const char *why){if(err)*err=why;return false;};
    if(!width||!height||width>1024||height>1024||lands.size()>64)
        return fail("RMS land placement: invalid dimensions or land count");
    for(const auto &land:lands) {
        const int right=land.right<0?int(width):land.right;
        const int bottom=land.bottom<0?int(height):land.bottom;
        if(land.x < -1||land.y < -1||land.x>=int(width)||land.y>=int(height)||
           land.radius<0||land.radius>1024||land.separation<0||land.separation>1024||
           land.left<0||land.top<0||land.right < -1||land.bottom < -1||
           right>int(width)||bottom>int(height)||
           right-land.left<2*land.radius||bottom-land.top<2*land.radius||
           land.minPlacementDistance < -1||land.minPlacementDistance>4096)
            return fail("RMS land placement: unsupported bounds or metadata");
    }
    auto result=lands;uint32_t rng=randomState;
    for(size_t i=0;i<result.size();++i) {
        auto &land=result[i];
        if(land.x>=0&&land.y>=0)continue;
        const int spanX=(land.right<0?int(width):land.right)-land.left-2*land.radius;
        const int spanY=(land.bottom<0?int(height):land.bottom)-land.top-2*land.radius;
        int collisions=990;
        for(;;) {
            if(!maxAttempts)return fail("RMS land placement: attempt budget exceeded");
            --maxAttempts;
            land.x=int(genieRandomDraw(rng)*uint32_t(spanX)/32767u);
            land.y=int(genieRandomDraw(rng)*uint32_t(spanY)/32767u);
            if((land.x<spanX/3||land.x>spanX-spanX/3)&&
               (land.y<spanY/3||land.y>spanY-spanY/3))continue;
            land.x+=land.left+land.radius;land.y+=land.top+land.radius;
            bool collision=false;
            for(size_t j=0;j<result.size();++j) {
                // Original tests the OTHER x but CURRENT y for resolution.
                if(j==i||result[j].x<0||land.y<0)continue;
                const auto &other=result[j];
                const int distance=land.minPlacementDistance>=0?land.minPlacementDistance:
                    land.radius+other.radius+std::max(land.separation,other.separation);
                if(std::abs(land.x-other.x)<distance&&std::abs(land.y-other.y)<distance) {
                    collision=true;break;
                }
            }
            if(!collision)break;
            if(!--collisions){land.x=int(width)/2;land.y=int(height)/2;break;}
        }
    }
    lands=std::move(result);randomState=rng;return true;
}
}
