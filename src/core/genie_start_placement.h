// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cmath>

namespace swgb {
struct GenieStartFootprint {
    int minX=0,minY=0,maxX=0,maxY=0; // Inclusive terrain tiles.
};

// 48e180: master6c/70 when clearance is requested, otherwise3c/40.
// Callers supply the actual DAT extents, without the UI's minimum0.5 clamp.
inline bool genieStartFootprint(float x,float y,float halfX,float halfY,
                                int width,int height,GenieStartFootprint &out) {
    if (!std::isfinite(x) || !std::isfinite(y) ||
        !std::isfinite(halfX) || !std::isfinite(halfY) ||
        halfX<0 || halfY<0 || width<=0 || height<=0) return false;
    const float minX=float(double(x)-halfX),minY=float(double(y)-halfY);
    // Original x87 adds/subtracts before storing these bounds as float.
    const float maxX=float(double(x)+halfX-double(0.001f));
    const float maxY=float(double(y)+halfY-double(0.001f));
    if (minX<0 || minY<0 || double(maxX)>=width || double(maxY)>=height)
        return false;
    out={int(minX),int(minY),int(maxX),int(maxY)};
    return true;
}

// 48e697 checks mode 3 against the original left column and last row.
template<class Elevation>
bool genieHillMode3Allows(const GenieStartFootprint &area,Elevation height) {
    const int reference=height(area.minX,area.maxY);
    for(int y=area.minY;y<area.maxY;++y)
        if(std::abs(int(height(area.minX,y))-reference)>1)return false;
    for(int x=area.minX+1;x<=area.maxX;++x)
        if(std::abs(int(height(x,area.maxY))-reference)>1)return false;
    return true;
}

inline bool genieStartTerrainPassable(float multiplier) {
    // 48e603: not the movement rule (>0).
    return multiplier>0.05f;
}

inline bool genieStartRectanglesOverlap(float x,float y,float halfX,float halfY,
                                        float otherX,float otherY,
                                        float otherHalfX,float otherHalfY) {
    // 5c9631/5c7ec8: dy is stored to float; dx remains on the x87 stack.
    const double dx=std::abs(double(otherX)-x);
    const float dy=std::abs(float(double(otherY)-y));
    return dx<double(halfX)+otherHalfX &&
           double(dy)<double(halfY)+otherHalfY;
}
} // namespace swgb
