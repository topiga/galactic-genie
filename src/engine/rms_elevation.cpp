// SPDX-License-Identifier: GPL-3.0-or-later
#include "rms_elevation.h"
#include "../core/genie_random.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

namespace swgb {
bool generateRmsElevationRow(uint32_t width, uint32_t height, const RmsElevationRow &row,
    const std::vector<uint8_t> &terrain, uint32_t &randomState,
    std::vector<uint8_t> &heights, std::vector<uint8_t> &protection, std::string *err) {
    if (err) err->clear();
    auto fail = [&](const char *message) {
        if (err) *err = std::string("RMS elevation: ") + message;
        return false;
    };
    if (width < 2 || width > 1024 || width != height || terrain.size() != size_t(width)*height ||
        heights.size() != terrain.size() || protection.size() != terrain.size())
        return fail("unsupported dimensions or array sizes");
    if (!row.elevation || row.elevation > 7 || row.baseHeight >= row.elevation ||
        row.clumps > 999 || row.spacing > 1024 ||
        row.targetTiles > static_cast<uint32_t>(std::numeric_limits<int32_t>::max()) ||
        std::any_of(heights.begin(),heights.end(),[](uint8_t v) { return v>7; }) ||
        std::any_of(protection.begin(),protection.end(),[](uint8_t v) { return v>101; }))
        return fail("unsupported elevation row or tile state");
    auto result = heights, mask = protection;
    uint32_t rng = randomState;
    uint64_t work = 0;
    constexpr uint64_t workLimit = 64000000;
    const int w = static_cast<int>(width), h = static_cast<int>(height);
    std::vector<int> candidates;
    for (size_t i=terrain.size();i-- > 0;)
        if (terrain[i]==row.baseTerrain && result[i]==row.baseHeight && !mask[i])
            candidates.push_back(static_cast<int>(i));
    // 0x4d6b90: two bucket passes, not the terrain worker's partial shuffle.
    for (int pass=0;pass<2;++pass) {
        std::array<std::vector<int>,100> buckets;
        for (int i : candidates)
            buckets[std::min(99u,genieRandomDraw(rng)*100u/32767u)].push_back(i);
        candidates.clear();
        for (size_t b=0;b<100;b+=2)
            while (!buckets[b].empty() || !buckets[b+1].empty())
                for (size_t k=b;k<b+2;++k)
                    if (!buckets[k].empty()) {
                        candidates.push_back(buckets[k].back()); buckets[k].pop_back();
                    }
        std::reverse(candidates.begin(),candidates.end());
    }
    struct Node { int next=-1, previous=-1, owner=-1, priority=0; };
    std::vector<Node> nodes(terrain.size());
    std::vector<int> heads(row.clumps+1,-1);
    for (size_t k=0;k<candidates.size();++k)
        nodes[candidates[k]] = {k+1<candidates.size()?candidates[k+1]:-1,
                               k?candidates[k-1]:-1,0,0};
    if (!candidates.empty()) heads[0]=candidates.front();
    auto remove = [&](int i) {
        auto &n=nodes[i];
        if (n.owner<0) return;
        if (n.previous>=0) nodes[n.previous].next=n.next; else heads[n.owner]=n.next;
        if (n.next>=0) nodes[n.next].previous=n.previous;
        n.next=n.previous=n.owner=-1;
    };
    auto enqueue = [&](int owner,int i,int priority) {
        remove(i);
        int previous=-1,next=heads[owner];
        while (next>=0 && nodes[next].priority<priority) {
            if (++work>workLimit) return false;
            previous=next; next=nodes[next].next;
        }
        nodes[i]={next,previous,owner,priority};
        if (previous>=0) nodes[previous].next=i; else heads[owner]=i;
        if (next>=0) nodes[next].previous=i;
        return ++work<=workLimit;
    };
    auto gate = [&](int x,int y,bool score) {
        const int spacing=static_cast<int>(row.spacing);
        // Seed gate 0x4d9a00 checks the candidate even at spacing0;
        // growth gate 0x4d9ac0 skips the terrain restriction at spacing0.
        if (spacing || !score)
            for (int yy=std::max(0,y-spacing);yy<=std::min(h-1,y+spacing);++yy)
                for (int xx=std::max(0,x-spacing);xx<=std::min(w-1,x+spacing);++xx) {
                    if (++work>workLimit) return 0;
                    if (row.baseTerrain==255 || terrain[yy*w+xx]!=row.baseTerrain ||
                        result[yy*w+xx]<row.baseHeight) return 0;
                }
        if (!score) return 1;
        int count=0;
        for (int yy=std::max(0,y-2);yy<=std::min(h-1,y+2);++yy)
            for (int xx=std::max(0,x-2);xx<=std::min(w-1,x+2);++xx) {
                if (++work>workLimit) return 0;
                count+=result[yy*w+xx]==row.elevation;
            }
        return count;
    };
    const int radius=row.clumps ? std::max(2,static_cast<int>(std::sqrt(
        static_cast<double>(row.targetTiles)/row.clumps))/2) : 2;
    uint32_t created=0,count=0;
    while (created<row.clumps && heads[0]>=0) {
        if (++work>workLimit) return fail("work limit exceeded");
        const int i=heads[0],x=i%w,y=i/w;
        remove(i);
        if (mask[i] || !gate(x,y,false)) continue;
        // 0x4d9dd0 uses clipped x+radius for BOTH upper bounds. Do not repair it.
        const int upper=std::min(w-1,x+radius);
        for (int yy=std::max(0,y-radius);yy<=upper;++yy)
            for (int xx=std::max(0,x-radius);xx<=upper;++xx) {
                if (++work>workLimit) return fail("work limit exceeded");
                remove(yy*w+xx);
            }
        result[i]=row.elevation; ++created; ++count;
        const int xs[]={x-1,x,x+1,x},ys[]={y,y-1,y,y+1};
        for (int k=0;k<4;++k)
            if (xs[k]>=0 && xs[k]<w && ys[k]>=0 && ys[k]<h &&
                !enqueue(static_cast<int>(created),ys[k]*w+xs[k],0))
                return fail("work limit exceeded");
    }
    bool popped;
    do {
        popped=false;
        for (uint32_t owner=1;owner<=row.clumps;++owner) {
            if (count>=row.targetTiles || heads[owner]<0) continue;
            if (++work>workLimit) return fail("work limit exceeded");
            const int i=heads[owner],x=i%w,y=i/w;
            remove(i); popped=true;
            if (mask[i]>genieRandomDraw(rng)*100u/32767u) { mask[i]=101; continue; }
            const int score=gate(x,y,true);
            if (result[i]!=row.baseHeight || !score) continue;
            result[i]=row.elevation; ++count;
            const int priority=250-15*score;
            const int xs[]={x-1,x+1,x,x},ys[]={y,y,y-1,y+1};
            for (int k=0;k<4;++k)
                if (xs[k]>=0 && xs[k]<w && ys[k]>=0 && ys[k]<h &&
                    result[ys[k]*w+xs[k]]==row.baseHeight &&
                    !enqueue(static_cast<int>(owner),ys[k]*w+xs[k],
                        priority+static_cast<int>(genieRandomDraw(rng)*100u/32767u)))
                    return fail("work limit exceeded");
        }
    } while (popped);
    if (work>workLimit) return fail("work limit exceeded");
    heights.swap(result); protection.swap(mask); randomState=rng;
    return true;
}
} // namespace swgb
