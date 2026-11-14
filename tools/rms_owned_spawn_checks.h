// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
static void ownedRmsSpawnChecks(swgb::Assets &assets,swgb::SoftRenderer &renderer) {
    const std::string source=R"(<LAND_GENERATION> base_terrain 0
        create_player_lands { terrain_type 0 base_size 3 number_of_tiles 60 }
        <OBJECTS_GENERATION> create_object 109 {
            number_of_objects 1 set_place_for_every_player
            min_distance_to_players 0 max_distance_to_players 0 })";
    swgb::RmsScript script; std::string error;
    check(script.load(0,[&](int32_t id,std::vector<uint8_t> &bytes) {
        if (id!=0) return false;
        bytes.assign(source.begin(),source.end()); return true;
    },{},&error),error.c_str());
    swgb::Game game(assets);
    // Original full object stage, preceded by the verified32x32 bare-land pipeline.
    const uint32_t seeds[]={1u,12345u,UINT32_MAX};
    const uint32_t finalStates[]={2036296550u,2943925683u,764690905u};
    const uint32_t hashes[][3]={{2282508853u,1877077813u,2491377461u},
        {470130693u,461407749u,3220284933u},{2078377669u,3067649733u,3863289541u}};
    const float xs[]={18,20,20},ys[]={5,11,10};
    for(uint16_t civ=1;civ<=8;++civ) for(size_t fixture=0;fixture<3;++fixture) {
        const uint32_t seed=seeds[fixture];
        uint32_t state=seed;
        swgb::RmsMap map;
        check(swgb::generateRmsMap(script,{},32,32,assets.dat(),state,map,&error,1,{civ}),error.c_str());
        check(map.ownedObjects.size()==1 && map.objects.empty() && map.scenery.empty(),
              "Bounded RMS creates exactly one owned Command Center");
        const auto &center=map.ownedObjects[0];
        uint32_t hash=2166136261u;
        for(uint8_t tile:map.terrain) hash=(hash^tile)*16777619u;
        check(state==finalStates[fixture] && center.x==xs[fixture] && center.y==ys[fixture] &&
              hash==hashes[civ==2?1:civ==6?2:0][fixture],
              "Original full owned object stage position/foundation/RNG fixture");
        const uint32_t expectedState=state;
        state=seed;
        check(game.initRms(script,{},32,state,&error,1,{civ}),error.c_str());
        check(state==expectedState && game.activeObjectCount()==1 &&
              game.objectPlayer(1)==1 && game.objectUnitId(1)==109 &&
              game.objectHitPoints(1)==2500 && game.underConstructionObjectCount()==0,
              "Owned RMS import preserves owner, ID, completed state, HP and RNG");
        const auto position=game.objectPosition(1);
        check(position[0]==center.x && position[1]==center.y,
              "Owned RMS import retains integer center coordinates");
        for(int y=0;y<32;++y) for(int x=0;x<32;++x)
            check(game.terrainAtForTesting(x,y)==map.terrain[size_t(y)*32+x],
                  "Owned RMS import preserves the final foundation raster");
        const int unlock[]={451,452,454,453,-1,455,387,388};
        std::vector<int> order={0,486};
        if(civ==2) order.push_back(493);
        order.push_back(22);
        if(unlock[civ-1]>=0) order.push_back(unlock[civ-1]);
        check(game.technologyOrderForTesting(1)==order,
              "Owned RMS import preserves original civilization research chronology");
        check(game.populationCapacityForTesting(1)==(civ==5?200.f:5.f) &&
              game.populationUsedForTesting(1)==0 && game.unitAvailableForTesting(1,70)==(civ!=5),
              "Owned RMS population and housing follow post-map civilization bonuses");
        check(game.resource(1,98)==375 && game.resource(1,165)==375 &&
              game.resource(1,0)==0 && game.resource(1,1)==0 && game.resource(1,2)==0 && game.resource(1,3)==0,
              "Completed RMS building records cost without charging starting resources");
        check(game.selectObjectForTesting(1),"Owned RMS Command Center is inspectable");
        for(const auto &invalid:std::vector<std::vector<uint16_t>>{{},{0},{9},{1,9}}) {
            const auto previous=state;
            check(!game.initRms(script,{},32,state,&error,uint32_t(invalid.size()==2?2:1),invalid) &&
                  state==previous && game.activeObjectCount()==1 && game.objectSelected(1),
                  "Unsupported owned setup preserves the current game and RNG");
        }
    }
    for (uint16_t civ=1;civ<=8;++civ) {
        uint32_t state=1;
        check(game.initRms(script,{},32,state,&error,2,{1,civ}),error.c_str());
        check(state==1410736551u && game.activeObjectCount()==2 && game.objectPlayer(2)==2 &&
              game.objectUnitId(2)==109 && game.objectHitPoints(2)==2500,
              "Two owners retain original creation order and independent completed buildings");
        check(game.populationCapacityForTesting(2)==(civ==5?200.f:5.f) &&
              game.populationUsedForTesting(2)==0 && game.resource(2,98)==375 && game.resource(2,165)==375,
              "Second owner receives its own civilization, population and cost accounting");
        check(!game.objectVisibleForTesting(2),"Separate owners start as enemies with no shared sight");
        game.lookAt(13,20);
        game.render(renderer,960,544);
        check(game.terrainAtForTesting(13,20)==0,"Rendering unseen enemy retains pending foundation");
        const auto scout=game.spawnObjectForTesting(1,83,1,13,20);
        check(scout && game.objectVisibleForTesting(2),"Local scout reveals enemy center");
        swgb::InputState idle;
        game.updatePaused(0,idle);
        check(game.terrainAtForTesting(13,20)==0,"Sight and paused update do not consume deferred terrain");
        check(game.moveObjectForTesting(scout,2,2),"Leave enemy in explored fog");
        game.render(renderer,960,544);
        check(game.terrainAtForTesting(13,20)==0,"Remembered enemy rendering does not consume deferred terrain");
        check(game.moveObjectForTesting(scout,13,20),"Reveal enemy again");
        game.lookAt(-1000,-1000);
        game.render(renderer,960,544);
        check(game.terrainAtForTesting(13,20)==0,"Off-camera visible enemy retains pending terrain");
        // Center tile is outside this96x48 viewport but inside the original padded scan.
        game.lookAt(18,20);
        game.render(renderer,96,48);
        const int foundation=civ==2?4:civ==6?6:27;
        for(int y=18;y<22;++y) for(int x=11;x<15;++x)
            check(game.terrainAtForTesting(x,y)==foundation,"Padded terrain dispatch consumes deferred16tile foundation despite clipped pixels");
        check(game.setTerrainForTesting(13,20,0),"Replace painted terrain");
        game.render(renderer,960,544);
        check(game.terrainAtForTesting(13,20)==0,"Deferred foundation is consumed exactly once");
    }
    std::puts("PASS: owned RMS Command Center import, civilization order, population and cost accounting");
}
