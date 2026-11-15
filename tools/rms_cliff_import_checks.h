// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
static void checkRmsCliffImport(swgb::Assets &assets) {
    using namespace swgb;
    const std::string source="<LAND_GENERATION> base_terrain 6 create_land { terrain_type 6 land_position 50 50 base_size 0 number_of_tiles 0 } <CLIFF_GENERATION>";
    RmsScript script; RmsMap map; std::string error;
    check(script.load(0,[&](int32_t id,std::vector<uint8_t> &out) {
        if(id) return false;out.assign(source.begin(),source.end());return true;
    },{},&error),error.c_str());
    uint32_t rng=1;
    check(generateRmsMap(script,{},120,120,assets.dat(),rng,map,&error),error.c_str());
    Game game(assets);rng=1;
    check(game.initRms(script,{},120,rng,&error),error.c_str());
    check(rng==1150416911u && game.activeObjectCount()==map.cliffs.size(),"original cliff native import count/RNG");
    for(const auto &o:map.cliffs) {
        const uint32_t id=o.creationOrdinal+1;
        const auto p=game.objectPosition(id);
        check(game.objectUnitId(id)==o.unit && game.objectPlayer(id)==0 && p[0]==o.x && p[1]==o.y,"native cliff original IDs/types/positions");
        const float pi=3.14159265358979323846f,theta=pi*.5f+o.facet*(2*pi/25);
        const float sx=std::cos(theta)*.5f,sy=std::sin(theta);
        const float expected=std::atan2(sy-sx,sy+sx),actual=game.objectFacing(id);
        check(std::abs(std::sin(actual)-std::sin(expected))<.00001f &&
              std::abs(std::cos(actual)-std::cos(expected))<.00001f,"native cliff original facet projection");
    }
    struct RaisedCase { int levels; uint32_t rng,heightHash; size_t count; };
    const RaisedCase cases[]={{1,2269132341u,2088011581u,47},{7,2215531909u,4157776466u,33}};
    for(const auto &c:cases) {
        const std::string raised=source+" <ELEVATION_GENERATION> create_elevation "+std::to_string(c.levels)+
            " { base_terrain 6 number_of_tiles 8000 number_of_clumps 30 }";
        check(script.load(0,[&](int32_t id,std::vector<uint8_t> &out) {
            if(id) return false;out.assign(raised.begin(),raised.end());return true;
        },{},&error),error.c_str());
        rng=1;check(generateRmsMap(script,{},120,120,assets.dat(),rng,map,&error),error.c_str());
        rng=1;check(game.initRms(script,{},120,rng,&error),error.c_str());
        check(rng==c.rng && game.activeObjectCount()==c.count,"elevated cliff native import count/RNG");
        uint32_t heightHash=2166136261u;
        for(const auto &o:map.cliffs) {
            const auto p=game.objectPosition(o.creationOrdinal+1);
            check(p[0]==o.x && p[1]==o.y,"elevated cliff native import positions");
            const float height=game.elevationForTesting(p[0],p[1]);
            check(std::isfinite(height) && height>=0 && height<=c.levels && height==std::floor(height),
                "original cliff creation height is an exact level");
            heightHash=(heightHash^(o.creationOrdinal+1))*16777619u;
            heightHash=(heightHash^uint32_t(height*256))*16777619u;
        }
        check(heightHash==c.heightHash,"native cliff rendering heights match original object z values");
    }
    const int forestTypes[][3]={{21,413,12},{18,348,12},{20,349,11}};
    for(const auto &spec:forestTypes) {
        const std::string forest=source+" <TERRAIN_GENERATION> create_terrain "+std::to_string(spec[0])+" { base_terrain 6 number_of_tiles 2000 number_of_clumps 20 spacing_to_other_terrain_types 2 clumping_factor 20 }";
        check(script.load(0,[&](int32_t id,std::vector<uint8_t> &out) {
            if(id) return false;out.assign(forest.begin(),forest.end());return true;
        },{},&error),error.c_str());
        rng=1;check(generateRmsMap(script,{},120,120,assets.dat(),rng,map,&error),error.c_str());
        rng=1;check(game.initRms(script,{},120,rng,&error),error.c_str());
        check(rng==1567879473u && game.activeObjectCount()==2065,"cliff/forest native import count/RNG");
        uint32_t nextId=map.cliffs.back().creationOrdinal+2;
        for(const auto &tree:map.scenery) {
            const auto p=game.objectPosition(nextId);
            check(game.objectUnitId(nextId)==spec[1] && game.objectPlayer(nextId)==0 && p[0]==tree.x && p[1]==tree.y,
                "native trees preserve original IDs after cliff replacements and rejected overlaps");
            const float pi=3.14159265358979323846f,theta=pi*.5f+tree.facet*(2*pi/spec[2]);
            const float sx=std::cos(theta)*.5f,sy=std::sin(theta),expected=std::atan2(sy-sx,sy+sx);
            const float actual=game.objectFacing(nextId);
            check(std::abs(std::sin(actual)-std::sin(expected))<.00001f &&
                std::abs(std::cos(actual)-std::cos(expected))<.00001f,"native forest import preserves original11/12-angle facets");
            ++nextId;
        }
    }
    for(const auto &spec:forestTypes) for(bool avoid:{false,true}) for(bool fixed:{false,true}) {
        if(fixed && spec[0]!=21) continue;
        const std::string players="<LAND_GENERATION> base_terrain 6 create_player_lands { terrain_type 6 base_size 3 number_of_tiles 1000 } "
            "<CLIFF_GENERATION> <TERRAIN_GENERATION> create_terrain "+std::to_string(spec[0])+" { base_terrain 6 number_of_tiles 2000 number_of_clumps 20 spacing_to_other_terrain_types 2 clumping_factor 20 "+
            std::string(avoid?"set_avoid_player_start_areas ":"")+"}";
        check(script.load(0,[&](int32_t id,std::vector<uint8_t> &out) {
            if(id) return false;out.assign(players.begin(),players.end());return true;
        },{},&error),error.c_str());
        const std::unordered_set<std::string> defines=fixed?std::unordered_set<std::string>{"FIXED_POSITIONS"}:std::unordered_set<std::string>{};
        rng=1;check(generateRmsMap(script,defines,120,120,assets.dat(),rng,map,&error),error.c_str());
        rng=1;check(game.initRms(script,defines,120,rng,&error),error.c_str());
        check(rng==(fixed?(avoid?3019377153u:685587785u):(avoid?2281347096u:405893282u)) &&
            game.activeObjectCount()==(fixed?(avoid?2048u:2069u):(avoid?2055u:2053u)),
            "native player-land forest and cliff import count/RNG");
        uint32_t nextId=0;
        for(const auto &o:map.cliffs) {
            nextId=o.creationOrdinal+1;
            const auto p=game.objectPosition(nextId);
            check(game.objectUnitId(nextId)==o.unit && p[0]==o.x && p[1]==o.y,"native player-land cliffs retain original replacement IDs");
        }
        ++nextId;
        for(const auto &tree:map.scenery) {
            const auto p=game.objectPosition(nextId);
            check(game.objectUnitId(nextId)==spec[1] && game.objectPlayer(nextId)==0 && p[0]==tree.x && p[1]==tree.y,
                "native player-land forest retains original tree IDs and positions");
            ++nextId;
        }
    }

    const std::string ownedSource=R"(/* Authored two-player startup diagnostic; no resource rows beyond forest. */
<LAND_GENERATION>
base_terrain 6
create_player_lands {
  terrain_type 6
  base_size 13
  number_of_tiles 7200
}
<CLIFF_GENERATION>
<TERRAIN_GENERATION>
create_terrain 21 {
  base_terrain 6
  number_of_tiles 6000
  number_of_clumps 60
  spacing_to_other_terrain_types 2
  clumping_factor 20
  set_avoid_player_start_areas
}
<OBJECTS_GENERATION>
create_object 109 {
  set_place_for_every_player
  number_of_objects 1
  min_distance_to_players 0
  max_distance_to_players 0
  group_placement_radius 18
}
create_object 83 {
  set_place_for_every_player
  number_of_objects 1
  min_distance_to_players 6
  max_distance_to_players 6
  group_placement_radius 3
}
create_object 550 {
  set_place_for_every_player
  match_player_civ
  number_of_objects 1
  min_distance_to_players 7
  max_distance_to_players 9
  group_placement_radius 3
}
)";
    check(script.load(0,[&](int32_t id,std::vector<uint8_t> &out) {
        if(id) return false;out.assign(ownedSource.begin(),ownedSource.end());return true;
    },{},&error),error.c_str());
    struct OwnedPose { int pending,frame; float delay; };
    struct OwnedCase { uint32_t seed,rng,objects,terrain; size_t count; uint32_t last; OwnedPose poses[10]; };
    const OwnedCase ownedCases[]={
        {1u,2896867181u,3285359612u,568551394u,6045,6065,{{255,0,0.000000000f},{27,0,0.000000000f},{255,2,0.000000000f},{255,7,1.470534325f},{255,2,0.000000000f},{255,0,0.000000000f},{255,0,0.000000000f},{255,7,0.000000000f},{255,0,0.000000000f},{255,0,0.000000000f}}},
        {12345u,2097857618u,3449277198u,4028300437u,6023,6057,{{255,0,0.000000000f},{27,0,0.000000000f},{255,1,0.000000000f},{255,0,0.000000000f},{255,3,0.000000000f},{255,6,0.000000000f},{255,4,0.000000000f},{255,7,2.415539980f},{255,0,0.000000000f},{255,0,0.000000000f}}},
        {4294967295u,2375127600u,1812976232u,14822313u,6095,6130,{{255,0,0.000000000f},{27,0,0.000000000f},{255,7,1.325174809f},{255,7,1.086825132f},{255,7,1.362102032f},{255,7,2.867732763f},{255,6,0.000000000f},{255,2,0.000000000f},{255,0,0.000000000f},{255,0,0.000000000f}}},
    };
    for(const auto &c:ownedCases) {
        rng=c.seed;check(generateRmsMap(script,{},120,120,assets.dat(),rng,map,&error,2,{1,1}),error.c_str());
        check(map.ownedObjects.size()==10 && map.objects.empty() && map.resourcesAfterOwned.empty(),"cliff startup has original ten owned units");
        uint32_t hash=2166136261u,terrainHash=2166136261u,nextId=0;
        auto mix=[&](uint32_t id,int owner,int unit,int facet,float x,float y) {
            for(uint32_t v:{id,uint32_t(owner),uint32_t(unit),uint32_t(facet),uint32_t(x*2),uint32_t(y*2)}) hash=(hash^v)*16777619u;
        };
        for(const auto &o:map.cliffs) {mix(o.creationOrdinal,0,o.unit,o.facet,o.x,o.y);nextId=o.creationOrdinal+1;}
        for(const auto &o:map.scenery) {check(o.alive,"cliff startup retains original live trees");mix(nextId++,0,o.unitId,o.facet,o.x,o.y);}
        const uint32_t ownedBase=nextId;
        for(const auto &o:map.ownedObjects) mix(nextId++,o.owner,o.unitId,o.facet,o.x,o.y);
        for(auto tile:map.terrain) terrainHash=(terrainHash^tile)*16777619u;
        check(rng==c.rng && hash==c.objects && terrainHash==c.terrain && nextId==c.last+1 &&
            map.cliffs.size()+map.scenery.size()+map.ownedObjects.size()==c.count,
            "original cliff/forest/owned startup records, foundations, allocation gaps and RNG");
        rng=c.seed;check(game.initRms(script,{},120,rng,&error,2,{1,1}),error.c_str());
        check(rng==c.rng && game.activeObjectCount()==c.count,"native owned cliff startup count/RNG");
        for(size_t i=0;i<map.ownedObjects.size();++i) {
            const auto &o=map.ownedObjects[i];const auto &pose=c.poses[i];const uint32_t id=ownedBase+uint32_t(i)+1;
            check(o.pendingFoundation==pose.pending && o.animationFrame==pose.frame &&
                std::abs(o.initialIdleDelay-pose.delay)<.00001f,"original owned cliff startup foundation and animation phase");
            const auto position=game.objectPosition(id),animation=game.objectAnimationForTesting(id);
            const auto &unit=assets.dat().civs[o.civilization].units[o.unitId];const auto *graphic=assets.dat().graphic(unit.standingGraphic[0]);
            check(game.objectUnitId(id)==o.unitId && game.objectPlayer(id)==o.owner && position[0]==o.x && position[1]==o.y &&
                game.objectHitPoints(id)==(o.unitId==109?2500.f:o.unitId==550?60.f:40.f) &&
                game.pendingFoundationForTesting(id)==pose.pending && animation[0]==pose.frame*graphic->frameDuration &&
                std::abs(animation[1]-pose.delay)<.00001f,"native original starting IDs, positions, health and pose");
        }
        const int resourceIds[]={11,37,40,98,164,165};
        const float resources[]={4,3,1,575,200,375};
        for(int owner:{1,2}) {
            check(game.populationCapacityForTesting(owner)-game.populationUsedForTesting(owner)==1 &&
                game.resource(owner,21)==2,"original cliff startup free population and completed research count");
            for(size_t i=0;i<6;++i)
                check(game.resource(owner,resourceIds[i])==resources[i],"original cliff startup population and cost bookkeeping");
        }
        check(game.spawnObjectForTesting(1,83,1,2.5f,2.5f)==c.last+2,"native next allocation retains original cliff replacement gaps");
    }
    const auto retainedCount=game.activeObjectCount();
    for(const auto &civs:std::vector<std::vector<uint16_t>>{{1,2},{2,1},{}}) {
        rng=1;
        check(!game.initRms(script,{},120,rng,&error,2,civs) && rng==1 && game.activeObjectCount()==retainedCount,
            "unverified owned cliff civilizations preserve the current world");
    }
    rng=1;
    check(!game.initRms(script,{"FIXED_POSITIONS"},120,rng,&error,2,{1,1}) && rng==1 && game.activeObjectCount()==retainedCount,
        "unverified owned cliff fixed positions preserve the current world");

}
