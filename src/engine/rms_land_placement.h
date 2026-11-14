// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
namespace swgb {
struct RmsLandPlacement {
    int x=-1,y=-1,radius=3,separation=0;
    int left=0,top=0,right=-1,bottom=-1;
    int minPlacementDistance=-1;
};
// Original cross-check0x4dff10..0x4e012c, after player centers are resolved.
// Walks original land order; either negative coordinate resamples BOTH.
// Explicit centers consume no draws. Metadata owner/landID/zone is separate.
// Corner rejections do not decrement the original990 collision retries; the
// native total-attempt budget additionally bounds that otherwise unbounded loop.
// Failure preserves every input center and RNG.
bool resolveRmsLandPositions(uint32_t width,uint32_t height,
    std::vector<RmsLandPlacement> &lands,uint32_t &randomState,
    std::string *err=nullptr,size_t maxAttempts=1024*1024);
}
