// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Original executable, zero owner sharing masks, 2026-11-14.
// Actual worker gate ignores existing workers, but blocks either owner's CC and trees.
static void rmsClippedWorkerChecks(swgb::Assets &assets) {
    const std::string land=R"(<LAND_GENERATION>
base_terrain 41
create_player_lands {
  terrain_type 9
  base_size 2
  number_of_tiles 60
}
<TERRAIN_GENERATION>
)";
    const std::string stock=R"(create_terrain 10 {
  set_avoid_player_start_areas
  base_terrain 41
  number_of_tiles 37
  number_of_clumps 4
  spacing_to_other_terrain_types 1
}
)";
    const std::string inland=R"(create_terrain 10 {
  base_terrain 9
  number_of_tiles 4
  number_of_clumps 2
  spacing_to_other_terrain_types 1
}
)";
    const std::string finish=R"(create_terrain 6 {
  base_terrain 9
  number_of_tiles 9
  number_of_clumps 2
  clumping_factor 6
}
<OBJECTS_GENERATION>
create_object 109 {
  number_of_objects 1
  set_place_for_every_player
  min_distance_to_players 0
  max_distance_to_players 0
}
)";
    const std::string workers=R"(create_object 83 {
 number_of_objects 1
 set_place_for_every_player
 min_distance_to_players 1
 max_distance_to_players 3
}
)";
    const std::string grass=R"(<LAND_GENERATION>
base_terrain 0
create_player_lands { terrain_type 0 base_size 2 number_of_tiles 60 }
<OBJECTS_GENERATION>
create_object 109 { number_of_objects 1 set_place_for_every_player min_distance_to_players 0 max_distance_to_players 0 }
)";
    swgb::RmsScript scripts[3];std::string error;
    for(unsigned profile=0;profile<3;++profile) {
        const auto source=(profile?land+(profile==1?stock:inland)+finish:grass)+workers;
        check(scripts[profile].load(0,[&](int32_t id,std::vector<uint8_t> &bytes) {
            if(id)return false;bytes.assign(source.begin(),source.end());return true;
        },{},&error),error.c_str());
    }
    struct Worker { int unit;float x,y;int facet,frame;float delay; };
    const struct { uint32_t profile,size,seed,finalState,trees,terrainHash,sceneryHash;int x1,y1,x2,y2;Worker workers[6]; } cases[]={
        {0u,8u,1u,2608792336u,0u,2701429973u,2166136261u,3,2,3,4,{{83,0.5f,3.5f,1,6,0.0f},{293,0.5f,0.5f,6,2,0.0f},{293,5.5f,0.5f,1,7,1.6390576362609863f},{293,0.5f,1.5f,1,4,0.0f},{293,5.5f,1.5f,7,6,0.0f},{293,6.5f,1.5f,1,7,1.9838252067565918f}}},
        {0u,8u,12345u,3619364661u,0u,2430892085u,2166136261u,2,4,4,2,{{293,0.5f,1.5f,5,5,0.0f},{293,1.5f,1.5f,6,5,0.0f},{293,4.5f,4.5f,2,5,0.0f},{83,1.5f,1.5f,6,6,0.0f},{83,1.5f,0.5f,4,2,0.0f},{293,6.5f,0.5f,5,0,0.0f}}},
        {0u,8u,4294967295u,1817071324u,0u,1778475317u,2166136261u,4,2,2,4,{{293,1.5f,1.5f,4,7,2.932859182357788f},{293,1.5f,0.5f,2,7,1.9855036735534668f},{293,6.5f,0.5f,3,2,0.0f},{83,1.5f,1.5f,0,4,0.0f},{83,0.5f,1.5f,7,1,0.0f},{83,4.5f,4.5f,3,7,0.0f}}},
        {0u,8u,8u,3884604737u,0u,2701429973u,2166136261u,3,2,3,4,{{83,0.5f,0.5f,2,6,0.0f},{83,5.5f,0.5f,7,2,0.0f},{83,6.5f,0.5f,5,7,1.3268532752990723f},{293,0.5f,3.5f,4,7,2.9731438159942627f},{293,0.5f,1.5f,5,7,0.0f},{293,5.5f,1.5f,5,7,0.0f}}},
        {0u,8u,1967u,3884498987u,0u,2024859861u,2166136261u,3,4,3,2,{{293,0.5f,2.5f,6,7,0.0f},{293,0.5f,1.5f,1,6,0.0f},{293,5.5f,1.5f,6,7,2.018738269805908f},{83,0.5f,0.5f,4,6,0.0f},{293,0.5f,3.5f,5,7,2.8210699558258057f},{293,5.5f,0.5f,3,6,0.0f}}},
        {0u,9u,1u,1529594182u,0u,3709476095u,2166136261u,5,2,4,5,{{293,2.5f,0.5f,1,7,2.5810418128967285f},{293,7.5f,0.5f,7,7,1.155980110168457f},{293,8.5f,0.5f,6,7,1.6806848049163818f},{293,1.5f,3.5f,1,6,0.0f},{293,1.5f,6.5f,0,7,1.7239906787872314f},{293,1.5f,2.5f,6,1,0.0f}}},
        {0u,9u,12345u,1482652750u,0u,2454750783u,2166136261u,2,6,5,3,{{293,0.5f,3.5f,6,7,0.8363902568817139f},{293,1.5f,3.5f,1,5,0.0f},{293,2.5f,3.5f,1,1,0.0f},{293,2.5f,0.5f,5,7,1.5611743927001953f},{293,3.5f,0.5f,2,7,2.073793649673462f},{293,4.5f,0.5f,7,4,0.0f}}},
        {0u,9u,4294967295u,313114158u,0u,3300166207u,2166136261u,5,3,2,6,{{293,4.5f,0.5f,5,3,0.0f},{293,2.5f,0.5f,7,4,0.0f},{293,3.5f,0.5f,6,4,0.0f},{83,0.5f,3.5f,1,4,0.0f},{83,1.5f,3.5f,3,0,0.0f},{83,2.5f,3.5f,2,6,0.0f}}},
        {0u,9u,8u,1688512537u,0u,3709476095u,2166136261u,5,2,4,5,{{83,2.5f,1.5f,7,7,0.0f},{293,2.5f,0.5f,6,7,1.1140172481536865f},{293,7.5f,0.5f,4,7,0.0f},{293,1.5f,4.5f,5,1,0.0f},{293,1.5f,2.5f,4,7,0.22540974617004395f},{293,2.5f,2.5f,7,7,0.0f}}},
        {0u,9u,1967u,3515692686u,0u,2632717695u,2166136261u,4,5,5,2,{{83,2.5f,2.5f,7,1,0.0f},{83,1.5f,2.5f,7,4,0.0f},{83,7.5f,2.5f,2,1,0.0f},{293,2.5f,2.5f,6,7,0.0f},{293,2.5f,0.5f,6,2,0.0f},{293,7.5f,0.5f,7,3,0.0f}}},
        {0u,10u,1u,4200042692u,0u,3309554085u,2166136261u,5,2,4,5,{{83,2.5f,0.5f,5,7,0.20526742935180664f},{83,7.5f,0.5f,6,5,0.0f},{293,8.5f,0.5f,6,7,1.3419599533081055f},{83,1.5f,5.5f,3,7,1.6316721439361572f},{83,1.5f,2.5f,3,2,0.0f},{293,2.5f,2.5f,1,2,0.0f}}},
        {0u,10u,12345u,527207278u,0u,2342841797u,2166136261u,2,6,5,3,{{293,0.5f,3.5f,1,5,0.0f},{293,1.5f,3.5f,4,7,0.5916622877120972f},{293,2.5f,3.5f,0,7,1.2127139568328857f},{293,2.5f,1.5f,3,1,0.0f},{293,2.5f,2.5f,0,1,0.0f},{293,2.5f,3.5f,7,7,2.8217413425445557f}}},
        {0u,10u,4294967295u,1541566829u,0u,3725568997u,2166136261u,5,3,2,6,{{83,6.5f,0.5f,4,1,0.0f},{83,2.5f,1.5f,7,4,0.0f},{83,5.5f,0.5f,3,6,0.0f},{293,2.5f,3.5f,3,0,0.0f},{293,0.5f,3.5f,3,3,0.0f},{293,1.5f,3.5f,7,4,0.0f}}},
        {0u,10u,8u,1333155968u,0u,3309554085u,2166136261u,5,2,4,5,{{83,2.5f,0.5f,3,7,0.0f},{83,2.5f,1.5f,2,2,0.0f},{293,7.5f,0.5f,3,7,0.8216192722320557f},{83,1.5f,5.5f,3,1,0.0f},{293,1.5f,2.5f,7,2,0.0f},{293,2.5f,2.5f,2,2,0.0f}}},
        {0u,10u,1967u,700418498u,0u,2297390021u,2166136261u,4,5,5,2,{{293,1.5f,4.5f,6,7,1.2842187881469727f},{293,1.5f,3.5f,3,7,0.7225867509841919f},{293,2.5f,2.5f,4,7,1.381572961807251f},{293,2.5f,2.5f,2,7,0.10623490810394287f},{293,2.5f,0.5f,2,0,0.0f},{293,7.5f,0.5f,6,0,0.0f}}},
        {0u,12u,1u,3505849627u,0u,497125077u,2166136261u,5,2,5,6,{{83,2.5f,2.5f,5,6,0.0f},{293,2.5f,0.5f,2,7,0.0f},{293,7.5f,0.5f,2,6,0.0f},{293,2.5f,4.5f,0,5,0.0f},{293,2.5f,7.5f,4,7,0.0f},{293,2.5f,3.5f,3,0,0.0f}}},
        {0u,12u,12345u,619990524u,0u,1010265461u,2166136261u,2,7,6,3,{{83,1.5f,4.5f,2,7,0.3727835416793823f},{293,0.5f,4.5f,2,7,1.6031372547149658f},{293,2.5f,4.5f,5,2,0.0f},{293,4.5f,0.5f,4,4,0.0f},{293,3.5f,2.5f,4,4,0.0f},{293,3.5f,0.5f,3,7,0.6470534801483154f}}},
        {0u,12u,4294967295u,3660458647u,0u,1925613237u,2166136261u,6,3,2,7,{{293,3.5f,2.5f,3,1,0.0f},{293,7.5f,0.5f,7,3,0.0f},{293,5.5f,0.5f,3,4,0.0f},{83,1.5f,4.5f,0,2,0.0f},{293,2.5f,4.5f,3,7,0.0f},{293,0.5f,4.5f,7,7,1.077761173248291f}}},
        {0u,12u,8u,2234416757u,0u,3420471157u,2166136261u,6,2,4,6,{{83,3.5f,2.5f,7,7,0.0f},{83,3.5f,0.5f,3,7,0.0f},{293,8.5f,0.5f,1,5,0.0f},{293,1.5f,5.5f,7,1,0.0f},{293,1.5f,3.5f,4,1,0.0f},{293,3.5f,3.5f,0,2,0.0f}}},
        {0u,12u,1967u,3058960960u,0u,251897397u,2166136261u,4,6,6,2,{{83,2.5f,3.5f,2,5,0.0f},{83,3.5f,3.5f,6,0,0.0f},{293,1.5f,3.5f,2,2,0.0f},{293,3.5f,2.5f,3,5,0.0f},{293,3.5f,1.5f,5,2,0.0f},{293,3.5f,0.5f,0,4,0.0f}}},
        {0u,6u,1u,939369794u,0u,2778902149u,2166136261u,2,2,3,3,{{293,4.5f,0.5f,2,2,0.0f},{293,5.5f,0.5f,7,7,0.9125950336456299f},{293,5.5f,1.5f,1,6,0.0f},{83,4.5f,0.5f,1,2,0.0f},{293,5.5f,0.5f,4,7,1.9086275100708008f},{293,5.5f,1.5f,4,7,0.9508651494979858f}}},
        {0u,6u,12345u,3509082334u,0u,99703109u,2166136261u,2,3,3,2,{{83,0.5f,0.5f,0,4,0.0f},{83,5.5f,0.5f,3,0,0.0f},{293,5.5f,1.5f,1,1,0.0f},{83,0.5f,0.5f,4,3,0.0f},{83,5.5f,0.5f,4,1,0.0f},{293,5.5f,1.5f,0,7,2.772728681564331f}}},
        {0u,6u,4294967295u,3062912940u,0u,3388525477u,2166136261u,3,2,2,3,{{83,0.5f,0.5f,5,7,0.0f},{83,5.5f,0.5f,1,2,0.0f},{83,5.5f,1.5f,0,3,0.0f},{293,0.5f,0.5f,2,2,0.0f},{293,5.5f,0.5f,3,7,0.0f},{293,5.5f,1.5f,2,2,0.0f}}},
        {0u,6u,8u,689349622u,0u,2778902149u,2166136261u,2,2,3,3,{{293,4.5f,0.5f,3,7,1.3449811935424805f},{293,5.5f,0.5f,6,3,0.0f},{293,5.5f,1.5f,2,3,0.0f},{83,4.5f,0.5f,4,2,0.0f},{293,5.5f,0.5f,2,0,0.0f},{293,5.5f,1.5f,6,7,1.9710683822631836f}}},
        {0u,6u,1967u,1484863830u,0u,3473585253u,2166136261u,3,3,2,2,{{293,4.5f,0.5f,1,2,0.0f},{293,5.5f,0.5f,3,7,0.0f},{293,5.5f,1.5f,1,3,0.0f},{293,4.5f,0.5f,3,7,0.0f},{293,5.5f,0.5f,2,7,1.6370434761047363f},{293,5.5f,1.5f,2,0,0.0f}}},
        {0u,7u,1u,2915123068u,0u,3375549119u,2166136261u,2,2,3,3,{{293,4.5f,0.5f,6,7,0.0f},{293,5.5f,0.5f,2,7,2.2634663581848145f},{293,5.5f,1.5f,1,6,0.0f},{293,4.5f,0.5f,0,6,0.0f},{293,5.5f,0.5f,1,0,0.0f},{293,6.5f,0.5f,7,5,0.0f}}},
        {0u,7u,12345u,2108129520u,0u,3811430975u,2166136261u,2,3,3,2,{{83,0.5f,0.5f,2,7,0.888759970664978f},{83,5.5f,0.5f,3,7,2.9462873935699463f},{293,5.5f,1.5f,1,6,0.0f},{83,0.5f,0.5f,3,5,0.0f},{293,5.5f,0.5f,0,5,0.0f},{293,6.5f,0.5f,6,7,2.4094972610473633f}}},
        {0u,7u,4294967295u,2331022848u,0u,1484863999u,2166136261u,3,2,2,3,{{83,0.5f,0.5f,0,0,0.0f},{83,5.5f,0.5f,6,7,2.8996245861053467f},{293,6.5f,0.5f,2,7,0.05319380760192871f},{293,0.5f,0.5f,4,7,1.1120028495788574f},{293,5.5f,0.5f,5,6,0.0f},{293,5.5f,1.5f,6,2,0.0f}}},
        {0u,7u,8u,2088930554u,0u,3375549119u,2166136261u,2,2,3,3,{{83,4.5f,0.5f,7,2,0.0f},{83,5.5f,0.5f,7,5,0.0f},{293,5.5f,1.5f,5,6,0.0f},{83,4.5f,0.5f,2,4,0.0f},{293,5.5f,0.5f,5,1,0.0f},{293,6.5f,0.5f,7,0,0.0f}}},
        {0u,7u,1967u,2274933611u,0u,1683168959u,2166136261u,3,3,2,2,{{293,4.5f,0.5f,3,1,0.0f},{293,5.5f,0.5f,3,7,2.8643758296966553f},{293,6.5f,0.5f,3,1,0.0f},{83,4.5f,0.5f,0,2,0.0f},{83,5.5f,0.5f,4,7,2.3571276664733887f},{83,5.5f,1.5f,4,2,0.0f}}},
        {2u,8u,1u,2439777195u,4u,3595804343u,1915878902u,3,2,3,4,{{83,0.5f,2.5f,2,3,0.0f},{293,0.5f,0.5f,1,6,0.0f},{293,5.5f,0.5f,3,1,0.0f},{83,0.5f,1.5f,0,2,0.0f},{293,5.5f,1.5f,5,7,1.000885009765625f},{293,0.5f,2.5f,4,6,0.0f}}},
        {2u,8u,12345u,2513526906u,4u,201609723u,2538841917u,2,4,4,2,{{293,0.5f,1.5f,7,5,0.0f},{293,1.5f,1.5f,0,7,2.6669819355010986f},{293,5.5f,4.5f,2,1,0.0f},{293,1.5f,0.5f,3,3,0.0f},{293,6.5f,0.5f,5,4,0.0f},{293,7.5f,0.5f,1,5,0.0f}}},
        {2u,8u,4294967295u,559312427u,4u,1977717098u,39741669u,4,2,2,4,{{293,1.5f,0.5f,3,6,0.0f},{293,6.5f,0.5f,2,7,0.0f},{293,7.5f,0.5f,0,6,0.0f},{293,0.5f,1.5f,5,5,0.0f},{293,1.5f,1.5f,3,7,0.41978204250335693f},{293,4.5f,4.5f,0,5,0.0f}}},
        {2u,12u,1u,3020037260u,4u,3334412409u,2637377491u,5,2,5,6,{{293,2.5f,0.5f,1,7,0.16934716701507568f},{293,2.5f,3.5f,5,0,0.0f},{293,7.5f,0.5f,0,7,0.06460762023925781f},{83,2.5f,4.5f,5,2,0.0f},{293,2.5f,3.5f,4,1,0.0f},{293,7.5f,3.5f,3,7,2.4340038299560547f}}},
        {2u,12u,12345u,1044750498u,4u,710293658u,3445907455u,2,7,6,3,{{83,0.5f,4.5f,0,3,0.0f},{293,1.5f,4.5f,1,7,1.995574712753296f},{293,2.5f,4.5f,0,2,0.0f},{83,3.5f,0.5f,4,2,0.0f},{83,4.5f,0.5f,1,7,0.0f},{83,5.5f,0.5f,0,4,0.0f}}},
        {2u,12u,4294967295u,4278218705u,4u,1426162262u,3744705286u,6,3,2,7,{{293,4.5f,0.5f,5,0,0.0f},{293,3.5f,2.5f,6,0,0.0f},{293,5.5f,0.5f,3,7,2.9483015537261963f},{83,0.5f,4.5f,2,7,0.0f},{293,1.5f,4.5f,6,7,1.99859619140625f},{293,2.5f,4.5f,5,7,0.0f}}},
    };
    for(const auto &f:cases) {
        uint32_t state=f.seed;swgb::RmsMap map;
        check(swgb::generateRmsMap(scripts[f.profile],{},f.size,f.size,assets.dat(),state,map,&error,2,{1,1}),error.c_str());
        check(state==f.finalState && map.scenery.size()==f.trees && map.ownedObjects.size()==8,
              "Original clipped-worker counts and complete RNG");
        uint32_t terrainHash=2166136261u,sceneryHash=terrainHash;
        for(auto terrain:map.terrain)terrainHash=(terrainHash^terrain)*16777619u;
        for(const auto &tree:map.scenery) {
            check(tree.alive,"Original clipped-worker scenery retained");
            for(uint32_t value:{uint32_t(tree.unitId),uint32_t(tree.x*2),uint32_t(tree.y*2),uint32_t(tree.facet)})
                sceneryHash=(sceneryHash^value)*16777619u;
        }
        check(terrainHash==f.terrainHash && sceneryHash==f.sceneryHash,"Original clipped-worker terrain and scenery");
        const auto &a=map.ownedObjects[0];const auto &b=map.ownedObjects[1];
        check(a.unitId==109 && a.owner==1 && a.x==f.x1 && a.y==f.y1 && a.pendingFoundation==255 &&
              b.unitId==109 && b.owner==2 && b.x==f.x2 && b.y==f.y2 && b.pendingFoundation==27,
              "Original clipped-worker CC positions and deferred enemy foundation");
        for(size_t i=0;i<6;++i) {
            const auto &actual=map.ownedObjects[i+2];const auto &expected=f.workers[i];
            check(actual.owner==(i<3?1:2) && actual.unitId==expected.unit && actual.x==expected.x &&
                  actual.y==expected.y && actual.facet==expected.facet && actual.animationFrame==expected.frame &&
                  std::abs(actual.initialIdleDelay-expected.delay)<.00001f,
                  "Original clipped-worker order, variants, positions, facing and initial animation");
        }
        state=f.seed;
        swgb::Game game(assets);
        check(game.initRms(scripts[f.profile],{},f.size,state,&error,2,{1,1}),error.c_str());
        check(state==f.finalState && game.activeObjectCount()==f.trees+8,
              "Clipped worker import preserves original count and RNG");
    }
    std::puts("PASS: original clipped-worker grass and forest full-stage comparisons");
}
