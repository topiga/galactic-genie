// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstring>
#include "../src/engine/rms_walls.h"
// Actual derived4e7420/4df9b0, raw54208/54223 Tiny/default settings; local tiny-derived findings, 2026-11-16.
static void rmsWalledStartupChecks(swgb::Assets &assets,swgb::ResourceSet &resources) {
    swgb::RmsScript script;std::string error;
    const std::unordered_set<std::string> defines{"TINY_MAP","FIXED_POSITIONS","DEFAULT_RESOURCES"};
    struct Case {uint32_t seed,count,rng,objects,terrain,heights,slopes,cost1,cost2,ownedHeightHealth,obstruction,directions,population1=21,population2=21,resource=54208,gaiaCost=450;};
    const Case cases[]={
        {1u,1852u,472822087u,2066791139u,695987317u,370085139u,2196496358u,2200u,2160u,1319479453u,2795300037u,3497619881u},
        {12345u,1808u,3283216374u,733826055u,917820985u,2894856452u,830763451u,2085u,2160u,337729919u,3530970053u,85941139u},
        {16160u,1857u,1539544777u,3803719893u,1933540263u,4051794037u,3701867154u,2015u,2140u,551165221u,2355176645u,93798073u,6u,21u},
        {8000u,1829u,2585261580u,716729200u,686078103u,1708483483u,2844762376u,2010u,2185u,328983822u,1656264133u,956334648u,16u,21u},
        {4000u,1844u,553662306u,1287185804u,2053428819u,92760354u,2443288221u,2150u,2195u,2660093171u,2244109253u,2309856651u,21u,21u},
        {7000u,1910u,2138941861u,3662151171u,957272514u,227356200u,306823974u,2305u,2230u,3989418064u,1627561669u,4236902406u,21u,21u},
        {0u,1866u,3055721841u,2929698890u,2901788339u,2380709378u,1316939534u,2080u,2225u,2294080446u,2015201477u,2725680523u,21u,21u},
        {13u,1804u,1253119621u,1403536778u,136691793u,4149177852u,945275628u,2120u,2080u,1439175937u,2691342533u,265710714u,21u,21u},
        {10u,1889u,527864596u,1504477423u,1930828339u,3923568668u,4230527957u,2080u,2200u,3232139797u,3159405765u,4067783459u,21u,16u},
        {1u,7009u,1629495980u,4285465516u,1621593130u,1017236165u,1017236165u,930u,1100u,3174229881u,2034302149u,3096632578u,1u,1u,54223u,1500u},
        {4000u,8256u,2591708538u,2713779736u,2229548251u,1017236165u,1017236165u,940u,1000u,838673138u,594823877u,2818966321u,1u,1u,54223u,1300u},
        {16160u,6911u,3017032858u,2080249368u,1044171819u,1017236165u,1017236165u,920u,990u,3211155313u,634942661u,3472657141u,1u,1u,54223u,1550u},
    };
    const auto bits=[](float value){uint32_t out;std::memcpy(&out,&value,sizeof out);return out;};
    const auto hash=[](const auto &values){uint32_t out=2166136261u;for(auto v:values)out=(out^uint32_t(v))*16777619u;return out;};
    const auto gridHash=[&](const auto &values) {
        if(!values.empty())return hash(values);
        uint32_t out=2166136261u;for(int n=0;n<14400;++n)out*=16777619u;return out;
    };
    for(const auto &c:cases) {
        check(script.load(int32_t(c.resource),resources,&error),error.c_str());
        const bool arena=c.resource==54223;
        swgb::RmsMap map;uint32_t rng=c.seed;
        check(swgb::generateRmsMap(script,defines,120,120,assets.dat(),rng,map,&error,2,{1,1}),error.c_str());
        check(rng==c.rng && hash(map.terrain)==c.terrain && gridHash(map.elevation)==c.heights && gridHash(map.slopes)==c.slopes,
              "Original derived-reader walled-map terrain, height, slope and final RNG");
        check(map.objects.empty(),"Walled map has no pre-owned object rows");
        uint32_t next=0,count=0;std::vector<uint32_t> records;
        auto record=[&](uint32_t id,int owner,int unit,int facet,float x,float y) {
            records.insert(records.end(),{id,uint32_t(owner),uint32_t(unit),uint32_t(facet),bits(x),bits(y)});++count;
        };
        for(const auto &o:map.cliffs){record(o.creationOrdinal,0,o.unit,o.facet,o.x,o.y);next=o.creationOrdinal+1;}
        for(const auto &o:map.scenery){if(o.alive)record(next,0,o.unitId,o.facet,o.x,o.y);++next;}
        for(const auto &o:map.ownedObjects)record(next++,o.owner,o.unitId,o.facet,o.x,o.y);
        const auto resourceBase=next+1;
        for(const auto &o:map.resourcesAfterOwned)record(next++,o.owner,o.unitId,o.facet,o.x,o.y);
        check(count==c.count && hash(records)==c.objects,"Original complete walled-map object order, IDs, owners, units, positions and facets");
        swgb::RmsWallInput wallInput;wallInput.width=wallInput.height=120;wallInput.terrain=map.terrain;
        wallInput.minimumDistance=arena?18:16;wallInput.maximumDistance=arena?22:20;
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
        check(hash(boundary.obstruction)==c.obstruction && hash(boundary.directions)==c.directions,"Original walled-map obstruction and retained direction cache, including untouched water cells");
        swgb::Game game(assets);rng=c.seed;
        check(game.initRms(script,defines,120,rng,&error,2,{1,1}),error.c_str());
        check(rng==c.rng && game.activeObjectCount()==c.count,"Walled-map import retains original RNG and object count");
        std::vector<uint32_t> ownedHeightHealth;
        for(size_t n=0;n<records.size();n+=6)if(records[n+1]) {
            const auto id=records[n]+1;const auto pos=game.objectPosition(id);
            ownedHeightHealth.insert(ownedHeightHealth.end(),{records[n],bits(game.elevationForTesting(pos[0],pos[1])),bits(game.objectHitPoints(id))});
        }
        check(hash(ownedHeightHealth)==c.ownedHeightHealth,"Original walled-map owned object heights on slopes and HP");
        check(game.resource(0,98)==c.gaiaCost && game.resource(0,164)==c.gaiaCost && game.resource(0,165)==0 && game.resource(0,11)==5,"Original walled-map Gaia counters");
        for(int owner:{1,2}) {
            const auto cost=owner==1?c.cost1:c.cost2;
            check(game.resource(owner,98)==cost && game.resource(owner,165)==cost-200 && game.resource(owner,164)==200 &&
                  game.resource(owner,4)==(owner==1?c.population1:c.population2) && game.resource(owner,11)==4 && game.resource(owner,21)==(arena?2:3),"Original walled-map building costs and population");
            check(game.populationUsedForTesting(owner)==4 && game.populationCapacityForTesting(owner)==4+(owner==1?c.population1:c.population2),
                  "Original walled-map HUD population includes only completed Prefabs");
            check(game.technologyOrderForTesting(owner)==(arena?std::vector<int>{0,486,22,451,10}:std::vector<int>{0,486,22,451,11,10,16,31,85,209,20}),"Original walled-map completion research order");
        }
        for(size_t n=0;n<map.resourcesAfterOwned.size();++n) {
            const auto &o=map.resourcesAfterOwned[n];const uint32_t id=resourceBase+uint32_t(n);const auto pos=game.objectPosition(id);
            check(game.objectActive(id) && game.objectUnitId(id)==o.unitId && game.objectPlayer(id)==o.owner &&
                  std::abs(pos[0]-o.y)<1e-5f && std::abs(pos[1]-(120-o.x))<1e-5f,"Walled-map native import preserves every trailing object");
            if(!o.owner)continue;
            const int hp=o.unitId==79?1250:o.unitId==70?800:o.unitId==50?480:(o.unitId==87 || o.unitId==117)?1500:2750;
            check(game.objectHitPoints(id)==hp && game.pendingFoundationForTesting(id)==o.pendingFoundation,"Original walled-map building health and deferred foundation");
            if(o.unitId==50)check(game.objectResourceAmount(id)==15,"Original walled-map Farm food");
            if(o.unitId==117)check((int(std::lround(game.objectFacing(id)/(6.283185307179586f/5)))%5+5)%5==o.facet,"Original walled-map wall adjacency survives import");
            const auto pose=game.objectAnimationForTesting(id);
            check(pose[0]==0 && pose[1]==0,"Original walled-map completion resets building animation");
        }
        if(c.seed==1 && !arena) {
            auto bad=assets.dat();bad.civs[1].units[117].hitPoints=1501;rng=c.seed;
            check(!swgb::generateRmsMap(script,defines,120,120,bad,rng,map,&error,2,{1,1}) && rng==c.seed && map.terrain.empty(),"Fortress validates wall prototype before generation");
            check(!swgb::generateRmsMap(script,defines,120,120,assets.dat(),rng,map,&error,2,{1,2}) && rng==c.seed && map.terrain.empty(),"Unverified Fortress civilizations reject atomically");
        }
    }
    std::puts("PASS: actual Tiny Fortress54208/Arena54223, original world records and native import");
}
