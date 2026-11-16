// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "../src/engine/rms_map.h"

static void rmsSessionPlayerChecks(swgb::Assets &assets) {
    const std::string source="<LAND_GENERATION> base_terrain 6 create_land { terrain_type 6 "
        "land_position 50 50 base_size 0 number_of_tiles 0 }";
    swgb::RmsScript script;std::string error;
    check(script.load(60000,[&](int32_t id,std::vector<uint8_t> &bytes) {
        if(id!=60000)return false;
        bytes.assign(source.begin(),source.end());return true;
    },{},&error),error.c_str());
    swgb::Game game(assets);
    for(uint32_t count=3;count<=8;++count) {
        std::vector<uint16_t> civs(count);
        for(size_t i=0;i<civs.size();++i)civs[i]=i%2?1:6;
        for(bool allied:{false,true}) {
            std::vector<std::vector<uint8_t>> diplomacy(count+1,std::vector<uint8_t>(count+1,3));
            for(size_t p=0;p<=count;++p)for(size_t q=0;q<=count;++q)
                if(p==q || (allied && p && q))diplomacy[p][q]=0;
            uint32_t rng=1;
            check(game.initRms(script,{},24,rng,&error,count,civs,diplomacy),error.c_str());
            check(game.activeObjectCount()==0,"Empty RMS keeps declared session players without inventing objects");
            for(size_t p=1;p<=count;++p) {
                const std::vector<int> expected=allied
                    ?std::vector<int>{civs[p-1]==6?9:4,10,15}
                    :(civs[p-1]==6?std::vector<int>{9,15}:std::vector<int>{4,10});
                check(game.civilizationEffectsForTesting(int(p))==expected,
                      "Players without starting objects still contribute directional civilization/team bonuses");
                const auto *unit=game.unitPrototypeForTesting(int(p),4);
                check(unit && game.unitSearchRadiusForTesting(int(p),*unit)==(allied || civs[p-1]==6?7:5),
                      "Empty-player setup reaches actual per-owner unit effects");
            }
        }
    }
    const auto retained=game.civilizationEffectsForTesting(1);
    for(const auto &civs:std::vector<std::vector<uint16_t>>{{6},{6,0},{6,9},{6,65535}}) {
        uint32_t rng=12345;
        check(!game.initRms(script,{},24,rng,&error,2,civs) && rng==12345 &&
              game.civilizationEffectsForTesting(1)==retained,
              "Invalid explicit player setup is rejected without replacing the current session");
    }
    uint32_t rng=1;
    check(game.initRms(script,{},24,rng,&error),error.c_str());
    check(game.civilizationEffectsForTesting(1).empty(),"Unspecified civilizations retain terrain-only diagnostic mode");
    std::puts("PASS: explicit RMS players survive empty starts, allied bonuses and invalid setup rejection");
}
