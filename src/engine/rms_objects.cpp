// SPDX-License-Identifier: GPL-3.0-or-later
#include "rms_objects.h"
#include "../core/genie_random.h"

namespace swgb {
bool generateRmsObjectCandidates(uint32_t width,uint32_t height,uint32_t &randomState,
                                 std::vector<uint32_t> &out,std::string *err) {
    out.clear();
    if (err) err->clear();
    if (!width || !height || width>1024 || height>1024) {
        if (err) *err="RMS object candidates: dimensions must be 1..1024";
        return false;
    }
    const uint32_t count=width*height, sentinel=count;
    std::vector<uint32_t> next(count+1),previous(count+1);
    for (uint32_t i=0;i<count;++i) {
        next[i]=i+1;
        previous[i]=i?i-1:sentinel;
    }
    next[sentinel]=0;
    previous[sentinel]=count-1;
    uint32_t rng=randomState;
    if (width>3 && height>3) {
        const uint32_t dx=width-2,dy=height-2;
        for (uint32_t i=0;i<dx*dy/4;++i) {
            const uint32_t x=genieRandomDraw(rng)*dx/32767u;
            const uint32_t y=genieRandomDraw(rng)*dy/32767u;
            const uint32_t tile=y*width+x;
            // Original unlink/insert-after-sentinel moves nodes; repeated draws do not duplicate them.
            next[previous[tile]]=next[tile];
            previous[next[tile]]=previous[tile];
            next[tile]=next[sentinel];
            previous[tile]=sentinel;
            previous[next[sentinel]]=tile;
            next[sentinel]=tile;
        }
    }
    out.reserve(count);
    for (uint32_t tile=next[sentinel];tile!=sentinel;tile=next[tile]) out.push_back(tile);
    randomState=rng;
    return true;
}
} // namespace swgb
