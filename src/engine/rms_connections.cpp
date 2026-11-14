// SPDX-License-Identifier: GPL-3.0-or-later
#include "rms_connections.h"
#include "../core/genie_random.h"
#include <algorithm>
#include <cmath>
#include <queue>

namespace swgb {
bool connectRmsLandPair(uint32_t width,uint32_t height,
    std::vector<uint8_t> &terrain,RmsConnectionPoint start,RmsConnectionPoint end,
    const RmsConnectionSettings &settings,uint32_t &randomState,std::string *err) {
    if(err)err->clear();
    auto fail=[&](const char *why){if(err)*err=why;return false;};
    if(!width||!height||width>1024||height>1024||terrain.size()!=size_t(width)*height)
        return fail("RMS connection: invalid dimensions");
    auto inside=[&](int x,int y){return x>=0&&y>=0&&x<int(width)&&y<int(height);};
    if(!inside(start.x,start.y)||!inside(end.x,end.y))return fail("RMS connection: point outside map");
    for(auto t:terrain)if(t>=settings.size())return fail("RMS connection: terrain outside original99-slot table");
    for(const auto &s:settings)
        if(s.width<0||s.width>1024||s.variance<0||s.variance>s.width||
           s.replacement < -1||s.replacement>=99||!std::isfinite(s.cost)||s.cost<0||s.cost>10000)
            return fail("RMS connection: unsupported terrain settings");
    auto index=[&](int x,int y){return size_t(y)*width+x;};
    if(settings[terrain[index(end.x,end.y)]].cost<=0)return true;
    if(start.x==end.x&&start.y==end.y)return true;
    // Original queue inserts equal priorities before older entries. Nodes are
    // marked on discovery; this search never relaxes an already visited node.
    struct Node { float total,cost; uint32_t order; int x,y; };
    struct Later {
        bool operator()(const Node &a,const Node &b)const {
            return a.total!=b.total ? a.total>b.total : a.order<b.order;
        }
    };
    std::priority_queue<Node,std::vector<Node>,Later> queue;
    std::vector<uint8_t> directions(terrain.size());
    directions[index(start.x,start.y)]=3;
    directions[index(end.x,end.y)]=2;
    uint32_t order=0;
    int x=start.x,y=start.y;
    float cost=0;
    bool found=false;
    while(!found) {
        const int dx=end.x-x,dy=end.y-y;
        const float distance=static_cast<float>(std::sqrt(double(dx*dx+dy*dy)));
        bool south=false,north=false,west=false,east=false;
        auto visit=[&](int nx,int ny,uint8_t direction,bool corner,double heuristic,bool &passable) {
            passable=false;
            if(!inside(nx,ny))return;
            auto &mark=directions[index(nx,ny)];
            // The original accepts the goal before testing diagonal corner clearance.
            if(mark==2){mark=direction;found=true;return;}
            if(mark){passable=mark!=1;return;}
            const float step=settings[terrain[index(nx,ny)]].cost;
            if(step<=0){mark=1;return;}
            if(!corner)return;
            const float nextCost=static_cast<float>(double(cost)+double(step)*(direction>=8?double(1.36f):1.0));
            mark=direction;passable=true;
            queue.push({static_cast<float>(double(nextCost)+heuristic),nextCost,++order,nx,ny});
        };
        bool unused=false;
        visit(x,y+1,7,true,double(distance)+(end.y>y?-1:1),south);if(found)break;
        visit(x,y-1,6,true,double(distance)+(end.y<y?-1:1),north);if(found)break;
        const float left=static_cast<float>(double(distance)+(end.x<x?-1:1));
        visit(x-1,y,4,true,left,west);if(found)break;
        visit(x-1,y-1,8,north&&west,double(left)+(end.y<y?-double(.41f):double(.41f)),unused);if(found)break;
        visit(x-1,y+1,9,south&&west,double(left)+(end.y>y?-double(.41f):double(.41f)),unused);if(found)break;
        const float right=static_cast<float>(double(distance)+(end.x>x?-1:1));
        visit(x+1,y,5,true,right,east);if(found)break;
        visit(x+1,y-1,10,north&&east,double(right)+(end.y<y?-double(.41f):double(.41f)),unused);if(found)break;
        visit(x+1,y+1,11,south&&east,double(right)+(end.y>y?-double(.41f):double(.41f)),unused);if(found)break;
        if(queue.empty())return true;
        auto next=queue.top();queue.pop();x=next.x;y=next.y;cost=next.cost;
    }
    // Backtrack before painting: include the start, exclude the goal. Each
    // rectangle consumes four RNG draws, including zero-variance widths.
    std::vector<uint8_t> painted(terrain.size());
    uint32_t rng=randomState;
    x=end.x;y=end.y;
    constexpr int backX[]={1,-1,0,0,1,1,-1,-1};
    constexpr int backY[]={0,0,1,-1,1,-1,1,-1};
    for(size_t visits=0;visits<=terrain.size();++visits) {
        const size_t previous=index(x,y);
        const uint8_t direction=directions[previous];
        if(direction<4||direction>11)break;
        x+=backX[direction-4];y+=backY[direction-4];
        if(!inside(x,y))break;
        const auto &s=settings[terrain[index(x,y)]];
        auto extent=[&](){return int((uint64_t(genieRandomDraw(rng))*uint32_t(s.variance*2))/32767)+s.width-s.variance;};
        const int left=std::max(0,x-extent()),top=std::max(0,y-extent());
        const int right=std::min(int(width)-1,x+extent()),bottom=std::min(int(height)-1,y+extent());
        for(int py=top;py<=bottom;++py)for(int px=left;px<=right;++px)painted[index(px,py)]=1;
        directions[previous]=255;
        if(visits==terrain.size())return fail("RMS connection: predecessor traversal limit");
    }
    for(size_t i=0;i<terrain.size();++i)
        if(painted[i]&&settings[terrain[i]].replacement>=0)terrain[i]=uint8_t(settings[terrain[i]].replacement);
    randomState=rng;
    return true;
}
} // namespace swgb
