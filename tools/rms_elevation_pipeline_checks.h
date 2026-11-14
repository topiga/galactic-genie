// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "engine/rms_script.h"
#include "engine/rms_elevation.h"
#include "engine/rms_terrain.h"
#include "engine/rms_terrain_cleanup.h"
#include "core/genie_random.h"
#include "core/genie_dat.h"
#include <cassert>
// Actual original scheduler4d6660 and LAND/ELEVATION/TERRAIN modules.
// Private rms-land-elevation-terrain-oracle-2026-11-14.json: nine cases also
// repeated with reversed module insertion order, producing identical results.
// Input seed is AFTER parser/player setup, before scheduler entry. No object stage.
static void checkRmsElevationPipeline() {
    struct Case {uint32_t seed;bool elevation,empty;uint32_t terrainHash,heightHash,slopeHash,finalRng;};
    const Case cases[] = {
        {1u,false,false,2076826315u,3869364677u,3869364677u,3016612915u},
        {1u,true,true,1954005839u,3869364677u,3869364677u,2392446512u},
        {1u,true,false,2417917839u,2263797398u,3251090577u,69003991u},
        {12345u,false,false,2635718178u,3869364677u,3869364677u,852461111u},
        {12345u,true,true,2433540205u,3869364677u,3869364677u,3491538824u},
        {12345u,true,false,3195485863u,117374749u,4020851852u,407135790u},
        {4294967295u,false,false,1218991605u,3869364677u,3869364677u,3402451769u},
        {4294967295u,true,true,1455171130u,3869364677u,3869364677u,3402451769u},
        {4294967295u,true,false,36948743u,2466080639u,1297753758u,2904799644u},
    };
    auto hash=[](const std::vector<uint8_t> &a){uint32_t h=2166136261u;for(auto v:a)h=(h^v)*16777619u;return h;};
    swgb::dat::TerrainBlock terrainData;
    terrainData.terrains.resize(7);terrainData.terrainsUsed=7;
    terrainData.terrains[0].enabled=terrainData.terrains[3].enabled=terrainData.terrains[6].enabled=1;
    for(const auto &c:cases) {
        uint32_t rng=c.seed;std::string error;std::vector<uint8_t> terrain,heights(256),slopes(256),mask;
        swgb::genieRandomDraw(rng);
        swgb::RmsLandGrowth land{6,8,8,3,160,8,0,20};
        assert(swgb::growRmsLands(16,16,0,{land},terrainData,rng,terrain,&error));
        assert(swgb::smoothRmsTerrain(16,16,0,terrain,terrain,&error));
        swgb::genieRandomDraw(rng);
        if(c.elevation) {
            std::vector<swgb::RmsElevationRow> rows;
            if(!c.empty) rows={{40,1,2,2,0,6},{40,2,2,1,1,6}};
            assert(swgb::generateRmsElevation(16,16,rows,terrain,rng,heights,slopes,mask,{},&error));
            swgb::genieRandomDraw(rng);
        }
        swgb::RmsTerrainRow row{30,3,6,3,1,10,false};row.flatTerrainOnly=true;
        assert(swgb::generateRmsTerrain(16,16,{row},rng,terrain,&error,{},heights,slopes));
        assert(swgb::smoothRmsTerrain(16,16,1,terrain,terrain,&error));
        assert(swgb::cleanupRmsTerrain(16,16,terrain,&error));
        swgb::genieRandomDraw(rng);
        assert(hash(terrain)==c.terrainHash && hash(heights)==c.heightHash && hash(slopes)==c.slopeHash && rng==c.finalRng);
    }
}
