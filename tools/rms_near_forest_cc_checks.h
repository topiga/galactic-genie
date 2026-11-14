// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Original CC executable oracle, 2026-11-14: 45 natural near-start forest cases.
// Coordinate/hash inputs are original RMS coordinates; Game performs its normal import.
static void nearForestCommandCenterChecks(swgb::Assets &assets) {
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
    swgb::RmsScript scripts[2];std::string error;
    for(unsigned profile=0;profile<2;++profile) {
        const std::string source=land+(profile?inland:stock)+finish;
        check(scripts[profile].load(0,[&](int32_t id,std::vector<uint8_t> &bytes) {
            if(id)return false;
            bytes.assign(source.begin(),source.end());return true;
        },{},&error),error.c_str());
    }
    const struct {
        uint32_t profile,size,seed,civ1,civ2,finalState,trees,terrainHash,sceneryHash,x1,y1,x2,y2;
    } cases[]={
        {0u, 12u, 1u, 1u, 1u, 1446536323u, 0u, 2005791904u, 2166136261u, 6u, 3u, 5u, 6u},
        {0u, 12u, 12345u, 1u, 1u, 2523021713u, 0u, 774819290u, 2166136261u, 3u, 7u, 6u, 4u},
        {0u, 12u, 4294967295u, 1u, 1u, 2364592492u, 0u, 3284464459u, 2166136261u, 6u, 4u, 3u, 7u},
        {0u, 16u, 1u, 1u, 1u, 2386695432u, 0u, 3970802188u, 2166136261u, 9u, 4u, 7u, 10u},
        {0u, 16u, 12345u, 1u, 1u, 51968014u, 0u, 3739427279u, 2166136261u, 4u, 10u, 10u, 6u},
        {0u, 16u, 4294967295u, 1u, 1u, 2677479498u, 0u, 1905949289u, 2166136261u, 10u, 6u, 4u, 10u},
        {0u, 20u, 1u, 1u, 1u, 382618099u, 2u, 1945014062u, 526814996u, 10u, 4u, 8u, 12u},
        {0u, 20u, 12345u, 1u, 1u, 429332534u, 20u, 30901886u, 997163840u, 4u, 12u, 12u, 6u},
        {0u, 20u, 4294967295u, 1u, 1u, 372345475u, 16u, 3984825712u, 2901758904u, 12u, 6u, 4u, 12u},
        {0u, 24u, 1u, 1u, 1u, 406587555u, 37u, 2546082054u, 4082123162u, 12u, 4u, 10u, 14u},
        {0u, 24u, 12345u, 1u, 1u, 2108418931u, 38u, 166511813u, 4112937402u, 4u, 14u, 14u, 8u},
        {0u, 24u, 4294967295u, 1u, 1u, 1819346969u, 38u, 2533458808u, 4183357089u, 14u, 7u, 4u, 15u},
        {1u, 12u, 1u, 1u, 1u, 83059500u, 4u, 1986448999u, 505731179u, 6u, 3u, 5u, 6u},
        {1u, 12u, 12345u, 1u, 1u, 1011388293u, 4u, 310018107u, 2674859499u, 3u, 7u, 6u, 4u},
        {1u, 12u, 4294967295u, 1u, 1u, 1839250178u, 4u, 1082411946u, 2685072703u, 6u, 4u, 3u, 7u},
        {0u, 12u, 1u, 2u, 1u, 1446536323u, 0u, 340896048u, 2166136261u, 6u, 3u, 5u, 6u},
        {0u, 12u, 12345u, 2u, 1u, 2523021713u, 0u, 3164128778u, 2166136261u, 3u, 7u, 6u, 4u},
        {0u, 12u, 4294967295u, 2u, 1u, 2364592492u, 0u, 689127707u, 2166136261u, 6u, 4u, 3u, 7u},
        {0u, 16u, 1u, 2u, 1u, 2386695432u, 0u, 1235057148u, 2166136261u, 9u, 4u, 7u, 10u},
        {0u, 16u, 12345u, 2u, 1u, 51968014u, 0u, 1739618415u, 2166136261u, 4u, 10u, 10u, 6u},
        {0u, 16u, 4294967295u, 2u, 1u, 2677479498u, 0u, 4047875337u, 2166136261u, 10u, 6u, 4u, 10u},
        {0u, 20u, 1u, 2u, 1u, 382618099u, 2u, 3789763070u, 526814996u, 10u, 4u, 8u, 12u},
        {0u, 20u, 12345u, 2u, 1u, 429332534u, 20u, 4292617550u, 997163840u, 4u, 12u, 12u, 6u},
        {0u, 20u, 4294967295u, 2u, 1u, 372345475u, 16u, 1461747584u, 2901758904u, 12u, 6u, 4u, 12u},
        {0u, 24u, 1u, 2u, 1u, 406587555u, 37u, 659580582u, 4082123162u, 12u, 4u, 10u, 14u},
        {0u, 24u, 12345u, 2u, 1u, 2108418931u, 38u, 795090165u, 4112937402u, 4u, 14u, 14u, 8u},
        {0u, 24u, 4294967295u, 2u, 1u, 1819346969u, 38u, 3870396776u, 4183357089u, 14u, 7u, 4u, 15u},
        {1u, 12u, 1u, 2u, 1u, 83059500u, 4u, 3888774327u, 505731179u, 6u, 3u, 5u, 6u},
        {1u, 12u, 12345u, 2u, 1u, 1011388293u, 4u, 1860129747u, 2674859499u, 3u, 7u, 6u, 4u},
        {1u, 12u, 4294967295u, 2u, 1u, 1839250178u, 4u, 1958546794u, 2685072703u, 6u, 4u, 3u, 7u},
        {0u, 12u, 1u, 6u, 1u, 1446536323u, 0u, 3900172784u, 2166136261u, 6u, 3u, 5u, 6u},
        {0u, 12u, 12345u, 6u, 1u, 2523021713u, 0u, 2651860410u, 2166136261u, 3u, 7u, 6u, 4u},
        {0u, 12u, 4294967295u, 6u, 1u, 2364592492u, 0u, 1476140507u, 2166136261u, 6u, 4u, 3u, 7u},
        {0u, 16u, 1u, 6u, 1u, 2386695432u, 0u, 1316280228u, 2166136261u, 9u, 4u, 7u, 10u},
        {0u, 16u, 12345u, 6u, 1u, 51968014u, 0u, 2203873311u, 2166136261u, 4u, 10u, 10u, 6u},
        {0u, 16u, 4294967295u, 6u, 1u, 2677479498u, 0u, 3004706977u, 2166136261u, 10u, 6u, 4u, 10u},
        {0u, 20u, 1u, 6u, 1u, 382618099u, 2u, 925953342u, 526814996u, 10u, 4u, 8u, 12u},
        {0u, 20u, 12345u, 6u, 1u, 429332534u, 20u, 315626478u, 997163840u, 4u, 12u, 12u, 6u},
        {0u, 20u, 4294967295u, 6u, 1u, 372345475u, 16u, 2384790272u, 2901758904u, 12u, 6u, 4u, 12u},
        {0u, 24u, 1u, 6u, 1u, 406587555u, 37u, 3482361422u, 4082123162u, 12u, 4u, 10u, 14u},
        {0u, 24u, 12345u, 6u, 1u, 2108418931u, 38u, 320154909u, 4112937402u, 4u, 14u, 14u, 8u},
        {0u, 24u, 4294967295u, 6u, 1u, 1819346969u, 38u, 2276010728u, 4183357089u, 14u, 7u, 4u, 15u},
        {1u, 12u, 1u, 6u, 1u, 83059500u, 4u, 329497591u, 505731179u, 6u, 3u, 5u, 6u},
        {1u, 12u, 12345u, 6u, 1u, 1011388293u, 4u, 1499427899u, 2674859499u, 3u, 7u, 6u, 4u},
        {1u, 12u, 4294967295u, 6u, 1u, 1839250178u, 4u, 1593120370u, 2685072703u, 6u, 4u, 3u, 7u},
    };
    swgb::Game game(assets);
    for(const auto &f:cases) {
        uint32_t state=f.seed;swgb::RmsMap map;
        check(swgb::generateRmsMap(scripts[f.profile],{},f.size,f.size,assets.dat(),state,map,&error,2,
                                  {uint16_t(f.civ1),uint16_t(f.civ2)}),error.c_str());
        check(state==f.finalState && map.scenery.size()==f.trees && map.ownedObjects.size()==2,
              "Original near-forest two-CC object count and RNG");
        uint32_t terrainHash=2166136261u,sceneryHash=terrainHash;
        for(auto terrain:map.terrain)terrainHash=(terrainHash^terrain)*16777619u;
        for(const auto &tree:map.scenery) {
            check(tree.alive,"Original clear-footprint forest trees remain alive");
            for(uint32_t value:{uint32_t(tree.unitId),uint32_t(tree.x*2),uint32_t(tree.y*2),uint32_t(tree.facet)})
                sceneryHash=(sceneryHash^value)*16777619u;
        }
        check(terrainHash==f.terrainHash && sceneryHash==f.sceneryHash,
              "Original near-forest terrain and retained tree order/position/facet hashes");
        const auto &first=map.ownedObjects[0];const auto &second=map.ownedObjects[1];
        check(first.owner==1 && second.owner==2 && first.unitId==109 && second.unitId==109 &&
              first.x==f.x1 && first.y==f.y1 && second.x==f.x2 && second.y==f.y2 &&
              first.pendingFoundation==255 && second.pendingFoundation==27,
              "Original near CC positions and zero-mask startup deferred foundation");
        state=f.seed;
        check(game.initRms(scripts[f.profile],{},f.size,state,&error,2,
                           {uint16_t(f.civ1),uint16_t(f.civ2)}),error.c_str());
        check(state==f.finalState && game.activeObjectCount()==f.trees+2 &&
              game.objectPlayer(f.trees+1)==1 && game.objectPlayer(f.trees+2)==2 &&
              game.objectUnitId(f.trees+1)==109 && game.objectUnitId(f.trees+2)==109,
              "Native near-forest import keeps CC IDs immediately after original Gaia creation order");
        check(game.spawnObjectForTesting(1,83,1,2.5f,2.5f)==f.trees+3,
              "Native next ID follows the two imported original CCs");
    }
}
