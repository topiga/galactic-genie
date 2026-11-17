// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "genie_random.h"
#include <cmath>
#include <cassert>

namespace swgb {
struct GenieStartPosition { float x,y; };

// Original5d1110 empty-primary branch, original world coordinates (before native Y inversion).
// Call only for an empty owner; dimensions30..65535, finite radii0..32767.
// allow(unit,position) implements original master+38; create(unit,position) returns an opaque
// object handle. complete(handle,position) includes camera/completion before workerCount().
// Callbacks share randomState: constructor/completion draws must occur at these boundaries.
// Worker retries are deliberately unbounded, as in the original; caller must supply a viable map.
template<class Allow,class Create,class Complete,class WorkerCount>
bool genieEmptyOwnerStart(uint32_t &randomState,int32_t width,int32_t height,
                          float radiusX,float radiusY,Allow allow,Create create,
                          Complete complete,WorkerCount workerCount) {
    assert(width>=30 && height>=30 && width<=65535 && height<=65535);
    assert(std::isfinite(radiusX) && std::isfinite(radiusY) && radiusX>=0.f && radiusY>=0.f && radiusX<=32767.f && radiusY<=32767.f);
    const float halfX=radiusX-std::trunc(radiusX)>0.f ? .5f:0.f;
    const float halfY=radiusY-std::trunc(radiusY)>0.f ? .5f:0.f;
    auto center=[&]() {
        const auto y=15+int64_t(genieRandomDraw(randomState))*(height-30)/32767;
        const auto x=15+int64_t(genieRandomDraw(randomState))*(width-30)/32767;
        return GenieStartPosition{float(x)+halfX,float(y)+halfY};
    };
    auto position=center();
    int32_t attempt=1;
    for(;attempt<5000;++attempt) {
        if(allow(109,position))break;
        position=center(); // The final sampled pair is unused after check4999 fails.
    }
    if(attempt==5000)return false;
    const auto building=create(109,position);
    complete(building,position);
    const int32_t workers=workerCount();
    constexpr double fraction=double(3.0518509447574615e-05f); // Original float654cc8.
    for(int32_t i=0;i<workers;++i) {
        const int32_t unit=genieRandomDraw(randomState)%100<50 ? 83:293;
        GenieStartPosition worker;
        do {
            const int32_t dy=int32_t(genieRandomDraw(randomState))*6/32767-3;
            const int32_t dx=int32_t(genieRandomDraw(randomState))*6/32767-3;
            worker.y=float(double(position.y)+dy+double(genieRandomDraw(randomState))*fraction);
            worker.x=float(double(position.x)+dx+double(genieRandomDraw(randomState))*fraction);
        } while(worker.x<0.f || worker.y<0.f || worker.x>=width || worker.y>=height || !allow(unit,worker));
        create(unit,worker);
    }
    return true;
}
} // namespace swgb
