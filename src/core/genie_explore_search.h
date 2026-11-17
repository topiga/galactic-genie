// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "genie_random.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

namespace swgb {
struct GenieExploreHistory {
    std::array<std::array<float,2>,8> points{};
    size_t count = 0;
    bool contains(int x,int y) const {
        for(size_t i=0;i<count;++i) {
            // Original x87 C3 test accepts unordered comparisons as well as equality.
            const auto equal=[](float a,float b){return !(a<b || a>b);};
            if(equal(points[i][0],float(x)) && equal(points[i][1],float(y))) return true;
        }
        return false;
    }
    void append(int x,int y) {
        if(count>=points.size())count=0;
        points[count++]={float(x),float(y)};
    }
};

// Original 41c1d0: coordinates and scan order are in the original map orientation.
struct GenieExploreSearch {
    float x=0,y=0,lineOfSight=0,destinationX=-1,destinationY=-1;
    int command=710;
    template<class Admit,class Path>
    bool choose(int width,int height,uint32_t &randomState,const GenieExploreHistory *history,
                Admit admit,Path path,int &outX,int &outY) const {
        if(command==705 && destinationX!=-1 && destinationY!=-1)return false;
        if(width<=0 || height<=0 || !std::isfinite(x) || !std::isfinite(y) ||
           !std::isfinite(lineOfSight) || !std::isfinite(destinationX) || !std::isfinite(destinationY) ||
           std::abs(x)>32767 || std::abs(y)>32767 || std::abs(lineOfSight)>32767 ||
           std::abs(destinationX)>32767 || std::abs(destinationY)>32767)return false;
        const int radius=int(lineOfSight)+3;
        // World61ee32 enables extended x87 precision during simulation callbacks.
        int minX=int(double(x)-radius),maxX=int(double(x)+radius);
        int minY=int(double(y)-radius),maxY=int(double(y)+radius);
        if(command==730) {
            minX=int(destinationX)-7;maxX=int(destinationX)+7;
            minY=int(destinationY)-7;maxY=int(destinationY)+7;
        }
        minX=std::max(0,minX);minY=std::max(0,minY);
        maxX=std::min(width-1,maxX);maxY=std::min(height-1,maxY);
        const int jitter=genieRandomDraw(randomState)%3;
        int64_t best=-1;
        const auto distance=[](int ax,int ay,int bx,int by) {
            const int64_t dx=int64_t(ax)-bx,dy=int64_t(ay)-by;return dx*dx+dy*dy;
        };
        for(int cy=minY;cy<=maxY;++cy)for(int cx=minX;cx<=maxX;++cx) {
            if(!admit(cx,cy) || (history && history->contains(cx,cy)))continue;
            int64_t score=distance(cx,cy,int(x),int(y));
            if(destinationX!=-1)score+=distance(cx,cy,int(destinationX),int(destinationY));
            if(best==-1 || score<best+jitter){best=score;outX=cx;outY=cy;}
        }
        // Failure leaves the selected coordinates written; there is no second-choice retry.
        return best!=-1 && path(outX,outY);
    }
};
} // namespace swgb
