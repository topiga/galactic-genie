// SPDX-License-Identifier: GPL-3.0-or-later
#include "engine/ai_group_search.h"
#include <cstdlib>
#include <iostream>
using namespace swgb;
static void check(bool value,const char *message) {
    if(!value){std::cerr<<"FAIL: "<<message<<'\n';std::exit(1);}
}
int main() {
    // tactical-no-target-idle-controls-2026-11-18.json: actual5ae7f0, controller boundaries modeled.
    for(int state=0;state<16;++state)
        check(aiGroupSearchIdle(state,-1,-1,false,false)==
              (state==0 || state==1 || state==2 || state==3 || state==13 || state==14),
              "Original idle action-state admission");
    check(!aiGroupSearchIdle(2,600,-1,false,true) && !aiGroupSearchIdle(2,610,-1,false,true),
          "Attack and movement tasks stay busy even when action state is complete");
    check(aiGroupSearchIdle(2,615,-1,false,false) && !aiGroupSearchIdle(2,615,77,false,false) &&
          !aiGroupSearchIdle(3,615,-1,false,false),"Task615 idle exception requires state2 and no target");
    check(!aiGroupSearchIdle(2,-1,-1,true,false) && aiGroupSearchIdle(2,-1,-1,true,true),
          "Unfinished original movement group vetoes idle actor");

    AiGroupSearchState search;uint32_t flags=40;search.lastBusyTime=20;
    check(refreshAiGroupSearch(search,flags,{{{2,4,6},true,true},{{4,8,10},true,true},{{99,99,99},false,false}},100),
          "Eligible member centroid refresh supported");
    check(search.centroid==std::array<float,3>{3,6,8} && flags==41 && search.lastBusyTime==20,
          "Idle refresh retains last-busy stamp and excludes invalid members");
    check(refreshAiGroupSearch(search,flags,{{{2,4,6},true,false}},200) && flags==40 &&
          search.lastBusyTime==200,"Any busy valid member stamps original group timer");
    check(!refreshAiGroupSearch(search,flags,{},300),"Empty group is explicitly unsupported");

    // Original5a3c40 initializer controls, target lookup exhausted.
    int phase=2;bool initialized=false;int dispatches=0;
    auto dispatch=[&]{check(phase==18 && !initialized && search.started==5200,
                          "Dispatch observes original initialization ordering");++dispatches;};
    check(beginAiGroupSearch(search,flags,phase,initialized,5199,dispatch)==AiGroupSearchStart::Waiting &&
          phase==1 && !initialized && !dispatches,"Initial exact4999 elapsed waits without dispatch");
    check(beginAiGroupSearch(search,flags,phase,initialized,5200,dispatch)==AiGroupSearchStart::Started &&
          initialized && dispatches==1 && search.origin==search.centroid,
          "Initial exact5000 elapsed copies centroid and starts search");
    search.lastBusyTime=6000;phase=2;initialized=false;
    check(beginAiGroupSearch(search,flags,phase,initialized,5999,dispatch)==AiGroupSearchStart::Waiting,
          "Initial signed-negative elapsed clamps to zero");
    flags&=~32u;search.origin={10,20,30};search.centroid={40,50,60};search.lastBusyTime=0;
    check(beginAiGroupSearch(search,flags,phase,initialized,10000,[&]{++dispatches;})==AiGroupSearchStart::Started &&
          search.origin==std::array<float,3>{10,20,30},"Cleared flag5 retains previous search origin");

    // Six original59b160 controls; RNG boundary supplied, clamp/truncation actual.
    struct PointCase {std::array<float,3> origin;uint32_t rx,ry;int width,height,x,y;};
    const PointCase points[]={{{50.9f,3,60.9f},0,0,100,100,30,40},
        {{50.9f,3,60.9f},39,39,100,100,69,79},{{50.9f,3,60.9f},80,121,100,100,30,41},
        {{1,0,2},0,0,100,100,0,0},{{90,0,60},39,39,96,64,95,63},{{-.9f,0,-.9f},21,21,100,100,1,1}};
    for(const auto &c:points) {
        search.origin=c.origin;int draws=0;
        const auto point=aiGroupSearchPoint(search,c.width,c.height,[&]{return draws++?c.ry:c.rx;});
        check(point && *point==std::array<int32_t,2>{c.x,c.y} && draws==2,
              "Original search point uses two ordered draws, truncation and rectangular clamp");
    }
    // Eight original5a1d70 controls: unsigned elapsed, strict thresholds, Stop before flag clear.
    struct TimeoutCase {uint32_t flags,now,stamp;bool release;};
    const TimeoutCase times[]={{41,6000,1000,false},{41,6001,1000,true},{40,6001,1000,false},
        {40,26000,1000,false},{40,26001,1000,true},{41,4900,0xffffff00u,true},
        {40,999,1000,true},{255,6001,1000,true}};
    for(const auto &c:times) {
        search.started=c.stamp;flags=c.flags;phase=18;int stopped=0;
        const bool released=advanceAiGroupSearch(search,flags,phase,c.now,[&]{
            check(phase==1 && flags==c.flags,"Original Stop precedes clearing search-origin flag");++stopped;});
        check(released==c.release && stopped==int(c.release) && phase==(c.release?1:18) &&
              flags==(c.release?c.flags&~32u:c.flags),"Original state18 timeout golden");
    }
    search={};search.centroid={12,3,34};flags=32u|8u;phase=2;initialized=false;
    beginAiGroupSearch(search,flags,phase,initialized,5000,[]{});
    check(search.origin==std::array<float,3>{12,3,34},"Fresh constructor32 plus attack-now8 captures origin");
    advanceAiGroupSearch(search,flags,phase,30001,[]{});
    search.centroid={50,6,70};
    beginAiGroupSearch(search,flags,phase,initialized,31000,[]{});
    check(search.origin==std::array<float,3>{12,3,34},"Second timeout cycle retains original wander origin");
    advanceAiGroupSearch(search,flags,phase,56001,[]{});
    flags|=32u; // Original successful target selection rearms centroid capture.
    beginAiGroupSearch(search,flags,phase,initialized,57000,[]{});
    check(search.origin==std::array<float,3>{50,6,70},"Successful acquisition rearms next origin capture");
    std::cout<<"Original group search helpers passed\n";
}
