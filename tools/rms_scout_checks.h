// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
static void rmsScoutChecks(swgb::Assets &assets) {
    const std::string grass=R"(<LAND_GENERATION>
base_terrain 0
create_player_lands { terrain_type 0 base_size 3 number_of_tiles 60 }
)";
    const std::string forest=R"(<LAND_GENERATION>
base_terrain 41
create_player_lands { terrain_type 9 base_size 3 number_of_tiles 60 }
<TERRAIN_GENERATION>
create_terrain 10 { base_terrain 41 number_of_tiles 37 number_of_clumps 4 spacing_to_other_terrain_types 1 set_avoid_player_start_areas }
create_terrain 6 { base_terrain 9 number_of_tiles 9 number_of_clumps 2 clumping_factor 6 }
)";
    const std::string objects=R"(<OBJECTS_GENERATION>
create_object 109 { set_place_for_every_player min_distance_to_players 0 max_distance_to_players 0 }
create_object 83 { set_place_for_every_player min_distance_to_players 6 max_distance_to_players 6 }
create_object 550 { set_place_for_every_player min_distance_to_players 7 max_distance_to_players 9 match_player_civ }
)";
    swgb::RmsScript scripts[2];std::string error;
    for(int profile=0;profile<2;++profile) {
        const auto source=(profile?forest:grass)+objects;
        check(scripts[profile].load(0,[&](int32_t id,std::vector<uint8_t>&bytes) {
            if(id)return false;bytes.assign(source.begin(),source.end());return true;
        },{},&error),error.c_str());
    }
    struct Scout { int unit;float x,y;int facet,frame;float delay; };
    const struct { uint32_t forest,players,civ1,civ2,seed,state,terrainHash,trees,sceneryHash;Scout scouts[2]; } cases[]={
        {0u,1u,1u,0u,1u,506035152u,2282508853u,0u,2166136261u,{{550,10.5f,1.5f,4,0,0.0f},{}}},
        {0u,1u,1u,0u,12345u,606195725u,1877077813u,0u,2166136261u,{{550,11.5f,18.5f,3,0,0.0f},{}}},
        {0u,1u,1u,0u,4294967295u,2522517941u,2491377461u,0u,2166136261u,{{550,13.5f,14.5f,1,0,0.0f},{}}},
        {0u,1u,2u,0u,1u,506035152u,470130693u,0u,2166136261u,{{552,10.5f,1.5f,4,7,0.25673383474349976f},{}}},
        {0u,1u,2u,0u,12345u,606195725u,461407749u,0u,2166136261u,{{552,11.5f,18.5f,3,3,0.0f},{}}},
        {0u,1u,2u,0u,4294967295u,2522517941u,3220284933u,0u,2166136261u,{{552,13.5f,14.5f,1,7,0.5543503761291504f},{}}},
        {0u,1u,3u,0u,1u,506035152u,2282508853u,0u,2166136261u,{{556,10.5f,1.5f,4,0,1.0916409492492676f},{}}},
        {0u,1u,3u,0u,12345u,606195725u,1877077813u,0u,2166136261u,{{556,11.5f,18.5f,3,0,0.22003234922885895f},{}}},
        {0u,1u,3u,0u,4294967295u,2522517941u,2491377461u,0u,2166136261u,{{556,13.5f,14.5f,1,0,1.2986785173416138f},{}}},
        {0u,1u,4u,0u,1u,2019156561u,2282508853u,0u,2166136261u,{{554,10.5f,1.5f,2,0,0.0f},{}}},
        {0u,1u,4u,0u,12345u,4153706386u,1877077813u,0u,2166136261u,{{554,11.5f,18.5f,0,0,0.0f},{}}},
        {0u,1u,4u,0u,4294967295u,2448817242u,2491377461u,0u,2166136261u,{{554,13.5f,14.5f,2,0,0.0f},{}}},
        {0u,1u,5u,0u,1u,506035152u,2282508853u,0u,2166136261u,{{258,10.5f,1.5f,4,0,1.0916409492492676f},{}}},
        {0u,1u,5u,0u,12345u,606195725u,1877077813u,0u,2166136261u,{{258,11.5f,18.5f,3,0,0.22003234922885895f},{}}},
        {0u,1u,5u,0u,4294967295u,2522517941u,2491377461u,0u,2166136261u,{{258,13.5f,14.5f,1,0,1.2986785173416138f},{}}},
        {0u,1u,6u,0u,1u,506035152u,2078377669u,0u,2166136261u,{{558,10.5f,1.5f,4,0,1.0916409492492676f},{}}},
        {0u,1u,6u,0u,12345u,606195725u,3067649733u,0u,2166136261u,{{558,11.5f,18.5f,3,0,0.22003234922885895f},{}}},
        {0u,1u,6u,0u,4294967295u,2522517941u,3863289541u,0u,2166136261u,{{558,13.5f,14.5f,1,0,1.2986785173416138f},{}}},
        {0u,1u,7u,0u,1u,506035152u,2282508853u,0u,2166136261u,{{927,10.5f,1.5f,4,0,0.0f},{}}},
        {0u,1u,7u,0u,12345u,606195725u,1877077813u,0u,2166136261u,{{927,11.5f,18.5f,3,0,0.0f},{}}},
        {0u,1u,7u,0u,4294967295u,2522517941u,2491377461u,0u,2166136261u,{{927,13.5f,14.5f,1,0,0.0f},{}}},
        {0u,1u,8u,0u,1u,506035152u,2282508853u,0u,2166136261u,{{932,10.5f,1.5f,4,0,1.0916409492492676f},{}}},
        {0u,1u,8u,0u,12345u,606195725u,1877077813u,0u,2166136261u,{{932,11.5f,18.5f,3,0,0.22003234922885895f},{}}},
        {0u,1u,8u,0u,4294967295u,2522517941u,2491377461u,0u,2166136261u,{{932,13.5f,14.5f,1,0,1.2986785173416138f},{}}},
        {0u,2u,1u,1u,1u,512005877u,2282508853u,0u,2166136261u,{{550,9.5f,9.5f,0,0,0.0f},{550,11.5f,27.5f,6,0,0.0f}}},
        {0u,2u,1u,1u,12345u,4028968488u,668798869u,0u,2166136261u,{{550,6.5f,11.5f,7,0,0.0f},{550,17.5f,2.5f,2,0,0.0f}}},
        {0u,2u,1u,1u,4294967295u,252401886u,2491377461u,0u,2166136261u,{{550,11.5f,8.5f,6,0,0.0f},{550,4.5f,28.5f,0,0,0.0f}}},
        {0u,2u,1u,2u,1u,512005877u,2282508853u,0u,2166136261u,{{550,9.5f,9.5f,0,0,0.0f},{552,11.5f,27.5f,6,7,0.3743765950202942f}}},
        {0u,2u,1u,2u,12345u,4028968488u,668798869u,0u,2166136261u,{{550,6.5f,11.5f,7,0,0.0f},{552,17.5f,2.5f,2,1,0.0f}}},
        {0u,2u,1u,2u,4294967295u,252401886u,2491377461u,0u,2166136261u,{{550,11.5f,8.5f,6,0,0.0f},{552,4.5f,28.5f,0,7,1.7568894624710083f}}},
        {0u,2u,1u,3u,1u,512005877u,2282508853u,0u,2166136261u,{{550,9.5f,9.5f,0,0,0.0f},{556,11.5f,27.5f,6,0,1.173479437828064f}}},
        {0u,2u,1u,3u,12345u,4028968488u,668798869u,0u,2166136261u,{{550,6.5f,11.5f,7,0,0.0f},{556,17.5f,2.5f,2,0,0.0f}}},
        {0u,2u,1u,3u,4294967295u,252401886u,2491377461u,0u,2166136261u,{{550,11.5f,8.5f,6,0,0.0f},{556,4.5f,28.5f,0,0,2.1352274417877197f}}},
        {0u,2u,1u,4u,1u,3198876314u,2282508853u,0u,2166136261u,{{550,9.5f,9.5f,0,0,0.0f},{554,11.5f,27.5f,2,0,0.0f}}},
        {0u,2u,1u,4u,12345u,2845241737u,668798869u,0u,2166136261u,{{550,6.5f,11.5f,7,0,0.0f},{554,17.5f,2.5f,7,0,0.0f}}},
        {0u,2u,1u,4u,4294967295u,1567749623u,2491377461u,0u,2166136261u,{{550,11.5f,8.5f,6,0,0.0f},{554,4.5f,28.5f,4,0,0.0f}}},
        {0u,2u,1u,5u,1u,512005877u,2282508853u,0u,2166136261u,{{550,9.5f,9.5f,0,0,0.0f},{258,11.5f,27.5f,6,0,1.173479437828064f}}},
        {0u,2u,1u,5u,12345u,4028968488u,668798869u,0u,2166136261u,{{550,6.5f,11.5f,7,0,0.0f},{258,17.5f,2.5f,2,0,0.0f}}},
        {0u,2u,1u,5u,4294967295u,252401886u,2491377461u,0u,2166136261u,{{550,11.5f,8.5f,6,0,0.0f},{258,4.5f,28.5f,0,0,2.1352274417877197f}}},
        {0u,2u,1u,6u,1u,512005877u,2282508853u,0u,2166136261u,{{550,9.5f,9.5f,0,0,0.0f},{558,11.5f,27.5f,6,0,1.173479437828064f}}},
        {0u,2u,1u,6u,12345u,4028968488u,668798869u,0u,2166136261u,{{550,6.5f,11.5f,7,0,0.0f},{558,17.5f,2.5f,2,0,0.0f}}},
        {0u,2u,1u,6u,4294967295u,252401886u,2491377461u,0u,2166136261u,{{550,11.5f,8.5f,6,0,0.0f},{558,4.5f,28.5f,0,0,2.1352274417877197f}}},
        {0u,2u,1u,7u,1u,512005877u,2282508853u,0u,2166136261u,{{550,9.5f,9.5f,0,0,0.0f},{927,11.5f,27.5f,6,0,0.0f}}},
        {0u,2u,1u,7u,12345u,4028968488u,668798869u,0u,2166136261u,{{550,6.5f,11.5f,7,0,0.0f},{927,17.5f,2.5f,2,0,0.0f}}},
        {0u,2u,1u,7u,4294967295u,252401886u,2491377461u,0u,2166136261u,{{550,11.5f,8.5f,6,0,0.0f},{927,4.5f,28.5f,0,0,0.0f}}},
        {0u,2u,1u,8u,1u,512005877u,2282508853u,0u,2166136261u,{{550,9.5f,9.5f,0,0,0.0f},{932,11.5f,27.5f,6,0,1.173479437828064f}}},
        {0u,2u,1u,8u,12345u,4028968488u,668798869u,0u,2166136261u,{{550,6.5f,11.5f,7,0,0.0f},{932,17.5f,2.5f,2,0,0.0f}}},
        {0u,2u,1u,8u,4294967295u,252401886u,2491377461u,0u,2166136261u,{{550,11.5f,8.5f,6,0,0.0f},{932,4.5f,28.5f,0,0,2.1352274417877197f}}},
        {1u,2u,1u,1u,1u,73585398u,2495521043u,37u,4201021415u,{{550,25.5f,7.5f,1,0,0.0f},{550,9.5f,13.5f,3,0,0.0f}}},
        {1u,2u,1u,1u,12345u,1534350773u,1986626585u,38u,2098472240u,{{550,1.5f,11.5f,0,0,0.0f},{550,27.5f,4.5f,6,0,0.0f}}},
        {1u,2u,1u,2u,1u,73585398u,2495521043u,37u,4201021415u,{{550,25.5f,7.5f,1,0,0.0f},{552,9.5f,13.5f,3,7,0.7740530967712402f}}},
        {1u,2u,1u,2u,12345u,1534350773u,1986626585u,38u,2098472240u,{{550,1.5f,11.5f,0,0,0.0f},{552,27.5f,4.5f,6,7,2.886568546295166f}}},
        {1u,2u,1u,4u,1u,1218713583u,2495521043u,37u,4201021415u,{{550,25.5f,7.5f,1,0,0.0f},{554,9.5f,13.5f,3,0,0.0f}}},
        {1u,2u,1u,4u,12345u,4261941338u,1986626585u,38u,2098472240u,{{550,1.5f,11.5f,0,0,0.0f},{554,27.5f,4.5f,6,0,0.0f}}},
    };
    swgb::Game game(assets);
    for(const auto &f:cases) {
        std::vector<uint16_t> civs{uint16_t(f.civ1)};
        if(f.players==2)civs.push_back(uint16_t(f.civ2));
        uint32_t state=f.seed;swgb::RmsMap map;
        check(swgb::generateRmsMap(scripts[f.forest],{},32,32,assets.dat(),state,map,&error,f.players,civs),error.c_str());
        check(state==f.state && map.ownedObjects.size()==5*f.players && map.scenery.size()==f.trees,
              "Original three-row scout startup RNG and object counts");
        uint32_t hash=2166136261u,sceneryHash=hash;
        for(auto t:map.terrain)hash=(hash^t)*16777619u;
        for(const auto &t:map.scenery) {
            check(t.alive,"Original scout startup retains generated trees");
            for(uint32_t v:{uint32_t(t.unitId),uint32_t(t.x*2),uint32_t(t.y*2),uint32_t(t.facet)})
                sceneryHash=(sceneryHash^v)*16777619u;
        }
        check(hash==f.terrainHash && sceneryHash==f.sceneryHash,"Original scout startup terrain and scenery order");
        for(uint32_t owner=1;owner<=f.players;++owner) {
            const auto &actual=map.ownedObjects[4*f.players+owner-1];const auto &e=f.scouts[owner-1];
            check(actual.owner==owner && actual.unitId==e.unit && actual.x==e.x && actual.y==e.y &&
                  actual.facet==e.facet && actual.animationFrame==e.frame && std::abs(actual.initialIdleDelay-e.delay)<.000001f,
                  "Original scout civilization, position, facing and initial sprite state");
        }
        state=f.seed;
        check(game.initRms(scripts[f.forest],{},32,state,&error,f.players,civs),error.c_str());
        check(state==f.state && game.activeObjectCount()==f.trees+5*f.players,"Scout native import count and RNG");
        for(uint32_t owner=1;owner<=f.players;++owner) {
            const auto &e=f.scouts[owner-1];const auto id=f.trees+4*f.players+owner;
            const auto pos=game.objectPosition(id);const auto animation=game.objectAnimationForTesting(id);
            check(game.objectUnitId(id)==e.unit && game.objectPlayer(id)==int(owner) && pos[0]==e.x && pos[1]==e.y &&
                  std::abs(animation[0]-e.frame*.2f)<.000001f && std::abs(animation[1]-e.delay)<.000001f,
                  "Scout import preserves original IDs, world positions and animation seconds");
            check(game.populationUsedForTesting(owner)==4 && game.populationCapacityForTesting(owner)==(civs[owner-1]==5?200:5) &&
                  game.resource(owner,11)==4 && game.resource(owner,37)==3 && game.resource(owner,40)==1 &&
                  game.resource(owner,98)==575 && game.resource(owner,164)==200 && game.resource(owner,165)==375,
                  "Scout startup population, military count and construction accounting");
        }
        if(!f.forest && f.players==1 && f.civ1==2 && f.seed==1) {
            const auto before=game.objectAnimationForTesting(5);swgb::InputState idle;
            game.updatePaused(.5f,idle);
            check(game.objectAnimationForTesting(5)==before,"Paused scout retains its original randomized idle hold");
            game.update(.05f,idle);game.update(.05f,idle);
            const auto after=game.objectAnimationForTesting(5);
            check(after[0]==7*.2f && std::abs(after[1]-(before[1]-.1f))<.000001f,
                  "Scout holds its original final frame using graphic duration in seconds");
            game.groupMoveForTesting({5},3,3);game.update(.05f,idle);
            check(game.objectAnimationForTesting(5)[1]==0,"Scout movement clears the initial idle hold");
        }
        if(!f.forest && f.players==1 && f.civ1==4)
            check(game.objectAnimationForTesting(5)[1]==0,"Sequence6 scout has no animated-sprite initial hold");
    }
    const auto valid=grass+objects;
    std::vector<std::string> invalids{valid+"create_object 550 { }",valid+"create_object 83 { }"};
    for(const auto &change:std::vector<std::pair<std::string,std::string>>{
            {"match_player_civ",""},{"min_distance_to_players 7","min_distance_to_players 6"},
            {"max_distance_to_players 9","max_distance_to_players 10"},
            {"create_object 550 {","create_object 550 { number_of_objects 2"}}) {
        auto source=valid;source.replace(source.find(change.first),change.first.size(),change.second);invalids.push_back(source);
    }
    for(const auto &source:invalids) {
        swgb::RmsScript invalid;
        check(invalid.load(0,[&](int32_t id,std::vector<uint8_t>&bytes) {
            if(id)return false;bytes.assign(source.begin(),source.end());return true;
        },{},&error),error.c_str());
        uint32_t state=12345;swgb::RmsMap map;
        check(!swgb::generateRmsMap(invalid,{},32,32,assets.dat(),state,map,&error,1,{1}) &&
              state==12345 && map.terrain.empty() && map.ownedObjects.empty(),
              "Unsupported or extra scout rows reject atomically");
    }
    uint32_t state=12345;swgb::RmsMap map;
    check(!swgb::generateRmsMap(scripts[0],{},32,32,assets.dat(),state,map,&error,3,{1,2,3}) && state==12345,
          "Unverified scout owner counts remain rejected");
    std::puts("PASS: original three-row scout startup across all civilizations and forest contexts");
}
