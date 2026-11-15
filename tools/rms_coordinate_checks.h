// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
static void rmsCoordinateChecks(swgb::Assets &assets) {
    using namespace swgb;
    const std::string source="<LAND_GENERATION> base_terrain 6 create_player_lands { terrain_type 6 base_size 3 number_of_tiles 1000 } "
        "<CLIFF_GENERATION> <TERRAIN_GENERATION> create_terrain 0 { base_terrain 6 number_of_tiles 2000 number_of_clumps 20 spacing_to_other_terrain_types 2 clumping_factor 20 }";
    RmsScript script;RmsMap map;std::string error;
    check(script.load(0,[&](int32_t id,std::vector<uint8_t> &out) {
        if(id)return false;out.assign(source.begin(),source.end());return true;
    },{},&error),error.c_str());
    uint32_t rng=1;
    check(generateRmsMap(script,{},120,120,assets.dat(),rng,map,&error),error.c_str());
    Scenario scenario;scenario.map.width=scenario.map.height=120;scenario.map.tiles.resize(14400);
    for(size_t i=0;i<map.terrain.size();++i)scenario.map.tiles[i].terrain=map.terrain[i];
    for(const auto &o:map.cliffs) {
        ScenarioUnit unit;unit.unitId=o.unit;unit.spawnId=o.creationOrdinal+1;unit.state=2;unit.x=o.x;unit.y=o.y;
        scenario.units.push_back(unit);scenario.nextUnitId=unit.spawnId+1;
    }
    Game expected(assets),actual(assets);
    check(expected.initScenario(scenario,&error),error.c_str());rng=1;
    check(actual.initRms(script,{},120,rng,&error),error.c_str());
    for(const auto &o:map.cliffs)
        check(actual.objectPosition(o.creationOrdinal+1)==expected.objectPosition(o.creationOrdinal+1),
            "RMS and scenario imports must use the same original world coordinates");
    for(int y=0;y<120;++y)for(int x=0;x<120;++x)
        check(actual.terrainAtForTesting(x,y)==expected.terrainAtForTesting(x,y),"RMS terrain uses scenario world axes");
    const auto workerA=actual.spawnObjectForTesting(1,83,1,3.5f,3.5f),workerE=expected.spawnObjectForTesting(1,83,1,3.5f,3.5f);
    check(workerA && workerE,"coordinate comparison workers");
    size_t asymmetric=0;
    for(const auto &o:map.cliffs) {
        const auto &unit=assets.dat().civs[0].units[o.unit];
        if(unit.collisionSize[0]==unit.collisionSize[1])continue;
        ++asymmetric;const auto center=expected.objectPosition(o.creationOrdinal+1);
        for(float dx:{-1.75f,-1.1f,0.f,1.1f,1.75f})for(float dy:{-1.75f,-1.1f,0.f,1.1f,1.75f})
            check(actual.positionPassableForTesting(workerA,center[0]+dx,center[1]+dy)==
                expected.positionPassableForTesting(workerE,center[0]+dx,center[1]+dy),"RMS cliff collision boxes align with original scenario axes");
    }
    check(asymmetric>0,"coordinate regression covers narrow cliffs");
}
