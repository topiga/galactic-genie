// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "ai_power_field.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>
namespace swgb {
// Original58ece0, for existing building masters (UT80/48bce0).
inline int originalBuildingPlacementCategory(int id) {
    switch(id) {
    case 12:return 15;case 45:return 7;case 50:return 8;
    case 68:case 319:return 2;case 70:return 10;
    case 72:case 117:case 155:case 195:return 4;
    case 79:case 196:case 234:case 235:case 236:return 3;
    case 109:return 9;case 276:return 6;case 323:return 1;
    case 335:return 14;case 562:return 11;case 584:return 13;case 598:return 12;
    default:return originalBuildingRequiresPower(id)?16:5;
    }
}
struct AiPlacementRect {int left,top,right,bottom;};
// Categories8(farm)/10(prefab)/11(carbon)/13(nova) placement stages. All coordinates and
// radii must be in ORIGINAL axes. This is a scoring grid, not pathability.
// Caller supplies persistent resource records in original order; current terrain
// and occupancy mask; and registered owned buildings. Hostile/remembered
// buildings, recent attack rectangles, planned wall segments and master
// placement validation remain separate explicit providers, never silently
// inferred from an unexplored native object list.
class AiBuildPlacementGrid {
public:
    explicit AiBuildPlacementGrid(int size,int requestedCategory=10):size_(std::max(0,size)),
        requestedCategory_(requestedCategory),cells_(size_t(size_)*size_,60) {}
    int size() const{return size_;}
    int at(int x,int y) const{return cells_[size_t(y)*size_+x];}
    const std::vector<uint8_t> &cells() const{return cells_;}
    AiPlacementRect clamp(AiPlacementRect r) const {
        if(size_==0)return {0,0,-1,-1};
        const auto c=[&](int n){return std::clamp(n,0,size_-1);};
        return {c(r.left),c(r.top),c(r.right),c(r.bottom)};
    }
    void forbid(AiPlacementRect r) {rect(r,[&](uint8_t &v){v=255;});}
    void add(AiPlacementRect r,int amount) {
        rect(r,[&](uint8_t &v){if(v<255)v=uint8_t(std::min(254,int(v)+(amount&255)));});
    }
    void subtract(AiPlacementRect r,int amount) {
        rect(r,[&](uint8_t &v){if(v<255)v=uint8_t(std::max(0,int(v)-(amount&255)));});
    }
    //591af0 uses the first collision radius on both original axes.
    static AiPlacementRect buildingBounds(float x,float y,float radius) {
        const int half=int(radius),ix=int(x),iy=int(y);
        const int even=double(radius)==double(half)?1:0;
        return {ix-half,iy-half,ix+half-even,iy+half-even};
    }
    // Category10 skips resource bucket1 entirely, and disabled record+d==1.
    // Supply only resolved persistent records; other invalidation is upstream.
    void resource(int bucket,int unitClass,float x,float y,float radiusX,float radiusY,bool disabled=false) {
        const bool resourceDepot=requestedCategory_==11 || requestedCategory_==13;
        if(size_==0 || (!resourceDepot && bucket==1) || disabled)return;
        auto r=clamp({int(double(x)-radiusX-.5),int(double(y)-radiusY-.5),
                      int(double(x)+radiusX+.5),int(double(y)+radiusY+.5)});
        forbid({int(x),int(y),int(x),int(y)});
        if(resourceDepot) {
            if(unitClass==27)subtract(expand(r,5.),128);
            else if((requestedCategory_==11 && (unitClass==29 || unitClass==30)) ||
                    (requestedCategory_==13 && (unitClass==29 || unitClass==31)))
                subtract(expand(r,requestedCategory_==11?3.:1.),128);
            else if((requestedCategory_==11 && unitClass==31) ||
                    (requestedCategory_==13 && unitClass==30)) {
                //58b100 expands the previous clamped rectangle at each step.
                r=expand(r,1.);add(r,requestedCategory_==11?12:16);
                r=expand(r,3.);add(r,requestedCategory_==11?4:8);
                r=expand(r,5.);add(r,requestedCategory_==11?1:4);
            }
            return;
        }
        if(unitClass!=31)r=expand(r,2.);
        subtract(r,5);
    }
    void terrain(int x,int y,int terrainId,uint32_t occupancy) {
        if(size_==0)return;
        if(occupancy&0x4003)forbid({x,y,x,y});
        if(terrainId==1 || terrainId==22 || terrainId==23) {
            forbid({x,y,x,y});subtract({x-3,y-3,x+3,y+3},10);
        }
    }
    void ownedBuilding(int prototype,float x,float y,float collisionRadius,float lineOfSight,
                       float resource18,float resource24) {
        if(size_==0)return;
        const int category=originalBuildingPlacementCategory(prototype);
        auto bounds=buildingBounds(x,y,collisionRadius);
        if(requestedCategory_==8) {
            //58e3cd bypasses the CC access halo for farms;58d470 adds three
            // overlapping preference rectangles around CCs, food depots, farms.
            bounds=clamp(bounds);forbid(bounds);
            const int weight=category==9?240:category==2?120:category==8?60:0;
            if(weight) {
                const double sight=lineOfSight<6.f?lineOfSight:6.f;
                add(expand(bounds,sight),weight);
                add(expand(bounds,sight*.5),weight);
                // Preserve original float32 third before the wider product:
                // LOS6 expands14 leftward to11, not12, after truncation.
                add(expand(bounds,sight*double(0.3333333432674408f)),weight);
            }
            return;
        }
        if(category==9)bounds={bounds.left-3,bounds.top-3,bounds.right+3,bounds.bottom+3};
        bounds=clamp(bounds);forbid(bounds);
        if(requestedCategory_==11 || requestedCategory_==13) {
            //58e4d6: resource depots discourage nearby owned construction.
            double extent=lineOfSight<6.f?lineOfSight:6.f;
            if(category==9)extent*=.5;
            else if(category==1 || category==2 || category==11 || category==13)extent=5.;
            subtract(expand(bounds,extent),10);
            return;
        }
        const float powerExpansion=float(double(resource18)-2.);
        const float cappedSight=lineOfSight<6.f?lineOfSight:6.f;
        if(category==10) {add(expand(bounds,powerExpansion),60);return;}
        if(category==14) {
            //58d225 deliberately falls through the prefab bonus at58d22a.
            add(expand(bounds,float(double(resource24)-2.)),10);
            add(expand(bounds,powerExpansion),60);return;
        }
        if(category==15) {subtract(expand(bounds,powerExpansion),120);return;}
        if(category==16) {subtract(expand(bounds,powerExpansion),10);return;}
        const double extent=category==9?double(float(cappedSight*.25f))*2.:double(cappedSight)*2.;
        subtract(expand(bounds,extent),60);
    }
    //58e770 normal/forward known-building stage. Caller supplies only resolved
    // actor state<=2, classes6..10/18; alliance is the original player+24 result.
    void knownBuilding(int prototype,float x,float y,float radius,bool allied,
                       float range,float lineOfSight,bool forward=false) {
        if(size_==0)return;
        auto r=buildingBounds(x,y,radius);
        const int category=originalBuildingPlacementCategory(prototype);
        if(forward && !allied) {
            const int extent=int(lineOfSight)+2;
            r=clamp({r.left-extent,r.top-extent,r.right+extent,r.bottom+extent});
        } else if(!forward && !allied && (category==3 || category==9 || prototype==82)) {
            r={int(double(r.left)-range+.5),int(double(r.top)-range+.5),
               int(double(r.right)+range+.5),int(double(r.bottom)+range+.5)};
        }
        //487390 rejects the ENTIRE un-clamped rectangle if any edge lies outside.
        // The forward branch clamps first; the normal range branch does not.
        if(r.left<0 || r.top<0 || r.right<0 || r.bottom<0 ||
           r.left>=size_ || r.top>=size_ || r.right>=size_ || r.bottom>=size_)return;
        forbid(r);
    }
private:
    AiPlacementRect expand(AiPlacementRect r,double amount) const {
        return clamp({int(double(r.left)-amount),int(double(r.top)-amount),
                      int(double(r.right)+amount),int(double(r.bottom)+amount)});
    }
    template<class F> void rect(AiPlacementRect r,F operation) {
        if(size_==0)return;
        r=clamp(r);
        for(int y=r.top;y<=r.bottom;++y)for(int x=r.left;x<=r.right;++x)
            operation(cells_[size_t(y)*size_+x]);
    }
    int size_;
    int requestedCategory_;
    std::vector<uint8_t> cells_;
};
} // namespace swgb
