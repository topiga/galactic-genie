// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "genie_random.h"
#include <array>
#include <cmath>
#include <cstdint>

namespace swgb {
// Original 586a70. Separate from actor-local 41c1d0: explored fog may qualify.
// Coordinates use the original map orientation. Grid/visibility/path lifecycles
// stay with the caller; quarter-record meanings are deliberately not invented.
struct GenieExploreSpiral {
    struct Quarter {int32_t first=0,second=0;};
    float x=0,y=0;
    bool class58=false;
    std::array<Quarter,16> quarters{}; // X-major, then Y; original stride12.

    template<class Draw,class Grid,class Visible,class Admit,class Zone,class Path,class Mark,class Exhausted>
    bool chooseWithDraw(int width,int height,Draw draw,Grid grid,Visible visible,
                        Admit admit,Zone sameZone,Path path,Mark mark,Exhausted exhausted,
                        int &outX,int &outY) const {
        if(width<=0 || height<=0 || width>32767 || height>32767 ||
           !std::isfinite(x) || !std::isfinite(y) || std::abs(x)>32767 || std::abs(y)>32767)return false;
        int direction=draw()%4,cx=int(x),cy=int(y);
        bool left=false,right=false,top=false,bottom=false;
        for(int length=1;!(left&&right&&top&&bottom);++length) {
            for(int leg=0;leg<2;++leg) {
                direction=(direction+1)%4;
                for(int step=0;step<length;++step) {
                    switch(direction){case 0:++cy;break;case 1:++cx;break;case 2:--cy;break;case 3:--cx;break;}
                    if(cx<0){left=true;continue;}
                    if(cx>=width){right=true;continue;}
                    if(cy<0){top=true;continue;}
                    if(cy>=height){bottom=true;continue;}
                    const auto value=grid(cx,cy); // Original reads before visibility.
                    if(visible(cx,cy) || (value!=255 && (class58 || value!=2)) ||
                       !admit(cx,cy) || !sameZone(cx,cy))continue;
                    if(path(cx,cy)) {
                        mark(cx,cy,0);outX=cx;outY=cy;return true;
                    }
                    mark(cx,cy,1);
                    int64_t best=-1;int qx=0,qy=0;
                    for(int i=0;i<16;++i) {
                        // Preserve original signed32 score comparison and wrapping.
                        const uint32_t raw=uint32_t(quarters[i].first)+uint32_t(draw()%100)+25u*uint32_t(quarters[i].second);
                        const int64_t score=raw<=0x7fffffffu?int64_t(raw):int64_t(raw)-0x100000000LL;
                        if(best==-1 || score<best){best=score;qx=width*(i/4)/4;qy=height*(i%4)/4;}
                    }
                    const int farX=qx+width/4-1,farY=qy+height/4-1;
                    const int corner=draw()%4;
                    outX=corner<2?qx:farX;outY=(corner==1 || corner==2)?farY:qy;
                    // Original returns this quarter corner without another path check.
                    return true;
                }
            }
        }
        exhausted();return false;
    }
    template<class Grid,class Visible,class Admit,class Zone,class Path,class Mark,class Exhausted>
    bool choose(int width,int height,uint32_t &randomState,Grid grid,Visible visible,
                Admit admit,Zone sameZone,Path path,Mark mark,Exhausted exhausted,int &outX,int &outY) const {
        return chooseWithDraw(width,height,[&](){return genieRandomDraw(randomState);},
                              grid,visible,admit,sameZone,path,mark,exhausted,outX,outY);
    }
};
} // namespace swgb
