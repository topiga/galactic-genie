// SPDX-License-Identifier: GPL-3.0-or-later
#include "rms_slopes.h"
#include <algorithm>
#include <utility>

namespace swgb {
bool smoothRmsHeights(uint32_t width, uint32_t height,
    std::vector<uint8_t> &heights, std::string *err) {
    if (err) err->clear();
    auto fail=[&](const char *message) { if(err) *err=message;return false; };
    if (!width || !height || width>1024 || height>1024 || heights.size()!=size_t(width)*height ||
        std::any_of(heights.begin(),heights.end(),[](uint8_t h) { return h>8; }))
        return fail("RMS height smoothing: invalid dimensions or heights");
    const int w=static_cast<int>(width), h=static_cast<int>(height);
    auto result=heights;
    constexpr int dx[]={-1,0,1,1,1,0,-1,-1}, dy[]={-1,-1,-1,0,1,1,1,0};
    uint64_t work=0;
    auto pass=[&](int kind,bool &changed) {
        changed=false;
        for(int y=0;y<h;++y) for(int x=0;x<w;++x) {
            if(++work>64000000) return false;
            const int center=result[y*w+x];
            bool high[8]={};
            for(int k=0;k<8;++k) {
                const int nx=x+dx[k],ny=y+dy[k];
                high[k]=nx>=0 && nx<w && ny>=0 && ny<h && result[ny*w+nx]>center+(kind==2?1:0);
            }
            bool raise=false;
            if(kind==0) raise=(high[1] && high[5]) || (high[3] && high[7]);
            else if(kind==1) {
                for(int c=0;c<8;c+=2) if(high[c]) {
                    auto at=[&](int offset) { return high[(c+offset)%8]; };
                    raise=raise || (at(2) && !at(1)) || (at(3) && !at(2)) ||
                          (at(6) && !at(7)) || (at(5) && !at(6)) ||
                          (at(4) && !at(5) && !at(3));
                }
            } else raise=std::any_of(high,high+8,[](bool value) { return value; });
            if(raise) { ++result[y*w+x];changed=true; }
        }
        return true;
    };
    bool corners=false,gradient=false,gaps=false;
    do {
        do { if(!pass(0,gaps)) return fail("RMS height smoothing: work limit exceeded"); } while(gaps);
        if(!pass(1,corners) || !pass(2,gradient)) return fail("RMS height smoothing: work limit exceeded");
    } while(corners || gradient);
    heights=std::move(result);
    return true;
}

bool generateRmsSlopes(uint32_t width, uint32_t height,
    const std::vector<uint8_t> &heights, std::vector<uint8_t> &slopes, std::string *err) {
    if (err) err->clear();
    if (!width || !height || width>1024 || height>1024 || heights.size()!=size_t(width)*height) {
        if (err) *err="RMS slopes: invalid dimensions or height count";
        return false;
    }
    const int w=static_cast<int>(width), h=static_cast<int>(height);
    std::vector<uint8_t> result(heights.size());
    for (int y=0;y<h;++y) for (int x=0;x<w;++x) {
        const int center=heights[y*w+x], high=center+1, low=center-1;
        auto at=[&](int dx,int dy) {
            return x+dx<0 || x+dx>=w || y+dy<0 || y+dy>=h ? center : int(heights[(y+dy)*w+x+dx]);
        };
        const int n=at(0,-1), e=at(1,0), s=at(0,1), west=at(-1,0);
        uint8_t slope=0;
        // Original496170 precedence matters when several neighbors are higher.
        if (n==high && e==high) slope=14;
        else if (west==high && s==high) slope=13;
        else if (n==high && west==high) slope=16;
        else if (e==high && s==high) slope=15;
        else if (n==high) slope=6;
        else if (e==high) slope=8;
        else if (west==high) slope=5;
        else if (s==high) slope=7;
        else if (at(1,-1)==high) slope=west==low && s==low ? 2:10;
        else if (at(-1,1)==high) slope=n==low && e==low ? 1:9;
        else if (at(-1,-1)==high) slope=e==low && s==low ? 3:11;
        else if (at(1,1)==high) slope=n==low && west==low ? 4:12;
        result[y*w+x]=slope;
    }
    slopes=std::move(result);
    return true;
}
}
