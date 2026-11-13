// SPDX-License-Identifier: GPL-3.0-or-later
#include "engine/rms_terrain_cleanup.h"
#include <cstdlib>
#include <iostream>

static void check(bool ok, const std::string &message) {
    if (!ok) { std::cerr<<message<<'\n'; std::exit(1); }
}
int main() {
    using namespace swgb;
    // Original 0x4e6040 over all byte IDs, each water type and all nine positions.
    const uint8_t waters[]={1,4,22,23};
    const uint32_t expected[]={982760469u,4021962517u,370115077u,925266645u};
    std::string error;
    for (size_t i=0;i<4;++i) {
        uint32_t hash=2166136261u;
        for (unsigned type=0;type<256;++type) for (size_t position=0;position<9;++position) {
            std::vector<uint8_t> terrain(9,3);
            terrain[4]=static_cast<uint8_t>(type); terrain[position]=waters[i];
            check(cleanupRmsTerrain(3,3,terrain,&error),error);
            for (auto tile:terrain) hash=(hash^tile)*16777619u;
        }
        check(hash==expected[i],"original water cleanup corpus mismatch");
    }
    struct Case { uint32_t width,height; std::vector<uint8_t> input,output; };
    const Case cases[]={
        {2,3,{2,34,36,34,22,34},{2,34,2,37,22,37}},
        {2,1,{35,66},{35,66}},
        {1,2,{2,33},{2,33}},
        {2,1,{147,32},{147,32}},
        {1,1,{23},{23}},
        {4,3,{47,36,47,23,2,110,4,33,33,118,37,33},{47,2,37,23,2,2,4,37,33,2,37,37}},
        {3,3,{33,35,33,33,35,4,37,35,23},{33,37,37,33,37,4,37,37,23}},
        {2,2,{4,4,4,34},{4,4,4,37}},
    };
    for (const auto &c:cases) {
        auto terrain=c.input;
        check(cleanupRmsTerrain(c.width,c.height,terrain,&error) && terrain==c.output,
              "original mixed-water cleanup mismatch");
    }
    std::vector<uint8_t> terrain={1,3};
    const auto before=terrain;
    check(!cleanupRmsTerrain(0,2,terrain,&error) && terrain==before && !error.empty(),"zero dimension must preserve input");
    check(!cleanupRmsTerrain(1025,1,terrain,&error) && terrain==before,"oversized dimensions must preserve input");
    check(!cleanupRmsTerrain(2,2,terrain,&error) && terrain==before,"wrong raster size must preserve input");
    std::cout<<"9216 original water-cleanup cases and boundary checks passed\n";
}
