// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "../src/core/genie_explore_search.h"
#include <limits>
static void exploreSearchChecks() {
    swgb::GenieExploreSearch search;search.x=search.y=10;search.lineOfSight=2;
    const auto unseen=[](int x,int y){return (x-10)*(x-10)+(y-10)*(y-10)>4;};
    for(uint32_t seed:{2u,6u,1u}) {
        uint32_t rng=seed,expected=seed;const auto jitter=swgb::genieRandomDraw(expected)%3;
        int x=-1,y=-1,paths=0;
        check(search.choose(20,20,rng,nullptr,unseen,[&](int,int){++paths;return true;},x,y)&&
              x==(jitter?11:9)&&y==(jitter?12:8)&&paths==1&&rng==expected,
              "Original local exploration scan, jitter replacement and one RNG draw");
    }
    uint32_t rng=2;int x=-1,y=-1,paths=0;
    check(!search.choose(20,20,rng,nullptr,unseen,[&](int,int){++paths;return false;},x,y)&&
          x==9&&y==8&&paths==1,"Failed final path preserves chosen XY without retry");
    rng=2;const auto before=rng;x=y=-1;paths=0;
    check(!search.choose(20,20,rng,nullptr,[](int,int){return false;},[&](int,int){++paths;return true;},x,y)&&
          rng!=before&&x==-1&&y==-1&&paths==0,"No admitted candidate still consumes the search draw");
    search.command=705;search.destinationX=search.destinationY=15;rng=2;
    check(!search.choose(20,20,rng,nullptr,unseen,[](int,int){return true;},x,y)&&rng==2,
          "Known exploration command returns before consuming RNG");
    search.command=710;search.destinationX=search.destinationY=-1;
    swgb::GenieExploreHistory history;history.append(9,8);rng=2;x=y=-1;
    check(search.choose(20,20,rng,&history,unseen,[](int,int){return true;},x,y)&&!(x==9&&y==8),
          "Original history excludes a previously selected destination");
    check(history.contains(9,8)&&!history.contains(8,9),"History uses both float coordinates");
    history.points[0]={-0.f,0.f};check(history.contains(0,0),"History matches signed zero");
    history.points[0]={std::numeric_limits<float>::quiet_NaN(),8.f};
    check(history.contains(9,8)&&!history.contains(9,7),"Original unordered comparison still requires both coordinates");
    for(int i=1;i<8;++i)history.append(i,i);
    history.append(20,21);
    check(history.count==1&&history.contains(20,21)&&!history.contains(7,7),
          "Original history writer resets the active count after eight entries");
    struct Boundary {float x,y,los,gx,gy;int command,minX,maxX,minY,maxY,outX,outY;};
    const Boundary boundaries[]={
        {std::nextafter(1.f,0.f),10,2,-1,-1,710,0,5,5,15,0,10},
        {10.9f,8.2f,2.9f,-1,-1,710,5,15,3,13,10,8},
        {-.5f,2.9f,0,-1,-1,710,0,2,0,5,0,2},
        {.2f,.4f,1.9f,-1,-1,710,0,4,0,4,0,0},
        {19.9f,19.2f,1,-1,-1,710,15,19,15,19,19,19},
        {1,1,2,8.9f,7.9f,730,1,15,0,14,4,4},
        {18,18,2,1.9f,2.9f,730,0,8,0,9,8,9},
        {10,10,2,15,-1,710,5,15,5,15,12,5},
        {10,10,2,-1,15,710,5,15,5,15,10,10},
        {100,100,2,-1,-1,710,95,19,95,19,-1,-1},
        {10,10,-4,-1,-1,710,11,9,11,9,-1,-1}};
    for(const auto &c:boundaries) {
        search={c.x,c.y,c.los,c.gx,c.gy,c.command};rng=2;x=y=-1;int visited=0;
        const bool result=search.choose(20,20,rng,nullptr,[&](int cx,int cy){
            const int columns=c.maxX-c.minX+1;
            check(columns>0&&cx==c.minX+visited%columns&&cy==c.minY+visited/columns,
                  "Original fractional/clipped rectangle scans every candidate in Y-major order");
            ++visited;return true;
        },[](int,int){return true;},x,y);
        uint32_t expected=2;swgb::genieRandomDraw(expected);
        check(result==(c.outX>=0)&&x==c.outX&&y==c.outY&&rng==expected&&
              visited==std::max(0,c.maxX-c.minX+1)*std::max(0,c.maxY-c.minY+1),
              "Original clipped bounds, command730, one-coordinate bias and empty scan controls");
    }
    std::puts("PASS: original local exploration destination search, RNG, path rejection and history");
}
