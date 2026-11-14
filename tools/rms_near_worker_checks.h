// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Original executable, zero owner sharing masks, 2026-11-14.
// Actual worker gate ignores existing workers, but blocks either owner's CC and trees.
static void rmsNearWorkerChecks(swgb::Assets &assets) {
    const std::string land=R"(<LAND_GENERATION>
base_terrain 41
create_player_lands {
  terrain_type 9
  base_size 3
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
create_player_lands { terrain_type 0 base_size 3 number_of_tiles 60 }
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
        {0u,14u,1u,3316638276u,0u,435603717u,2166136261u,6,3,6,7,{{83,3.5f,2.5f,2,7,0.8279976844787598f},{293,3.5f,3.5f,3,2,0.0f},{293,3.5f,0.5f,4,1,0.0f},{293,3.5f,4.5f,5,3,0.0f},{293,8.5f,4.5f,5,1,0.0f},{293,9.5f,4.5f,0,1,0.0f}}},
        {0u,14u,8u,4153334494u,0u,3375236133u,2166136261u,7,3,5,7,{{83,6.5f,0.5f,4,3,0.0f},{83,4.5f,1.5f,0,7,2.859675884246826f},{293,4.5f,0.5f,3,7,0.0f},{293,2.5f,7.5f,2,5,0.0f},{293,2.5f,4.5f,4,6,0.0f},{293,3.5f,4.5f,5,2,0.0f}}},
        {0u,14u,1967u,457183562u,0u,2954650917u,2166136261u,5,7,7,3,{{83,2.5f,4.5f,5,7,0.774285078048706f},{83,3.5f,4.5f,5,7,1.463820219039917f},{293,4.5f,4.5f,2,7,1.2409131526947021f},{83,4.5f,2.5f,0,6,0.0f},{293,6.5f,0.5f,7,7,2.940244674682617f},{293,4.5f,0.5f,7,1,0.0f}}},
        {1u,12u,1u,23395191u,0u,2005791904u,2166136261u,6,3,5,6,{{293,3.5f,0.5f,7,3,0.0f},{293,4.5f,0.5f,1,2,0.0f},{293,5.5f,0.5f,6,3,0.0f},{293,2.5f,5.5f,5,7,2.225867509841919f},{293,2.5f,7.5f,5,7,2.2097537517547607f},{293,2.5f,3.5f,1,4,0.0f}}},
        {1u,12u,12345u,3567726727u,0u,774819290u,2166136261u,3,7,6,4,{{293,0.5f,4.5f,4,7,0.04412972927093506f},{293,0.5f,5.5f,7,0,0.0f},{293,1.5f,4.5f,3,5,0.0f},{83,3.5f,3.5f,1,5,0.0f},{83,5.5f,1.5f,6,7,0.0f},{83,3.5f,1.5f,2,7,0.0f}}},
        {1u,12u,4294967295u,3471676970u,0u,3284464459u,2166136261u,6,4,3,7,{{293,6.5f,1.5f,0,3,0.0f},{293,5.5f,1.5f,2,7,2.918759822845459f},{293,3.5f,3.5f,5,1,0.0f},{83,0.5f,5.5f,3,6,0.0f},{83,0.5f,6.5f,1,4,0.0f},{293,0.5f,4.5f,2,7,2.9533371925354004f}}},
        {1u,16u,1u,319481484u,0u,3970802188u,2166136261u,9,4,7,10,{{293,6.5f,2.5f,1,7,0.0f},{293,6.5f,1.5f,4,1,0.0f},{293,7.5f,1.5f,0,2,0.0f},{293,4.5f,8.5f,6,7,0.2935575246810913f},{293,4.5f,7.5f,5,6,0.0f},{293,5.5f,7.5f,0,6,0.0f}}},
        {1u,16u,12345u,3293418364u,0u,3739427279u,2166136261u,4,10,10,6,{{83,1.5f,7.5f,1,7,0.0f},{83,1.5f,8.5f,3,2,0.0f},{293,5.5f,7.5f,0,0,0.0f},{293,7.5f,7.5f,7,5,0.0f},{293,7.5f,5.5f,1,7,0.0f},{293,7.5f,3.5f,6,2,0.0f}}},
        {1u,16u,4294967295u,2114676846u,0u,1905949289u,2166136261u,10,6,4,10,{{293,10.5f,3.5f,4,7,2.0489516258239746f},{293,8.5f,3.5f,2,7,1.9509263038635254f},{293,7.5f,3.5f,5,7,0.0f},{293,5.5f,7.5f,6,6,0.0f},{293,3.5f,7.5f,5,1,0.0f},{293,1.5f,7.5f,3,2,0.0f}}},
        {1u,20u,1u,1360277262u,2u,1945014062u,526814996u,10,4,8,12,{{293,7.5f,2.5f,5,2,0.0f},{293,8.5f,1.5f,6,4,0.0f},{293,7.5f,4.5f,4,7,1.236548900604248f},{83,9.5f,9.5f,2,7,0.7967772483825684f},{293,5.5f,13.5f,5,3,0.0f},{293,5.5f,9.5f,2,7,0.26569414138793945f}}},
        {1u,20u,12345u,1757718167u,20u,30901886u,997163840u,4,12,12,6,{{83,5.5f,9.5f,7,4,0.0f},{293,1.5f,10.5f,6,7,0.6275826692581177f},{293,4.5f,9.5f,3,7,0.0f},{83,9.5f,3.5f,6,7,0.35331273078918457f},{83,9.5f,7.5f,3,0,0.0f},{83,9.5f,5.5f,6,3,0.0f}}},
        {1u,20u,4294967295u,1820852599u,16u,3984825712u,2901758904u,12,6,4,12,{{293,10.5f,3.5f,3,7,0.7920774221420288f},{293,9.5f,6.5f,4,5,0.0f},{293,12.5f,3.5f,1,7,2.766350269317627f},{293,4.5f,9.5f,0,7,0.11932730674743652f},{293,1.5f,9.5f,3,2,0.0f},{293,5.5f,9.5f,7,0,0.0f}}},
        {1u,24u,1u,3333471433u,37u,2546082054u,4082123162u,12,4,10,14,{{293,9.5f,3.5f,2,5,0.0f},{293,9.5f,1.5f,6,7,0.503036618232727f},{293,10.5f,1.5f,6,5,0.0f},{83,7.5f,15.5f,4,7,2.377605438232422f},{83,8.5f,11.5f,6,0,0.0f},{293,7.5f,14.5f,2,7,0.0f}}},
        {1u,24u,12345u,2125999897u,38u,166511813u,4112937402u,4,14,14,8,{{83,4.5f,11.5f,3,7,2.919431209564209f},{293,1.5f,11.5f,4,2,0.0f},{293,2.5f,11.5f,0,2,0.0f},{83,15.5f,5.5f,2,7,2.539078950881958f},{293,11.5f,5.5f,1,7,1.0361340045928955f},{293,12.5f,5.5f,0,2,0.0f}}},
        {1u,24u,4294967295u,1131199469u,38u,2533458808u,4183357089u,14,7,4,15,{{293,11.5f,4.5f,4,7,2.9570298194885254f},{293,12.5f,4.5f,0,4,0.0f},{293,13.5f,4.5f,2,7,2.9728078842163086f},{293,1.5f,14.5f,1,7,0.6883449554443359f},{293,1.5f,13.5f,1,2,0.0f},{293,1.5f,12.5f,3,7,0.04211556911468506f}}},
        {2u,12u,1u,409821674u,4u,1986448999u,505731179u,6,3,5,6,{{83,3.5f,2.5f,5,3,0.0f},{83,3.5f,0.5f,5,4,0.0f},{83,4.5f,0.5f,7,7,1.9730827808380127f},{293,2.5f,3.5f,0,7,1.0341196060180664f},{293,3.5f,3.5f,1,7,1.2446057796478271f},{293,8.5f,3.5f,3,7,1.4383068084716797f}}},
        {2u,12u,12345u,396218649u,4u,310018107u,2674859499u,3,7,6,4,{{293,0.5f,8.5f,1,7,0.15390479564666748f},{293,0.5f,5.5f,6,7,0.0f},{293,1.5f,4.5f,5,6,0.0f},{293,7.5f,1.5f,6,4,0.0f},{293,5.5f,1.5f,1,7,0.0f},{293,3.5f,4.5f,0,4,0.0f}}},
        {2u,12u,4294967295u,1206943953u,4u,1082411946u,2685072703u,6,4,3,7,{{83,4.5f,1.5f,5,2,0.0f},{293,5.5f,1.5f,5,0,0.0f},{293,6.5f,1.5f,7,7,2.758293390274048f},{293,3.5f,4.5f,7,2,0.0f},{293,0.5f,4.5f,7,5,0.0f},{293,1.5f,4.5f,4,7,1.8633074760437012f}}},
        {2u,16u,1u,4073076422u,4u,3297448794u,603497776u,9,4,7,10,{{83,6.5f,5.5f,4,7,0.271065354347229f},{293,9.5f,1.5f,7,3,0.0f},{293,8.5f,1.5f,3,4,0.0f},{293,7.5f,7.5f,3,4,0.0f},{293,4.5f,11.5f,7,5,0.0f},{293,4.5f,7.5f,6,5,0.0f}}},
        {2u,16u,12345u,2522815191u,4u,2226035578u,4036099419u,4,10,10,6,{{293,4.5f,7.5f,0,0,0.0f},{293,3.5f,7.5f,6,2,0.0f},{293,1.5f,7.5f,3,6,0.0f},{83,7.5f,3.5f,7,6,0.0f},{83,7.5f,6.5f,3,7,0.0f},{293,10.5f,3.5f,5,3,0.0f}}},
        {2u,16u,4294967295u,3302097141u,4u,1365239321u,21383286u,10,6,4,10,{{83,7.5f,3.5f,2,2,0.0f},{293,7.5f,7.5f,1,2,0.0f},{293,8.5f,3.5f,2,1,0.0f},{293,2.5f,7.5f,7,2,0.0f},{293,1.5f,7.5f,5,5,0.0f},{293,3.5f,7.5f,7,4,0.0f}}},
        {2u,20u,1u,3421864380u,4u,1443909030u,3161870651u,10,4,8,12,{{293,9.5f,1.5f,2,6,0.0f},{293,10.5f,1.5f,3,0,0.0f},{293,11.5f,1.5f,4,7,0.8622394800186157f},{293,5.5f,10.5f,6,7,0.0f},{293,6.5f,9.5f,1,0,0.0f},{293,5.5f,13.5f,4,7,0.0f}}},
        {2u,20u,12345u,749947432u,4u,488503660u,2671516176u,4,12,12,6,{{83,1.5f,11.5f,5,3,0.0f},{293,1.5f,9.5f,5,7,2.0234382152557373f},{293,2.5f,9.5f,2,7,0.0f},{293,9.5f,6.5f,4,0,0.0f},{293,11.5f,3.5f,1,7,0.0f},{293,13.5f,3.5f,6,7,0.0f}}},
        {2u,20u,4294967295u,1776953419u,4u,611343609u,1175618645u,12,6,4,12,{{293,9.5f,4.5f,1,7,0.0f},{293,9.5f,3.5f,0,0,0.0f},{293,10.5f,3.5f,0,2,0.0f},{83,1.5f,11.5f,6,7,0.8796960115432739f},{293,1.5f,9.5f,6,7,0.0f},{293,2.5f,9.5f,7,7,2.611926555633545f}}},
        {2u,24u,1u,700349950u,4u,3260006698u,1539318386u,12,4,10,14,{{293,9.5f,1.5f,6,0,0.0f},{293,10.5f,1.5f,6,7,1.8253731727600098f},{293,11.5f,1.5f,4,3,0.0f},{83,7.5f,12.5f,1,5,0.0f},{293,10.5f,11.5f,1,3,0.0f},{293,7.5f,11.5f,0,7,0.44965970516204834f}}},
        {2u,24u,12345u,1539563952u,4u,2706469960u,4175201568u,4,14,14,8,{{293,1.5f,15.5f,5,4,0.0f},{293,1.5f,14.5f,3,7,0.9559007883071899f},{293,1.5f,11.5f,0,7,0.0f},{293,11.5f,9.5f,7,7,2.7112948894500732f},{293,13.5f,5.5f,4,4,0.0f},{293,11.5f,7.5f,4,7,0.0f}}},
        {2u,24u,4294967295u,3146351222u,4u,2283553290u,3534609754u,14,7,4,15,{{83,14.5f,4.5f,3,7,2.6360971927642822f},{293,11.5f,6.5f,6,7,2.1728262901306152f},{293,11.5f,4.5f,3,7,2.154698371887207f},{83,1.5f,15.5f,3,6,0.0f},{83,1.5f,14.5f,4,7,2.471266746520996f},{293,3.5f,12.5f,3,2,0.0f}}},
    };
    for(const auto &f:cases) {
        uint32_t state=f.seed;swgb::RmsMap map;
        check(swgb::generateRmsMap(scripts[f.profile],{},f.size,f.size,assets.dat(),state,map,&error,2,{1,1}),error.c_str());
        check(state==f.finalState && map.scenery.size()==f.trees && map.ownedObjects.size()==8,
              "Original near-worker counts and complete RNG");
        uint32_t terrainHash=2166136261u,sceneryHash=terrainHash;
        for(auto terrain:map.terrain)terrainHash=(terrainHash^terrain)*16777619u;
        for(const auto &tree:map.scenery) {
            check(tree.alive,"Original near-worker scenery retained");
            for(uint32_t value:{uint32_t(tree.unitId),uint32_t(tree.x*2),uint32_t(tree.y*2),uint32_t(tree.facet)})
                sceneryHash=(sceneryHash^value)*16777619u;
        }
        check(terrainHash==f.terrainHash && sceneryHash==f.sceneryHash,"Original near-worker terrain and scenery");
        const auto &a=map.ownedObjects[0];const auto &b=map.ownedObjects[1];
        check(a.unitId==109 && a.owner==1 && a.x==f.x1 && a.y==f.y1 && a.pendingFoundation==255 &&
              b.unitId==109 && b.owner==2 && b.x==f.x2 && b.y==f.y2 && b.pendingFoundation==27,
              "Original near-worker CC positions and deferred enemy foundation");
        for(size_t i=0;i<6;++i) {
            const auto &actual=map.ownedObjects[i+2];const auto &expected=f.workers[i];
            check(actual.owner==(i<3?1:2) && actual.unitId==expected.unit && actual.x==expected.x &&
                  actual.y==expected.y && actual.facet==expected.facet && actual.animationFrame==expected.frame &&
                  std::abs(actual.initialIdleDelay-expected.delay)<.00001f,
                  "Original near-worker order, variants, positions, facing and initial animation");
        }
        state=f.seed;
        swgb::Game game(assets);
        check(game.initRms(scripts[f.profile],{},f.size,state,&error,2,{1,1}),error.c_str());
        check(state==f.finalState && game.activeObjectCount()==f.trees+8,
              "Near-worker native import preserves RNG and active objects");
    }
    std::puts("PASS: original near-worker grass and forest full-stage comparisons");
}
