// SPDX-License-Identifier: GPL-3.0-or-later
#include "../src/core/fog_edges.h"
#include <fstream>
#include <iterator>

static void fogEdgeChecks(const std::string &dataPath) {
    auto read=[](const std::string &path) {
        std::ifstream input(path,std::ios::binary);
        check(bool(input),"Open original fog edge fixture");
        return std::vector<uint8_t>(std::istreambuf_iterator<char>(input),{});
    };
    auto tiles=read(dataPath+"/TileEdge.Dat"),black=read(dataPath+"/BlkEdge.Dat");
    swgb::FogEdges edges;std::string error;
    check(edges.parse(tiles,black,&error),error.c_str());
    check(edges.shape(0,0).clear.size()==49 && edges.shape(0,0).fog.empty() && edges.shape(0,0).black.empty(),"Original flat no-edge mask is the full 97x49 tile");
    check(edges.height(1)==25 && edges.height(2)==73 && edges.height(0)==49,"Original slope edge dimensions");
    std::set<int> patterns;
    for(int i=0;i<256;++i) {const int p=swgb::FogEdges::pattern(uint8_t(i));check(p<47,"Fog pattern bounds");patterns.insert(p);}
    check(patterns.size()==47 && swgb::FogEdges::pattern(0x89)==swgb::FogEdges::pattern(0x80) &&
          swgb::FogEdges::pattern(0x13)==swgb::FogEdges::pattern(0x10),"Original cardinal-edge normalization suppresses redundant corners");
    auto bad=tiles;bad.pop_back();check(!edges.parse(bad,black,&error),"Reject truncated fog span list");
    bad=tiles;bad[0]=0;check(!edges.parse(bad,black,&error),"Reject invalid fog table offset");
    bad=tiles;bad[446]=250;check(!edges.parse(bad,black,&error),"Reject fog span outside original tile");
    check(edges.parse(tiles,black,&error),"Valid fog data remains loadable after rejected input");

    swgb::SoftRenderer renderer;
    const uint8_t opaque=255;
    auto *mask=renderer.createMaskTexture(1,1,&opaque);
    for(float zoom:{1.f,.5f}) {
        renderer.beginFrame(8,8,zoom,100,110,120);
        renderer.drawFog(mask,{-2/zoom,-1/zoom,12/zoom,10/zoom,0,0,1,1},1,2,3);
        renderer.endFrame();const auto &pixels=renderer.pixels();
        for(int y=0;y<8;++y) for(int x=0;x<8;++x) {
            const size_t i=size_t(y*8+x)*4;const bool shaded=(x+y)%2!=0;
            check(pixels[i]==(shaded?1:100) && pixels[i+1]==(shaded?2:110) && pixels[i+2]==(shaded?3:120),"Fog checker remains at destination pixel parity under clipping and zoom");
        }
    }
    renderer.destroyTexture(mask);
    std::printf("PASS: original fog edge decoder, slope masks, malformed input and destination checkerboard\n");
}
