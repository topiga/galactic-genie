// SPDX-License-Identifier: GPL-3.0-or-later
#include "rms_cliffs.h"
#include "../core/genie_random.h"
#include <algorithm>
#include <array>

namespace swgb {
bool planRmsCliffs(uint32_t size, const std::vector<uint8_t> &terrain,
    const std::vector<uint8_t> &heights, const RmsCliffSettings &s,
    const std::vector<RmsCliffProtection> &protection, uint32_t &randomState,
    std::vector<RmsCliffSegment> &segments, std::string *err) {
    if (err) err->clear();
    auto fail=[&](const char *message) {
        if (err) *err=std::string("RMS cliffs: ")+message;
        return false;
    };
    if (size<3 || size>1024 || terrain.size()!=size_t(size)*size || heights.size()!=terrain.size())
        return fail("unsupported dimensions or tile arrays");
    if (s.minCount<0 || s.maxCount<s.minCount || s.maxCount>999 ||
        s.minLength<0 || s.maxLength<s.minLength || s.maxLength>1024 ||
        s.curl<0 || s.curl>100 || s.waterDistance<0 || s.waterDistance>1024 ||
        s.cliffDistance<0 || s.cliffDistance>1024 || protection.size()>99 ||
        std::any_of(heights.begin(),heights.end(),[](uint8_t h){return h>7;}))
        return fail("unsupported settings or heights");
    for (const auto &p:protection)
        if (p.x<0 || p.y<0 || uint32_t(p.x)>=size || uint32_t(p.y)>=size || p.radius<0 || p.radius>2048)
            return fail("unsupported protected start");
    const int n=int(size)/3, width=int(size);
    std::vector<uint8_t> mask(n*n,1);
    uint64_t work=0;
    constexpr uint64_t workLimit=64000000;
    auto clear=[&](int x,int y,int radius) {
        const int xmin=std::max(0,x-radius), xmax=std::min(n-1,x+radius);
        const int ymin=std::max(0,y-radius), ymax=std::min(n-1,y+radius);
        work+=uint64_t(xmax-xmin+1)*uint64_t(ymax-ymin+1);
        if (work>workLimit) return false;
        // Original's missing bottom clamp affects unused coarse rows only on square maps.
        for (int yy=std::max(0,y-radius);yy<=std::min(n-1,y+radius);++yy)
            for (int xx=std::max(0,x-radius);xx<=std::min(n-1,x+radius);++xx)
                mask[yy*n+xx]=0;
        return true;
    };
    for (int y=0;y<n;++y) for (int x=0;x<n;++x) {
        bool valid=true, water=false; int elevation=-1;
        for (int yy=y*3;yy<y*3+3;++yy) for (int xx=x*3;xx<x*3+3;++xx) {
            const int t=terrain[yy*width+xx], h=heights[yy*width+xx];
            if (t==1 || t==22 || t==23 || t==4 || t==16 || t==15 || t==35) {
                valid=false; water=true;
            } else if (elevation<0) elevation=h;
            else if (elevation!=h) valid=false;
        }
        if (valid) {
            if (mask[y*n+x]) mask[y*n+x]=uint8_t(elevation+1);
        } else {
            if (water && !clear(x,y,s.waterDistance)) return fail("work limit exceeded");
            mask[y*n+x]=0;
        }
    }
    for (const auto &p:protection)
        if (!clear(p.x/3,p.y/3,p.radius/3+2)) return fail("work limit exceeded");
    for (int y=0;y<n;++y) for (int x=0;x<n;++x)
        if (mask[y*n+x] && (!y || !mask[(y-1)*n+x]) && (!x || !mask[y*n+x-1]) &&
            (y==n-1 || !mask[(y+1)*n+x]) && (x==n-1 || !mask[y*n+x+1])) mask[y*n+x]=0;
    std::vector<int> candidates;
    for (int i=n*n-1;i>=0;--i) if (mask[i]) candidates.push_back(i);
    uint32_t rng=randomState;
    // Same original 0x4d6b90 shuffle used by the elevation stage.
    for (int pass=0;pass<2;++pass) {
        std::array<std::vector<int>,100> buckets;
        for (int i:candidates) buckets[std::min(99u,genieRandomDraw(rng)*100u/32767u)].push_back(i);
        candidates.clear();
        for (size_t b=0;b<100;b+=2)
            while (!buckets[b].empty() || !buckets[b+1].empty())
                for (size_t k=b;k<b+2;++k) if (!buckets[k].empty()) {
                    candidates.push_back(buckets[k].back()); buckets[k].pop_back();
                }
        std::reverse(candidates.begin(),candidates.end());
    }
    auto draw=[&](int low,int high) {return low+int(genieRandomDraw(rng)*uint32_t(high-low)/32767u);};
    std::vector<RmsCliffSegment> result;
    size_t next=0;
    const int count=draw(s.minCount,s.maxCount);
    for (int cliff=0;cliff<count;++cliff) {
        const int length=draw(s.minLength,s.maxLength);
        if (length<3) continue;
        while (next<candidates.size() && !mask[candidates[next]]) ++next;
        if (next==candidates.size()) continue;
        int index=candidates[next++], x=index%n, y=index/n;
        const uint8_t elevation=mask[index];
        std::vector<int> path{index}; mask[index]=0;
        int direction=draw(0,4);
        auto move=[&](int d) {
            // 0x4d7c70 wraps with period THREE outside 0..3, not modulo four.
            if (d>3) d-=((d-1)/3)*3;
            if (d<0) d+=((2-d)/3)*3;
            int xx=x,yy=y;
            if (d==0) --yy; else if (d==1) ++xx; else if (d==2) ++yy; else --xx;
            if (xx<0 || yy<0 || xx>=n || yy>=n || mask[yy*n+xx]!=elevation) return false;
            x=xx; y=yy; return true;
        };
        for (int step=0;step<length;++step) {
            const int roll=draw(0,100);
            if (roll<(s.curl>>1)) direction=direction<1?3:direction-1;
            else if (roll<s.curl) direction=direction<3?direction+1:0;
            if (!move(direction) && !move(direction+1) && !move(direction-1) &&
                (step || !move(direction-2))) break;
            path.push_back(y*n+x); mask[y*n+x]=0;
        }
        int previous=path.back();
        for (auto it=path.rbegin();it!=path.rend();++it) {
            result.push_back({previous%n*3+1,previous/n*3+1,*it%n*3+1,*it/n*3+1});
            previous=*it;
            if (!clear(*it%n,*it/n,s.cliffDistance)) return fail("work limit exceeded");
        }
    }
    segments=std::move(result); randomState=rng;
    return true;
}
} // namespace swgb
