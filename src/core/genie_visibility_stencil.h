// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <algorithm>
#include <array>
#include <vector>

namespace swgb {
// Original4912f0: ordered perimeter samples, not every cell in the footprint.
inline std::vector<std::array<int,2>> genieVisibilityStencil(
    int cx,int cy,int left,int top,int right,int bottom,int width,int height) {
    if(left>right)std::swap(left,right);
    if(top>bottom)std::swap(top,bottom);
    if(left>=width || top>=height || right<0 || bottom<0 || (left==right && top==bottom))return {};
    const int nx=right-left+1,ny=bottom-top+1;
    const int qx=nx<=2?nx:(nx+1)/2,qy=ny<=2?ny:(ny+1)/2;
    std::vector<std::array<int,2>> offsets;
    const auto add=[&](int x,int y) {
        const std::array<int,2> offset={x-cx,y-cy};
        if((offset[0] || offset[1]) && std::find(offsets.begin(),offsets.end(),offset)==offsets.end())
            offsets.push_back(offset);
    };
    add(left,top);add(right,top);add(left,bottom);add(right,bottom);
    for(int i=1;i<qx-1;++i) {
        add(left+2*i,top);
        if(qy>1)add(right-2*i,bottom);
    }
    for(int i=1;i<qy-1;++i) {
        add(left,top+2*i);
        if(qx>1)add(right,bottom-2*i);
    }
    return offsets;
}
}
