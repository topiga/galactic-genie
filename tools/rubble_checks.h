// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

static void rubbleChecks(swgb::Assets &assets, swgb::SoftRenderer &renderer) {
    // Original sprites are selected by civilization and dead-unit link, not by a
    // replacement texture or a guessed footprint-to-sprite table.
    for(int civ:{1,2}) {
        swgb::Game game(assets);std::string error;
        check(game.init(0x5A17,64,&error,civ,civ),error.c_str());
        game.setClassicHud(true);
        std::vector<uint32_t> buildings;
        for(int unit:{50,68,70,72,317}) {
            const auto &prototype=assets.dat().civs.at(civ).units.at(unit);
            check(prototype.exists && prototype.deadUnitId>=0,"Building has original dead-unit asset");
            const auto &dead=assets.dat().civs.at(civ).units.at(prototype.deadUnitId);
            const auto *graphic=assets.dat().graphic(dead.standingGraphic[0]);
            check(unit==50 || (graphic && (graphic->slp>=0 ? assets.sheet(graphic->slp,0)!=nullptr : !graphic->deltas.empty())),
                  "Original rubble has a sprite or composite graphic");
            const auto id=game.spawnObjectForTesting(civ,unit,1,8.5f+buildings.size()*6,8.5f);
            check(id!=0,"Create rubble comparison building");
            check(game.spawnObjectForTesting(civ,83,1,8.5f+buildings.size()*6,10.5f)!=0,
                  "Observer keeps rubble visible after building loses sight");
            buildings.push_back(id);
        }
        const auto replacement=game.spawnObjectForTesting(civ,50,1,8.5f,16.5f);
        check(replacement!=0,"Replacement farm fixture");
        const auto initial=game.combatStats().activeRemains;
        for(auto id:buildings) check(game.damageObjectForTesting(id,100000),"Destroy rubble comparison building");
        check(game.combatStats().activeRemains==initial+buildings.size(),"Each destroyed building creates remains");
        for(int i=0;i<50;++i) game.update(.1f,{});
        check(game.terrainAtForTesting(8,8)==(civ==2?31:8),"Destroyed farm uses original exhausted terrain");
        const auto alive=game.combatStats().activeRemains;
        check(alive==initial+buildings.size(),"Rubble survives the destruction animation");
        game.lookAt(20.5f,8.5f);game.render(renderer,960,544);
        check(renderer.savePng("rubble-civ-"+std::to_string(civ)+".png"),"Save original rubble asset comparison");
        game.updatePaused(180,{});
        check(game.combatStats().activeRemains==alive,"Pause does not age rubble");
        check(game.damageObjectForTesting(replacement,100000),"Destroy replacement site farm");
        const auto rebuilt=game.spawnObjectForTesting(civ,50,1,8.5f,16.5f);
        check(rebuilt!=0,"Rebuild before old field decays");
        game.update(.1f,{});
        const int rebuiltTerrain=game.terrainAtForTesting(8,16);
        for(int i=0;i<750;++i) game.update(.1f,{});
        check(game.combatStats().activeRemains==initial+2,"Ordinary rubble decays before exhausted farms");
        check(game.terrainAtForTesting(8,8)==(civ==2?31:8),"Exhausted farm remains beyond one minute");
        for(int i=0;i<1100;++i) game.update(.1f,{});
        check(game.combatStats().activeRemains==initial,"Farm dead-unit lifetime eventually expires");
        check(game.terrainAtForTesting(8,8)==(civ==2?4:6),"Expired farm leaves civilization-specific ground");
        check(game.terrainAtForTesting(8,16)==rebuiltTerrain,"Old farm cleanup preserves replacement field");
        check(game.damageObjectForTesting(rebuilt,100000),"Destroy rebuilt field");
        for(int i=0;i<1850;++i) game.update(.1f,{});
        check(game.terrainAtForTesting(8,16)==(civ==2?4:6),"Repeated farm decay leaves civilization-specific ground");
        if(civ==1) {
            const auto older=game.spawnObjectForTesting(civ,50,1,8.5f,16.5f);
            game.update(.1f,{});check(game.damageObjectForTesting(older,100000),"Old overlapping dead field");
            for(int i=0;i<1600;++i) game.update(.1f,{});
            const auto newer=game.spawnObjectForTesting(civ,50,1,8.5f,16.5f);
            game.update(.1f,{});check(game.damageObjectForTesting(newer,100000),"New overlapping dead field");
            for(int i=0;i<250;++i) game.update(.1f,{});
            check(game.terrainAtForTesting(8,16)==8,"Old decay does not prematurely erase a newer exhausted field");
            for(int i=0;i<1600;++i) game.update(.1f,{});
            check(game.terrainAtForTesting(8,16)==6,"Newest exhausted field eventually leaves dirt terrain");
        }
    }
    std::printf("PASS: original farm, food processor, house, wall and airbase rubble assets, pause and decay\n");
}
