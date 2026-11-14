// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
static void rmsWorkerSpawnChecks(swgb::Assets &assets) {
    const std::string source=R"(<LAND_GENERATION> base_terrain 0
        create_player_lands { terrain_type 0 base_size 3 number_of_tiles 60 }
        <OBJECTS_GENERATION>
        create_object 109 { number_of_objects 1 set_place_for_every_player min_distance_to_players 0 max_distance_to_players 0 }
        create_object 83 { number_of_objects 1 set_place_for_every_player min_distance_to_players 1 max_distance_to_players 3 })";
    swgb::RmsScript script;std::string error;
    check(script.load(0,[&](int32_t id,std::vector<uint8_t> &bytes) {
        if(id) return false;
        bytes.assign(source.begin(),source.end());return true;
    },{},&error),error.c_str());
    struct Worker { int unit;float x,y;int facet,frame;float delay; };
    struct Fixture { uint32_t seed,finalState;Worker workers[3]; };
    const Fixture fixtures[]={
        {1u,1943766570u,{{83,19.5f,2.5f,4,7,0.000000000f},{83,17.5f,2.5f,3,0,0.000000000f},{83,15.5f,2.5f,0,7,1.398693800f}}},
        {12345u,229778277u,{{293,17.5f,9.5f,4,7,0.000000000f},{293,20.5f,8.5f,6,4,0.000000000f},{293,19.5f,8.5f,5,1,0.000000000f}}},
        {4294967295u,3382715282u,{{83,20.5f,7.5f,3,5,0.000000000f},{293,19.5f,7.5f,0,2,0.000000000f},{293,17.5f,11.5f,1,1,0.000000000f}}},
    };
    swgb::Game game(assets);
    for (uint16_t civ=1;civ<=8;++civ) for (const auto &f:fixtures) {
        uint32_t state=f.seed;swgb::RmsMap map;
        check(swgb::generateRmsMap(script,{},32,32,assets.dat(),state,map,&error,1,{civ}),error.c_str());
        check(state==f.finalState && map.ownedObjects.size()==4,"Original full worker stage RNG and count");
        for(size_t i=0;i<3;++i) {
            const auto &a=map.ownedObjects[i+1];const auto &e=f.workers[i];
            check(a.unitId==e.unit && a.x==e.x && a.y==e.y && a.facet==e.facet && a.animationFrame==e.frame &&
                  std::abs(a.initialIdleDelay-e.delay)<.00001f && a.owner==1 && a.civilization==civ,
                  "Original worker variant, position, facet and owner");
        }
        state=f.seed;
        check(game.initRms(script,{},32,state,&error,1,{civ}),error.c_str());
        check(state==f.finalState && game.activeObjectCount()==4 && game.populationUsedForTesting(1)==3 &&
              game.populationCapacityForTesting(1)==(civ==5?200.f:5.f),"RMS workers preserve RNG and population room");
        check(game.resource(1,98)==525 && game.resource(1,164)==150 && game.resource(1,165)==375 &&
              game.resource(1,11)==3 && game.resource(1,37)==3 && game.resource(1,0)==0,
              "Starting workers record unit costs and counts without spending food");
        for(size_t i=0;i<3;++i) {
            const auto &e=f.workers[i];const uint32_t id=uint32_t(i+2);
            const auto position=game.objectPosition(id);
            check(game.objectUnitId(id)==e.unit && game.objectPlayer(id)==1 && game.objectHitPoints(id)==(civ==8?55:40) &&
                  position[0]==e.x && position[1]==e.y,"Worker import preserves creation IDs, state and positions");
            const float facing=game.objectFacing(id);
            const float theta=std::atan2(std::cos(facing)+std::sin(facing),2*(std::cos(facing)-std::sin(facing)));
            const int facet=(int(std::lround((theta-1.57079632679f)/.78539816339f))%8+8)%8;
            check(facet==e.facet,"Worker rendered direction matches original final sprite facet");
        }
        if (civ==1 && f.seed==1) {
            swgb::InputState idle;
            const auto animation=game.objectAnimationForTesting(4);
            game.updatePaused(.5f,idle);
            check(game.objectAnimationForTesting(4)==animation,"Pause retains randomized initial idle timing");
            for(int tick=0;tick<5;++tick) game.update(.1f,idle);
            const auto held=game.objectAnimationForTesting(4);
            check(held[0]==7 && std::abs(held[1]-(animation[1]-.5f))<.00001f,
                  "Original prolonged initial frame holds before normal animation");
            game.groupMoveForTesting({4},3,3);
            game.update(.05f,idle);
            check(game.objectAnimationForTesting(4)[1]==0,"New action clears initial idle delay");
        }
        const auto before=state;
        check(!game.initRms(script,{},32,state,&error,3,{civ,civ}) && state==before && game.activeObjectCount()==4,
              "Missing owner civilization preserves existing game and RNG");
    }
    const struct { uint32_t seed,finalState;Worker workers[6]; } pairs[]={
        {1u,892528603u,{{293,16.5f,2.5f,2,0,0.000000000f},{293,19.5f,2.5f,1,7,0.000000000f},{293,15.5f,2.5f,4,4,0.000000000f},{293,10.5f,20.5f,1,7,0.000000000f},{293,10.5f,17.5f,2,4,0.000000000f},{293,11.5f,17.5f,4,6,0.000000000f}}},
        {12345u,3312625743u,{{293,2.5f,17.5f,4,5,0.000000000f},{293,2.5f,18.5f,4,2,0.000000000f},{293,4.5f,17.5f,1,7,2.812677383f},{293,21.5f,8.5f,2,7,1.676320791f},{293,17.5f,12.5f,5,7,0.350291491f},{293,17.5f,11.5f,2,7,1.807580709f}}},
        {4294967295u,1302958861u,{{293,17.5f,11.5f,1,6,0.000000000f},{293,17.5f,7.5f,6,6,0.000000000f},{293,18.5f,7.5f,3,7,1.271462202f},{293,4.5f,18.5f,3,6,0.000000000f},{293,2.5f,19.5f,2,5,0.000000000f},{293,2.5f,21.5f,0,0,0.000000000f}}},
    };
    for(uint16_t civ=1;civ<=8;++civ) for(const auto &f:pairs) {
        uint32_t state=f.seed;swgb::RmsMap map;
        check(swgb::generateRmsMap(script,{},32,32,assets.dat(),state,map,&error,2,{1,civ}),error.c_str());
        check(state==f.finalState && map.ownedObjects.size()==8,"Two-owner full worker stage RNG/order");
        for(size_t i=0;i<6;++i) {
            const auto &a=map.ownedObjects[i+2];const auto &e=f.workers[i];
            check(a.unitId==e.unit && a.x==e.x && a.y==e.y && a.owner==(i<3?1:2) &&
                  a.facet==e.facet && a.animationFrame==e.frame && std::abs(a.initialIdleDelay-e.delay)<.00001f,
                  "Original nonlocal worker positions, IDs, pose and animation");
        }
        state=f.seed;
        check(game.initRms(script,{},32,state,&error,2,{1,civ}),error.c_str());
        check(state==f.finalState && game.activeObjectCount()==8,"Eight native objects from two-owner worker row");
        for(int owner=1;owner<=2;++owner)
            check(game.resource(owner,98)==525 && game.resource(owner,164)==150 && game.resource(owner,165)==375 &&
                  game.resource(owner,11)==3 && game.resource(owner,37)==3 && game.populationUsedForTesting(owner)==3,
                  "Nonlocal owner resources and population remain independent");
        for(size_t i=0;i<6;++i)
            check(game.objectUnitId(uint32_t(i+3))==f.workers[i].unit &&
                  game.objectPlayer(uint32_t(i+3))==(i<3?1:2) &&
                  game.objectHitPoints(uint32_t(i+3))==(i>=3 && civ==8?55:40),
                  "Nonlocal native import retains IDs and applies later civilization HP bonus");
    }
    std::puts("PASS: full original RMS worker placement, variants, facing, population and cost import");
}
