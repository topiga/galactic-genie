// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "sight_stencils.h"
#include <algorithm>
#include <cmath>

namespace swgb {
// World61f09a..61f18b. A source must be alive/completed (original state2).
// Core12 can use its container's center; contained class61 droids do not emit.
inline int originalPowerRadius(int id,int unitClass,bool contained,int state,
                               float coreRadius,float droidRadius) {
    if(state!=2 || (id!=12 && (unitClass!=61 || contained)))return 0;
    const float value=unitClass==61?droidRadius:coreRadius;
    if(!std::isfinite(value) || value<1)return 0;
    return value>=20?20:int(value);
}

//61f18b uses the same integer-center stencil table as sight family0.
// Coordinates and callbacks are ORIGINAL map tiles. Never add footprint radii.
template<class Visit>
void forEachOriginalPowerTile(int width,int height,float x,float y,int radius,Visit visit) {
    if(width<=0 || height<=0 || radius<1 || radius>20 ||
       !std::isfinite(x) || !std::isfinite(y) || x<0 || y<0 || x>=width || y>=height)return;
    const int cx=int(x),cy=int(y);
    for(int dy=-radius;dy<=radius;++dy) {
        const int row=cy+dy;
        if(row<0 || row>=height)continue;
        const int half=kSightHalfWidths[0][radius][dy+radius];
        for(int col=std::max(0,cx-half);col<=std::min(width-1,cx+half);++col)visit(col,row);
    }
}

//48bce0 building master virtual0.
inline bool originalBuildingRequiresPower(int id) {
    switch(id) {
    case 45:case 49:case 68:case 84:case 87:case 101:case 103:case 104:
    case 209:case 317:case 319:case 323:case 335:case 530:case 562:case 584:return true;
    default:return false;
    }
}
//585a75..8b adds four informationAI exclusions to the master gate.
inline bool originalUnpoweredBuildingCandidate(int id) {
    switch(id) {
    case 45:case 49:case 84:case 87:case 101:case 103:case 104:
    case 209:case 317:case 319:case 335:case 530:return true;
    default:return false;
    }
}
//54bda0 bypasses the power bit for these prototypes/classes.
inline bool originalPowerExempt(int id,int unitClass) {
    return id==109 || id==82 || id==50 || unitClass==34 || unitClass==36 ||
           unitClass==10 || unitClass==9;
}
} // namespace swgb
