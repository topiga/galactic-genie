// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "../src/core/genie_explore_spiral.h"
static void exploreSpiralChecks() {
    // Original-instruction evidence: exploration-spiral-controls-2026-11-17.json.
    const int destinations[4][2]={{13,12},{12,7},{7,8},{8,13}};
    for(int rotation=0;rotation<4;++rotation) {
        swgb::GenieExploreSpiral search;search.x=search.y=10;
        int x=-1,y=-1,draws=0,paths=0,marked=-1;
        const bool ok=search.chooseWithDraw(20,20,[&](){++draws;return rotation;},
            [](int,int){return 255;},[](int a,int b){return std::abs(a-10)<=2&&std::abs(b-10)<=2;},
            [](int,int){return true;},[](int,int){return true;},[&](int,int){++paths;return true;},
            [&](int,int,int value){marked=value;},[](){check(false,"Spiral should not exhaust");},x,y);
        check(ok&&x==destinations[rotation][0]&&y==destinations[rotation][1]&&draws==1&&paths==1&&marked==0,
              "Original spiral rotation, first candidate and reservation");
    }
    for(int fixture=0;fixture<7;++fixture) {
        swgb::GenieExploreSpiral search;search.x=fixture==6?10.9f:10;search.y=fixture==6?10.2f:10;search.class58=fixture==1;
        int x=-1,y=-1,draws=0,paths=0,marked=-1,exhausted=0;
        const bool ok=search.chooseWithDraw(20,20,[&](){++draws;return 0;},
            [&](int,int){return fixture<=1?2:fixture==2?0:255;},
            [&](int a,int b){return fixture==4 || (std::abs(a-10)<=2&&std::abs(b-10)<=2);},
            [](int,int){return true;},[&](int,int){return fixture!=3;},[&](int,int){++paths;return fixture!=5;},
            [&](int,int,int value){marked=value;},[&](){++exhausted;},x,y);
        const bool fails=fixture>=1&&fixture<=4;
        check(ok==!fails&&draws==(fixture==5?18:1)&&exhausted==int(fails),
              "Original grid class gate, zone/visibility exhaustion and fallback RNG count");
        check(fails?(x==-1&&y==-1&&paths==0&&marked==-1):
              (paths==1&&marked==(fixture==5?1:0)&&x==(fixture==5?0:13)&&y==(fixture==5?0:12)),
              "Original spiral failure preserves output; failed path returns unchecked quarter corner");
    }
    for(int corner=0;corner<4;++corner) {
        swgb::GenieExploreSpiral search;search.x=search.y=10;
        for(auto &q:search.quarters)q={50,2};search.quarters[9]={0,0};
        int x=-1,y=-1,draws=0,paths=0;
        check(search.chooseWithDraw(20,20,[&](){++draws;return corner;},
            [](int,int){return 255;},[](int a,int b){return std::abs(a-10)<=2&&std::abs(b-10)<=2;},
            [](int,int){return true;},[](int,int){return true;},[&](int,int){++paths;return false;},
            [](int,int,int value){check(value==1,"Failed spiral path marks information grid1");},[](){},x,y)&&
            x==(corner<2?10:14)&&y==((corner==1||corner==2)?9:5)&&draws==18&&paths==1,
            "Original weighted quarter records, corner order and no fallback path retry");
    }
    swgb::GenieExploreSpiral search;search.x=search.y=10;
    uint32_t rng=1;int x=-1,y=-1;
    check(search.choose(20,20,rng,[](int,int){return 255;},
          [](int a,int b){return std::abs(a-10)<=2&&std::abs(b-10)<=2;},
          [](int,int){return true;},[](int,int){return true;},[](int,int){return true;},
          [](int,int,int){},[](){},x,y)&&x==12&&y==7&&rng==2745024u,
          "Spiral production wrapper advances the shared original CRT stream once on success");
    std::puts("PASS: original spiral rotations, candidate gates, information marks and random fallback");

}
