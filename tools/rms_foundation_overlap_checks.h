// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
static void overlappingRmsFoundationChecks(swgb::Assets &assets,swgb::SoftRenderer &renderer) {
    const struct { int civs[3],centers[3][2];uint32_t hash;std::vector<std::pair<int,uint8_t>> protectedTiles; } cases[]={
        {{1,2,6},{{8,8},{12,8},{12,8}},927914293u,{}},
        {{1,6,2},{{8,8},{12,8},{12,8}},3773723189u,{}},
        {{1,2,6},{{8,8},{12,8},{13,8}},2425908933u,{}},
        {{1,6,2},{{8,8},{12,8},{13,8}},3455983973u,{}},
        {{1,2,6},{{8,8},{13,8},{12,8}},3455983973u,{}},
        {{1,6,2},{{8,8},{13,8},{12,8}},2425908933u,{}},
        {{1,2,6},{{8,8},{12,8},{12,9}},2090376773u,{}},
        {{1,6,2},{{8,8},{12,8},{12,9}},1950428149u,{}},
        {{1,2,6},{{8,8},{12,9},{12,8}},1950428149u,{}},
        {{1,6,2},{{8,8},{12,9},{12,8}},2090376773u,{}},
        {{1,2,6},{{8,8},{11,7},{12,8}},467206693u,{}},
        {{1,6,2},{{8,8},{11,7},{12,8}},1501613013u,{}},
        {{1,2,6},{{8,8},{9,8},{10,8}},3099010261u,{}},
        {{1,6,2},{{8,8},{9,8},{10,8}},4176240021u,{}},
        {{1,2,6},{{8,8},{8,8},{8,8}},3077363829u,{}},
        {{1,6,2},{{8,8},{8,8},{8,8}},3077363829u,{}},
        {{1,2,6},{{8,8},{12,8},{13,8}},889759853u,{{202,1},{204,1},{206,1},{235,1},{237,1},{266,1},{268,1},{270,1},{299,1},{301,1}}},
        {{1,6,2},{{8,8},{12,8},{13,8}},785507557u,{{202,1},{204,1},{206,1},{235,1},{237,1},{266,1},{268,1},{270,1},{299,1},{301,1}}},
        {{1,2,6},{{8,8},{12,8},{13,8}},3539287621u,{{202,22},{204,22},{206,22},{235,22},{237,22},{266,22},{268,22},{270,22},{299,22},{301,22}}},
        {{1,6,2},{{8,8},{12,8},{13,8}},3507376325u,{{202,22},{204,22},{206,22},{235,22},{237,22},{266,22},{268,22},{270,22},{299,22},{301,22}}},
        {{1,2,6},{{8,8},{12,8},{13,8}},2782017677u,{{202,23},{204,23},{206,23},{235,23},{237,23},{266,23},{268,23},{270,23},{299,23},{301,23}}},
        {{1,6,2},{{8,8},{12,8},{13,8}},2906213u,{{202,23},{204,23},{206,23},{235,23},{237,23},{266,23},{268,23},{270,23},{299,23},{301,23}}},
        {{1,2,6},{{8,8},{12,8},{13,8}},2322497085u,{{202,27},{204,27},{206,27},{235,27},{237,27},{266,27},{268,27},{270,27},{299,27},{301,27}}},
        {{1,6,2},{{8,8},{12,8},{13,8}},2525891013u,{{202,27},{204,27},{206,27},{235,27},{237,27},{266,27},{268,27},{270,27},{299,27},{301,27}}},
    };
    swgb::Game game(assets);std::string error;
    for(const auto &f:cases) {
        swgb::Scenario scenario;
        scenario.map.width=scenario.map.height=32;
        scenario.map.tiles.resize(32*32);
        for(int y=6;y<10;++y) for(int x=6;x<10;++x) scenario.map.tiles[y*32+x].terrain=27;
        for(const auto &tile:f.protectedTiles) scenario.map.tiles[tile.first].terrain=tile.second;
        for(int i=0;i<3;++i) {
            auto &p=scenario.players[i];p.active=true;p.human=i==0;p.populationLimit=200;
            p.civilization=scenario.civilizations[i]=f.civs[i];
            for(int j=1;j<=3;++j) p.diplomacy[j]=j==i+1?0:3;
            swgb::ScenarioUnit u;u.unitId=109;u.player=uint8_t(i+1);u.spawnId=uint32_t(i+1);
            u.state=2;u.x=float(f.centers[i][0]);u.y=float(f.centers[i][1]);
            scenario.units.push_back(u);
        }
        scenario.nextUnitId=4;
        check(game.initScenario(scenario,&error),error.c_str());
        check(game.setPendingFoundationForTesting(2,27) && game.setPendingFoundationForTesting(3,27),
              "Original overlapping pending foundation setup");
        swgb::InputState idle;game.updatePaused(0,idle);
        check(game.pendingFoundationForTesting(2)==27 && game.pendingFoundationForTesting(3)==27,
              "Post-startup visibility alone leaves overlapping foundations pending");
        game.lookAt(8,24);game.render(renderer,960,544);
        check(game.pendingFoundationForTesting(2)==255 && game.pendingFoundationForTesting(3)==255,
              "Visible overlapping foundations consumed even when all terrain writes are protected");
        uint32_t hash=2166136261u;
        for(int y=0;y<32;++y) for(int x=0;x<32;++x)
            hash=(hash^uint32_t(game.terrainAtForTesting(y,31-x)))*16777619u;
        check(hash==f.hash,"Original overlapping foundation tile order and protected-water raster");
    }
    std::puts("PASS: original overlapping/coincident foundation order and protected terrain");
}
