// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
static void rmsElevationImportChecks(swgb::Assets &assets) {
    swgb::RmsScript script;std::string error;
    const std::string source=R"(<LAND_GENERATION>
base_terrain 0
create_land { terrain_type 6 land_position 50 50 base_size 3 number_of_tiles 160 clumping_factor 8 border_fuzziness 20 }
<TERRAIN_GENERATION>
create_terrain 3 { base_terrain 6 number_of_tiles 30 number_of_clumps 3 spacing_to_other_terrain_types 1 clumping_factor 10 set_flat_terrain_only }
<ELEVATION_GENERATION>
create_elevation 2 { base_terrain 6 number_of_tiles 40 number_of_clumps 2 }
)";
    check(script.load(0,[&](int32_t id,std::vector<uint8_t> &bytes) {
        if(id) return false;bytes.assign(source.begin(),source.end());return true;
    },{},&error),error.c_str());
    swgb::Game game(assets);uint32_t rng=UINT32_MAX;
    check(game.initRms(script,{},16,rng,&error),error.c_str());
    check(rng==2180953274u && game.activeObjectCount()==0,"Original elevation pipeline RNG and empty objects");
    // Original generated tile bytes composed with actual54f820 samples; all17 slopes.
    const float samples[][3]={
        {0.125f,0.75f,0.0f},
        {0.75f,0.125f,0.0f},
        {0.5f,0.5f,0.0f},
        {9.125f,7.75f,1.75f},
        {9.75f,7.125f,1.125f},
        {9.5f,7.5f,1.5f},
        {4.125f,6.75f,1.125f},
        {4.75f,6.125f,1.75f},
        {4.5f,6.5f,1.5f},
        {9.125f,10.75f,1.25f},
        {9.75f,10.125f,1.25f},
        {9.5f,10.5f,1.5f},
        {5.125f,3.75f,1.125f},
        {5.75f,3.125f,1.125f},
        {5.5f,3.5f,1.5f},
        {10.125f,3.75f,0.875f},
        {10.75f,3.125f,0.25f},
        {10.5f,3.5f,0.5f},
        {6.125f,10.75f,1.25f},
        {6.75f,10.125f,1.875f},
        {6.5f,10.5f,1.5f},
        {8.125f,1.75f,0.75f},
        {8.75f,1.125f,0.125f},
        {8.5f,1.5f,0.5f},
        {2.125f,4.75f,0.125f},
        {2.75f,4.125f,0.75f},
        {2.5f,4.5f,0.5f},
        {9.125f,1.75f,0.625f},
        {9.75f,1.125f,0.0f},
        {9.5f,1.5f,0.0f},
        {2.125f,5.75f,0.0f},
        {2.75f,5.125f,0.625f},
        {2.5f,5.5f,0.0f},
        {10.125f,4.75f,0.125f},
        {10.75f,4.125f,0.125f},
        {10.5f,4.5f,0.0f},
        {7.125f,1.75f,0.0f},
        {7.75f,1.125f,0.0f},
        {7.5f,1.5f,0.0f},
        {9.125f,2.75f,0.875f},
        {9.75f,2.125f,0.25f},
        {9.5f,2.5f,0.5f},
        {3.125f,5.75f,0.25f},
        {3.75f,5.125f,0.875f},
        {3.5f,5.5f,0.5f},
        {7.125f,2.75f,0.75f},
        {7.75f,2.125f,0.75f},
        {7.5f,2.5f,0.5f},
        {9.125f,4.75f,0.875f},
        {9.75f,4.125f,0.875f},
        {9.5f,4.5f,0.5f}
    };
    for(const auto &p:samples) check(std::abs(game.elevationForTesting(p[1],16-p[0])-p[2])<0.00001f,"Original RMS triangulated height import");
    swgb::Scenario flat;flat.map.width=flat.map.height=16;flat.map.tiles.resize(256);
    check(game.initScenario(flat,&error),error.c_str());
    for(const auto &p:samples) check(game.elevationForTesting(p[1],16-p[0])==0,"Scenario restart clears RMS tile elevation mode");
    std::puts("PASS: original RMS hill import, all17 triangulations and restart");
}
