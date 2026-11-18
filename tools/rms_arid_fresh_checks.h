// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "../src/engine/game.h"
#include "../src/engine/rms_map.h"

// Fresh original4df9b0 full-stage controls, 2026-11-18; Tiny120,
// two Empire players, fixed starts and default resources. These are map
// generator checkpoints, not proof of complete Windows session parity.
[[gnu::noinline]] static void rmsAridFreshChecks(swgb::Assets &assets) {
    swgb::RmsScript script;std::string error;
    check(script.load(54201,[&](int32_t id,std::vector<uint8_t> &bytes) {
        return assets.readRmsResource(id,bytes,&error);
    },swgb::RmsScript::Limits{},&error),error.c_str());
    const std::unordered_set<std::string> defines{
        "TINY_MAP","FIXED_POSITIONS","DEFAULT_RESOURCES","RANDOM_MAP","2_PLAYERS"};
    const struct {uint32_t seed,count,rng,objects,terrain,elevation,slopes;} cases[]={
        {2,2038,4135318061u,1325605065,1657922548,377194135,4131296099u},
        {3,1969,1588487930,656024466,2222262120u,2899297179u,3096231874u},
        {4,2064,2581643513u,2239854069u,2733658899u,3691845281u,2419950207u}
    };
    for(const auto &c:cases) {
        swgb::RmsMap map;uint32_t rng=c.seed;
        check(swgb::generateRmsMap(script,defines,120,120,assets.dat(),rng,map,&error,2,{1,1}),error.c_str());
        uint32_t hash=2166136261u,count=0,next=0;
        auto record=[&](uint32_t id,int owner,int unit,uint8_t facet,float x,float y) {
            for(uint32_t v:{id,uint32_t(owner),uint32_t(unit),uint32_t(facet),uint32_t(x*2),uint32_t(y*2)})
                hash=(hash^v)*16777619u;
            ++count;
        };
        for(const auto &o:map.cliffs){record(o.creationOrdinal,0,o.unit,uint8_t(o.facet),o.x,o.y);next=o.creationOrdinal+1;}
        for(const auto &o:map.scenery){if(o.alive)record(next,0,o.unitId,o.facet,o.x,o.y);++next;}
        for(const auto &o:map.ownedObjects)record(next++,o.owner,o.unitId,o.facet,o.x,o.y);
        for(const auto &o:map.resourcesAfterOwned)record(next++,o.owner,o.unitId,o.facet,o.x,o.y);
        check(map.objects.empty() && count==c.count && hash==c.objects && rng==c.rng,
              "Fresh original Arid ordered objects and final RNG");
        auto gridHash=[](const auto &grid){uint32_t h=2166136261u;for(auto v:grid)h=(h^v)*16777619u;return h;};
        check(gridHash(map.terrain)==c.terrain && gridHash(map.elevation)==c.elevation && gridHash(map.slopes)==c.slopes,
              "Fresh original Arid complete terrain, elevation and slopes");
        if(c.seed==2) {
            swgb::Game game(assets);rng=c.seed;
            check(game.initRms(script,defines,120,rng,&error,2,{1,1}),error.c_str());
            // Original spawn1960 is native1961. Actual423c68 draw25000:
            // sprite+18 stores1.392590045928955, including the1-second frame.
            check(game.objectUnitId(1961)==293 && game.objectPlayer(1961)==2,
                  "Original Arid precision regression identifies the same worker");
            const auto animation=game.objectAnimationForTesting(1961);
            check(animation[0]==7.f && animation[1]==0.392590045928955078125f,
                  "Original worker hold rounds after last-frame subtraction and survives runtime import");
        }
    }
}
