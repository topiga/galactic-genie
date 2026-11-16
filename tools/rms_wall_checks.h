// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "../src/engine/rms_walls.h"
// Actual499a60 stages; local rms-wall117-native-prototype findings, 2026-11-16.
static void ownedRmsWallBoundaryChecks(swgb::Assets &assets) {
    const std::string source=R"(#const RMS_6 6
#const RMS_21 21
#const RMS_109 109
#const RMS_83 83
#const RMS_550 550
#const RMS_87 87
<LAND_GENERATION> base_terrain RMS_6 create_player_lands { terrain_type RMS_6 base_size 13 number_of_tiles 7200 } <CLIFF_GENERATION> <TERRAIN_GENERATION> create_terrain RMS_21 { base_terrain RMS_6 number_of_tiles 6000 number_of_clumps 60 spacing_to_other_terrain_types 2 clumping_factor 20 set_avoid_player_start_areas } <OBJECTS_GENERATION> create_object RMS_109 { set_place_for_every_player number_of_objects 1 min_distance_to_players 0 max_distance_to_players 0 group_placement_radius 18 } create_object RMS_83 { set_place_for_every_player number_of_objects 1 min_distance_to_players 6 max_distance_to_players 6 group_placement_radius 3 } create_object RMS_550 { set_place_for_every_player match_player_civ number_of_objects 1 min_distance_to_players 7 max_distance_to_players 9 group_placement_radius 3 } )";
    struct Case { uint32_t seed,terrain; std::array<uint32_t,16> stages; };
    const Case cases[]={
        {1u, 742396127u, {3576039877u, 3664260213u, 3934188782u, 1644581822u, 680468115u, 3499084747u, 699229451u, 3423741803u, 1934467781u, 1842582091u, 4034547226u, 1198949132u, 1215750800u, 2041820795u, 806659457u, 344334813u}},
        {12345u, 2544610571u, {928842437u, 2400590430u, 2019525533u, 3663342404u, 2109202993u, 1277640200u, 3102986594u, 1621929952u, 1621929952u, 1969232334u, 131935967u, 3312367949u, 144266538u, 468505685u, 314624542u, 1240707077u}},
        {16160u, 3787531182u, {3332784581u, 1485044019u, 1060473042u, 4080570182u, 1714203709u, 3664350798u, 1212449678u, 426989960u, 1473983288u, 761724018u, 1376899582u, 990901592u, 86664909u, 2656006818u, 1831916887u, 1115432037u}},
    };
    const auto hash=[](const auto &values){uint32_t h=2166136261u;for(auto value:values)h=(h^uint32_t(value))*16777619u;return h;};
    struct ImportCase { uint32_t seed,count,added,rng,objects; int walls1,walls2; };
    const ImportCase imports[]={
        {1u,6095u,108u,3334285395u,1440103112u,46,41},
        {12345u,6076u,105u,222964401u,231367775u,58,29},
        {16160u,6161u,98u,3621490841u,2306590884u,56,27},
    };
    for(const auto &c:cases) {
        swgb::RmsScript script;std::string error;
        check(script.load(0,[&](int32_t id,std::vector<uint8_t> &bytes){if(id)return false;bytes.assign(source.begin(),source.end());return true;},{},&error),error.c_str());
        swgb::RmsMap map;uint32_t rng=c.seed;
        check(swgb::generateRmsMap(script,{"FIXED_POSITIONS"},120,120,assets.dat(),rng,map,&error,2,{1,1}),error.c_str());
        check(hash(map.terrain)==c.terrain,"Original wall-entry terrain from native startup");
        swgb::RmsWallInput input;input.width=input.height=120;input.terrain=map.terrain;
        input.restrictions=assets.dat().terrainRestrictions[25].passableBuildableDmgMultiplier;
        auto append=[&](int unit,int owner,float x,float y) {
            const auto &u=assets.dat().civs[owner?1:0].units[unit];
            const float rx=u.collisionSize[assets.dat().footprintAxesSwapped?1:0],ry=u.collisionSize[assets.dat().footprintAxesSwapped?0:1];
            input.objects.push_back({owner,x,y,rx,ry,u.obstructionType,u.obstructionClass,u.cls});
            if(unit==109)input.commandCenters.push_back({owner,x,y,rx,ry});
        };
        for(const auto &o:map.cliffs)append(o.unit,0,o.x,o.y);
        for(const auto &o:map.scenery)if(o.alive)append(o.unitId,0,o.x,o.y);
        for(const auto &o:map.ownedObjects)append(o.unitId,o.owner,o.x,o.y);
        check(map.resourcesAfterOwned.empty(),"Wall boundary fixture precedes extra owned rows");
        swgb::RmsWallBoundary result;
        check(swgb::generateRmsWallBoundary(input,result,&error),error.c_str());
        std::vector<uint32_t> actual{hash(result.obstruction),hash(result.directions)};
        check(result.routeBoundaries.size()==8,"Original four boundary routes per owner");
        for(const auto &grid:result.routeBoundaries)actual.push_back(hash(grid));
        for(const auto *grid:{&result.reachableScratch,&result.reachableBoundary,&result.prunedScratch,&result.prunedBoundary,&result.segmentScratch})actual.push_back(hash(*grid));
        std::vector<int> segments;
        for(const auto &owner:result.segments)for(const auto &v:owner)
            segments.insert(segments.end(),{v.owner,int(v.active),v.fromX,v.fromY,v.toX,v.toY,v.length});
        actual.push_back(hash(segments));
        check(actual.size()==c.stages.size() && std::equal(actual.begin(),actual.end(),c.stages.begin()),"Original wall obstruction, routes, reachability, pruning and ordered segments");
        if(c.seed==1) {
            const auto original=hash(result.obstruction);auto invalid=input;
            invalid.objects[0].obstructionType=10;
            check(!swgb::generateRmsWallBoundary(invalid,result,&error) && !error.empty() && hash(result.obstruction)==original,"Unverified wall obstruction rejects atomically");
            invalid=input;invalid.objects[0].unitClass=29;
            check(!swgb::generateRmsWallBoundary(invalid,result,&error) && hash(result.obstruction)==original,"Unverified special wall class rejects atomically");
            invalid=input;invalid.minimumDistance=18;
            check(!swgb::generateRmsWallBoundary(invalid,result,&error) && hash(result.obstruction)==original,"Unverified mixed wall-distance pair rejects atomically");
            invalid=input;invalid.terrain.pop_back();
            check(!swgb::generateRmsWallBoundary(invalid,result,&error) && hash(result.obstruction)==original,"Malformed wall terrain rejects atomically");
        }
        const auto &expected=*std::find_if(std::begin(imports),std::end(imports),[&](const auto &v){return v.seed==c.seed;});
        const std::string wallSource="#const RMS_117 117\n"+source+"create_object RMS_117 { set_place_for_every_player number_of_objects 9999 min_distance_to_players 16 max_distance_to_players 20 }";
        swgb::RmsScript wallScript;
        check(wallScript.load(0,[&](int32_t id,std::vector<uint8_t> &bytes){if(id)return false;bytes.assign(wallSource.begin(),wallSource.end());return true;},{},&error),error.c_str());
        swgb::RmsMap walls;rng=c.seed;
        check(swgb::generateRmsMap(wallScript,{"FIXED_POSITIONS"},120,120,assets.dat(),rng,walls,&error,2,{1,1}),error.c_str());
        std::vector<uint32_t> records;
        for(const auto &o:walls.resourcesAfterOwned)
            records.insert(records.end(),{uint32_t(o.unitId),o.owner,uint32_t(o.x*2),uint32_t(o.y*2),o.facet,o.annexParentIndex});
        check(rng==expected.rng && walls.resourcesAfterOwned.size()==expected.added && hash(records)==expected.objects,
              "Original ordered walls, gates, explicit posts, facets, parent indices and RNG");
        check(walls.terrain==map.terrain && walls.elevation==map.elevation && walls.slopes==map.slopes,"Walls retain original terrain grids");
        swgb::Game game(assets);rng=c.seed;
        check(game.initRms(wallScript,{"FIXED_POSITIONS"},120,rng,&error,2,{1,1}),error.c_str());
        check(rng==expected.rng && game.activeObjectCount()==expected.count,"Wall import does not duplicate annexes or object IDs");
        for(int owner:{1,2}) {
            const int cost=5*(owner==1?expected.walls1:expected.walls2);
            check(game.resource(owner,98)==575+cost && game.resource(owner,165)==375+cost && game.resource(owner,164)==200 &&
                  game.resource(owner,4)==1 && game.resource(owner,11)==4 && game.resource(owner,21)==2,"Original wall cost and population counters");
            check(game.technologyOrderForTesting(owner)==std::vector<int>({0,486,22,451,10}),"Original wall research without gate-head tech17");
        }
        uint32_t first=0;
        for(const auto &o:walls.cliffs)first=o.creationOrdinal+1;
        first+=uint32_t(walls.scenery.size()+walls.objects.size()+walls.ownedObjects.size())+1;
        for(size_t n=0;n<walls.resourcesAfterOwned.size();++n) {
            const auto &o=walls.resourcesAfterOwned[n];const uint32_t id=first+uint32_t(n);const auto pos=game.objectPosition(id);
            check(game.objectUnitId(id)==o.unitId && game.objectPlayer(id)==o.owner && pos[0]==o.y && pos[1]==120-o.x &&
                  game.objectHitPoints(id)==(o.unitId==117?1500:2750) && game.pendingFoundationForTesting(id)==255,
                  "Original imported wall/gate identity, position, HP and foundation");
            if(o.unitId==117)check((int(std::lround(game.objectFacing(id)/(6.283185307179586f/5)))%5+5)%5==o.facet,"Original wall adjacency facet survives native import");
            const auto animation=game.objectAnimationForTesting(id);
            check(animation[0]==0 && animation[1]==0,"Original wall/gate completion resets animation");
        }
        const auto gate=std::find_if(walls.resourcesAfterOwned.begin(),walls.resourcesAfterOwned.end(),[](const auto &o){return o.unitId==64 || o.unitId==88;});
        check(gate!=walls.resourcesAfterOwned.end(),"Fixture contains a gate assembly");
        const uint32_t gateId=first+uint32_t(gate-walls.resourcesAfterOwned.begin());
        check(game.damageObjectForTesting(gateId,3000) && !game.objectActive(gateId) && !game.objectActive(gateId+1) && !game.objectActive(gateId+2),
              "Explicit RMS post parents retain native gate destruction behavior");
        if(c.seed==1) {
            auto bad=assets.dat();bad.civs[1].units[81].resourceStorages[0].type=4;bad.civs[1].units[81].resourceStorages[0].amount=1;rng=c.seed;
            check(!swgb::generateRmsMap(wallScript,{"FIXED_POSITIONS"},120,120,bad,rng,walls,&error,2,{1,1}) && rng==c.seed && walls.terrain.empty(),
                  "Unsupported gate-post completion rejects before generation");
        }
    }
    std::puts("PASS: original wall routes, emission, explicit annex import, facets and completion");
}
