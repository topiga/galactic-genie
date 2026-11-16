// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
static void rmsGaiaFoundationChecks(swgb::Assets &assets,swgb::SoftRenderer &renderer,const std::string &dataDir) {
    swgb::ResourceSet resources;swgb::RmsScript script;std::string error;
    check(resources.add(dataDir+"/gamedata_x1.drs",&error),error.c_str());
    check(script.load(55019,resources,&error),error.c_str());
    const std::unordered_set<std::string> defines{"TINY_MAP","FIXED_POSITIONS","DEFAULT_RESOURCES","RANDOM_MAP","2_PLAYERS"};
    // Original496e80 on completed519/537: unseen0 retains27; explored128 and live15 paint Gaia27.
    for(const auto &fixture:std::vector<std::array<int,5>>{{2217,519,95,9,2},{2223,537,72,37,1}}) {
        const int id=fixture[0],unit=fixture[1],cx=fixture[2],cy=fixture[3],radius=fixture[4];
        for(int visibility:{0,1,2}) {
            swgb::Game game(assets);uint32_t rng=1;
            check(game.initRms(script,defines,120,rng,&error,2,{1,1}),error.c_str());
            check(game.objectUnitId(id)==unit && game.pendingFoundationForTesting(id)==27,"Gaia foundation remains deferred at startup");
            game.setInitiallyExploredForTesting(visibility==1);
            if(visibility==2)check(game.spawnObjectForTesting(1,83,1,float(cx),float(cy))!=0,"Reveal Gaia foundation");
            check(game.tileVisibilityForTesting(cx,cy-1)==visibility,"Gaia foundation visibility fixture");
            std::vector<int> before;
            for(int y=cy-radius-1;y<=cy+radius;++y)for(int x=cx-radius-1;x<=cx+radius;++x)
                before.push_back(game.terrainAtForTesting(x,y));
            game.lookAt(float(cx),float(cy));game.render(renderer,960,544);
            size_t index=0;
            for(int y=cy-radius-1;y<=cy+radius;++y)for(int x=cx-radius-1;x<=cx+radius;++x) {
                const bool inside=x>=cx-radius && x<cx+radius && y>=cy-radius && y<cy+radius;
                check(game.terrainAtForTesting(x,y)==(visibility && inside?27:before[index]),"Original Gaia foundation footprint and untouched border");
                ++index;
            }
            check(game.pendingFoundationForTesting(id)==(visibility?255:27),"Original Gaia fog-mode1 deferred callback consumption");
            if(visibility) {
                check(game.setTerrainForTesting(cx,cy,14),"Replace consumed foundation tile");
                game.render(renderer,960,544);
                check(game.terrainAtForTesting(cx,cy)==14,"Gaia deferred foundation runs only once");
            }
        }
    }
    std::puts("PASS: original Gaia foundation footprints and unseen/explored/visible dispatch");
}
