// SPDX-License-Identifier: GPL-3.0-or-later
#include "engine/rms_zones.h"
#include <cassert>
#include <limits>
#include <cstdio>
using namespace swgb;
int main() {
    // Original624ee0/625560 byte fixtures; local rms-passability-zone-oracle-2026-11-14.
    struct Case { unsigned size; const char *terrain,*labels; std::vector<float> mask; unsigned count; };
    const Case cases[]={
        {2,"0000","00000000",{1.0f,0.0f,2.0f},1},
        {2,"0000","00000000",{1.0f,-1.0f,0.0f},1},
        {2,"0000","00000000",{0.0f,0.0f,1.0f},1},
        {3,"000000000","000000000000000000",{1.0f,0.0f,2.0f},1},
        {3,"010101010","000102030405060708",{1.0f,0.0f,2.0f},9},
        {3,"010010010","000102000102000102",{1.0f,0.0f,2.0f},3},
        {3,"110121111","000001000200000000",{1.0f,0.0f,2.0f},3},
        {5,"0000000000000000000000000","00000000000000000000000000000000000000000000000000",{1.0f,0.0f,2.0f},1},
        {5,"0101010101010101010101010","000102030405060708090a0b0c0d0e0f101112131415161718",{1.0f,-1.0f,0.0f},25},
        {5,"0010000100001000010000100","00000102020000010202000001020200000102020000010202",{1.0f,-1.0f,0.0f},3},
        {5,"1101211111202010021222010","00000100000000000000000200030002020000000404050006",{1.0f,-1.0f,0.0f},7},
        {8,"0000000000000000000000000000000000000000000000000000000000000000","00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000",{1.0f,0.0f,2.0f},1},
        {8,"0101010110101010010101011010101001010101101010100101010110101010","000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f202122232425262728292a2b2c2d2e2f303132333435363738393a3b3c3d3e3f",{1.0f,-1.0f,0.0f},64},
        {8,"0000100000001000000010000000100000001000000010000000100000001000","00000000010202020000000001020202000000000102020200000000010202020000000001020202000000000102020200000000010202020000000001020202",{1.0f,-1.0f,0.0f},3},
        {8,"1101211111202010021222010202112011122021121020021222021102120020","000001000000000000000002000300040500000000000607050008000000000900000000000a00000000000b000a0a00000000000c0000000d0000000c0c000e",{1.0f,-1.0f,0.0f},15},
        {8,"0201011121001011202112020100022012012020112010201101220022102122","0001020304050505010102020607050501080606060609050a0b0c0c0c06060d0b0b0c0e0e0f060d0b0b0b100e0f060d0b0b110e0e0e0d0d0b0b0b120e0e0e0e",{1.0f,-1.0f,0.0f},19},
    };
    for(const auto &test:cases) {
        std::vector<uint8_t> terrain;
        for(const char *p=test.terrain;*p;++p) terrain.push_back(uint8_t(*p-'0'));
        RmsZones zones; std::string error;
        assert(generateRmsZones(test.size,terrain,test.mask,zones,&error));
        assert(error.empty() && zones.count==test.count && zones.tiles.size()==terrain.size());
        for(size_t i=0;i<zones.tiles.size();++i) {
            unsigned expected=0; assert(std::sscanf(test.labels+2*i,"%2x",&expected)==1);
            assert(zones.tiles[i]==expected);
        }
    }
    // Original checkerboard oracles: every tile is its own component; IDs wrap at255.
    for(unsigned size:{16u,24u,32u}) {
        std::vector<uint8_t> terrain(size*size);
        for(unsigned y=0;y<size;++y) for(unsigned x=0;x<size;++x) terrain[y*size+x]=(x+y)%2;
        RmsZones zones; assert(generateRmsZones(size,terrain,{1,0},zones));
        assert(zones.count==255);
        for(unsigned i=0;i<size*size;++i) assert(zones.tiles[i]==i%255);
    }
    RmsZones zones; std::string error;
    assert(generateRmsZones(1,{0},{1},zones,&error));
    assert(!generateRmsZones(2,{0},{1},zones,&error) && zones.tiles.empty() && !error.empty());
    assert(!generateRmsZones(1,{1},{1},zones,&error) && zones.count==0);
    assert(!generateRmsZones(0,{}, {},zones));
    assert(!generateRmsZones(1025,{}, {1},zones));
    assert(!generateRmsZones(1,{0},{std::numeric_limits<float>::quiet_NaN()},zones));
    assert(!generateRmsZones(1,{0},{std::numeric_limits<float>::infinity()},zones));
    assert(generateRmsZones(2,{0,1,2,0},{1,2,-1},zones,&error));
    assert((zones.tiles==std::vector<uint8_t>{0,0,1,0}));
    std::puts("PASS: original zone bytes, positive-mask connectivity, ID wrapping and validation");
}
