// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
static void rmsHolocronChecks(const swgb::dat::DatFile &data) {
    const struct { uint32_t entry,final;swgb::RmsPlayerStart starts[2];size_t count;float positions[5][2]; } cases[]={
        {1718704626u,2427565970u,{{37,8},{25,42}},5,{{11.5f,2.5f},{58.5f,49.5f},{0.5f,50.5f},{0.5f,23.5f},{62.5f,0.5f}}},
        {382995709u,3941495841u,{{8,40},{42,22}},3,{{61.5f,59.5f},{36.5f,47.5f},{6.5f,10.5f},{},{}}},
    };
    std::string error;
    for(const auto &f:cases) {
        std::vector<uint8_t> terrain(4096);
        for(int y=f.starts[0].y-2;y<f.starts[0].y+2;++y)
            for(int x=f.starts[0].x-2;x<f.starts[0].x+2;++x) terrain[y*64+x]=27;
        uint32_t rng=f.entry;std::vector<swgb::RmsTimberPlacement> placements;
        const std::vector<swgb::RmsPlayerStart> starts{f.starts[0],f.starts[1]};
        check(swgb::generateRmsHolocronPlacements(64,64,terrain,data,starts,rng,placements,&error),error.c_str());
        check(rng==f.final && placements.size()==f.count,"Original holocron row RNG and partial exhaustion");
        for(size_t i=0;i<f.count;++i) {
            const auto &p=placements[i];
            check(p.unitId==285 && p.x==f.positions[i][0] && p.y==f.positions[i][1] && p.facet==7 && p.headingRadians==0,
                  "Original holocron player exclusion, temporary spacing and static facing");
        }
        terrain[0]=10;rng=f.entry;
        check(!swgb::generateRmsHolocronPlacements(64,64,terrain,data,starts,rng,placements,&error) &&
              rng==f.entry && placements.empty(),"Unproven holocron forest terrain rejects atomically");
        terrain[0]=0;
        check(!swgb::generateRmsHolocronPlacements(32,64,terrain,data,starts,rng,placements,&error) && rng==f.entry,
              "Unproven holocron dimensions reject");
        check(!swgb::generateRmsHolocronPlacements(64,64,terrain,data,{f.starts[0]},rng,placements,&error) && rng==f.entry,
              "Unproven holocron owner counts reject");
    }
    std::puts("PASS: original bounded holocron placement after retained starting units");
}
