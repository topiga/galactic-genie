// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstring>
#include "../src/engine/rms_walls.h"
// Actual derived4e7420/4df9b0, raw54208 Tiny/default settings; local tiny-derived findings, 2026-11-16.
static void rmsFortressChecks(swgb::Assets &assets,swgb::ResourceSet &resources) {
    swgb::RmsScript script;std::string error;
    check(script.load(54208,resources,&error),error.c_str());
    const std::unordered_set<std::string> defines{"TINY_MAP","FIXED_POSITIONS","DEFAULT_RESOURCES"};
    struct Case {uint32_t seed,count,rng,objects,terrain,heights,slopes,cost1,cost2,ownedHeightHealth,obstruction,directions;};
    const Case cases[]={
        {1u,1852u,472822087u,2066791139u,695987317u,370085139u,2196496358u,2200u,2160u,1319479453u,2795300037u,3497619881u},
        {12345u,1808u,3283216374u,733826055u,917820985u,2894856452u,830763451u,2085u,2160u,337729919u,3530970053u,85941139u},
    };
    const auto bits=[](float value){uint32_t out;std::memcpy(&out,&value,sizeof out);return out;};
    const auto hash=[](const auto &values){uint32_t out=2166136261u;for(auto v:values)out=(out^uint32_t(v))*16777619u;return out;};
    for(const auto &c:cases) {
        swgb::RmsMap map;uint32_t rng=c.seed;
        check(swgb::generateRmsMap(script,defines,120,120,assets.dat(),rng,map,&error,2,{1,1}),error.c_str());
        check(rng==c.rng && hash(map.terrain)==c.terrain && hash(map.elevation)==c.heights && hash(map.slopes)==c.slopes,
              "Original derived-reader Fortress terrain, height, slope and final RNG");
        check(map.objects.empty(),"Fortress has no pre-owned object rows");
        uint32_t next=0,count=0;std::vector<uint32_t> records;
        auto record=[&](uint32_t id,int owner,int unit,int facet,float x,float y) {
            records.insert(records.end(),{id,uint32_t(owner),uint32_t(unit),uint32_t(facet),bits(x),bits(y)});++count;
        };
        for(const auto &o:map.cliffs){record(o.creationOrdinal,0,o.unit,o.facet,o.x,o.y);next=o.creationOrdinal+1;}
        for(const auto &o:map.scenery){if(o.alive)record(next,0,o.unitId,o.facet,o.x,o.y);++next;}
        for(const auto &o:map.ownedObjects)record(next++,o.owner,o.unitId,o.facet,o.x,o.y);
        const auto resourceBase=next+1;
        for(const auto &o:map.resourcesAfterOwned)record(next++,o.owner,o.unitId,o.facet,o.x,o.y);
        check(count==c.count && hash(records)==c.objects,"Original complete Fortress object order, IDs, owners, units, positions and facets");
        swgb::RmsWallInput wallInput;wallInput.width=wallInput.height=120;wallInput.terrain=map.terrain;
        wallInput.restrictions=assets.dat().terrainRestrictions[25].passableBuildableDmgMultiplier;
        auto obstacle=[&](int id,int owner,float x,float y) {
            const auto &u=assets.dat().civs[owner?1:0].units[id];
            const float rx=u.collisionSize[assets.dat().footprintAxesSwapped?1:0],ry=u.collisionSize[assets.dat().footprintAxesSwapped?0:1];
            wallInput.objects.push_back({owner,x,y,rx,ry,u.obstructionType,u.obstructionClass,u.cls});
            if(id==109)wallInput.commandCenters.push_back({owner,x,y,rx,ry});
        };
        for(const auto &o:map.cliffs)obstacle(o.unit,0,o.x,o.y);
        for(const auto &o:map.scenery)if(o.alive)obstacle(o.unitId,0,o.x,o.y);
        for(const auto &o:map.ownedObjects)obstacle(o.unitId,o.owner,o.x,o.y);
        for(const auto &o:map.resourcesAfterOwned) {
            if(o.unitId==117 || o.unitId==64 || o.unitId==88)break;
            obstacle(o.unitId,o.owner,o.x,o.y);
        }
        swgb::RmsWallBoundary boundary;
        check(swgb::generateRmsWallBoundary(wallInput,boundary,&error),error.c_str());
        check(hash(boundary.obstruction)==c.obstruction && hash(boundary.directions)==c.directions,"Original Fortress obstruction and retained direction cache, including untouched water cells");
        swgb::Game game(assets);rng=c.seed;
        check(game.initRms(script,defines,120,rng,&error,2,{1,1}),error.c_str());
        check(rng==c.rng && game.activeObjectCount()==c.count,"Fortress import retains original RNG and object count");
        std::vector<uint32_t> ownedHeightHealth;
        for(size_t n=0;n<records.size();n+=6)if(records[n+1]) {
            const auto id=records[n]+1;const auto pos=game.objectPosition(id);
            ownedHeightHealth.insert(ownedHeightHealth.end(),{records[n],bits(game.elevationForTesting(pos[0],pos[1])),bits(game.objectHitPoints(id))});
        }
        check(hash(ownedHeightHealth)==c.ownedHeightHealth,"Original Fortress owned object heights on slopes and HP");
        check(game.resource(0,98)==450 && game.resource(0,164)==450 && game.resource(0,165)==0 && game.resource(0,11)==5,"Original Fortress Gaia counters");
        for(int owner:{1,2}) {
            const auto cost=owner==1?c.cost1:c.cost2;
            check(game.resource(owner,98)==cost && game.resource(owner,165)==cost-200 && game.resource(owner,164)==200 &&
                  game.resource(owner,4)==21 && game.resource(owner,11)==4 && game.resource(owner,21)==3,"Original Fortress building costs and population");
            check(game.technologyOrderForTesting(owner)==std::vector<int>({0,486,22,451,11,10,16,31,85,209,20}),"Original Fortress completion research order");
        }
        for(size_t n=0;n<map.resourcesAfterOwned.size();++n) {
            const auto &o=map.resourcesAfterOwned[n];const uint32_t id=resourceBase+uint32_t(n);const auto pos=game.objectPosition(id);
            check(game.objectActive(id) && game.objectUnitId(id)==o.unitId && game.objectPlayer(id)==o.owner &&
                  std::abs(pos[0]-o.y)<1e-5f && std::abs(pos[1]-(120-o.x))<1e-5f,"Fortress native import preserves every trailing object");
            if(!o.owner)continue;
            const int hp=o.unitId==79?1250:o.unitId==70?800:o.unitId==50?480:(o.unitId==87 || o.unitId==117)?1500:2750;
            check(game.objectHitPoints(id)==hp && game.pendingFoundationForTesting(id)==o.pendingFoundation,"Original Fortress building health and deferred foundation");
            if(o.unitId==50)check(game.objectResourceAmount(id)==15,"Original Fortress Farm food");
            if(o.unitId==117)check((int(std::lround(game.objectFacing(id)/(6.283185307179586f/5)))%5+5)%5==o.facet,"Original Fortress wall adjacency survives import");
            const auto pose=game.objectAnimationForTesting(id);
            check(pose[0]==0 && pose[1]==0,"Original Fortress completion resets building animation");
        }
        if(c.seed==1) {
            auto bad=assets.dat();bad.civs[1].units[117].hitPoints=1501;rng=c.seed;
            check(!swgb::generateRmsMap(script,defines,120,120,bad,rng,map,&error,2,{1,1}) && rng==c.seed && map.terrain.empty(),"Fortress validates wall prototype before generation");
            check(!swgb::generateRmsMap(script,defines,120,120,assets.dat(),rng,map,&error,2,{1,2}) && rng==c.seed && map.terrain.empty(),"Unverified Fortress civilizations reject atomically");
        }
    }
    std::puts("PASS: actual Tiny Fortress54208, original world records and native import");
}
