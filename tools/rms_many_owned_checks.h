// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
static void manyOwnedRmsMapChecks(const swgb::dat::DatFile &data) {
    const std::string source=R"(<LAND_GENERATION> base_terrain 0
        create_player_lands { terrain_type 0 base_size 3 number_of_tiles 60 }
        <OBJECTS_GENERATION> create_object 109 { number_of_objects 1 set_place_for_every_player
        min_distance_to_players 0 max_distance_to_players 0 })";
    swgb::RmsScript script;std::string error;
    check(script.load(0,[&](int32_t id,std::vector<uint8_t> &bytes) {
        if(id) return false;
        bytes.assign(source.begin(),source.end());return true;
    },{},&error),error.c_str());
    const struct { uint32_t players,seed,finalState,hash;int centers[8][2]; } cases[]={
        {3,1u,1148844240u,550507413u,{{37,8},{48,42},{8,37}}},
        {3,12345u,2332918568u,1393265781u,{{8,18},{42,22},{29,42}}},
        {3,4294967295u,2254375797u,2325623861u,{{42,19},{8,21},{32,42}}},
        {4,1u,1282756777u,550507413u,{{37,8},{42,37},{8,25},{25,42}}},
        {4,12345u,2083010985u,2185196149u,{{22,8},{42,22},{8,40},{40,42}}},
        {4,4294967295u,1557121443u,2325623861u,{{42,19},{8,43},{43,42},{19,8}}},
        {5,1u,1208922292u,550507413u,{{37,8},{39,42},{8,19},{8,46},{42,30}}},
        {5,12345u,588926631u,3237039413u,{{28,8},{42,22},{8,27},{20,42},{47,42}}},
        {5,4294967295u,2189101832u,2325623861u,{{42,19},{23,42},{42,46},{25,8},{8,30}}},
        {6,1u,4009710423u,550507413u,{{37,8},{42,47},{8,39},{27,42},{42,25},{8,17}}},
        {6,12345u,3991305305u,4272835701u,{{30,8},{42,22},{8,20},{8,42},{42,44},{30,42}}},
        {6,4294967295u,1791893704u,2325623861u,{{42,19},{33,42},{42,41},{8,23},{8,45},{27,8}}},
        {7,1u,1710568764u,550507413u,{{37,8},{42,41},{8,32},{17,42},{42,22},{15,8},{36,42}}},
        {7,12345u,3800833735u,2295456309u,{{34,8},{42,22},{15,8},{8,32},{42,41},{17,42},{36,42}}},
        {7,4294967295u,2489421672u,2325623861u,{{42,19},{39,42},{42,38},{8,16},{8,35},{31,8},{20,42}}},
        {8,1u,1182598693u,550507413u,{{37,8},{42,37},{8,42},{25,42},{42,20},{20,8},{42,42},{8,25}}},
        {8,12345u,1810512835u,3606465301u,{{39,8},{42,22},{22,8},{8,23},{42,39},{23,42},{40,42},{8,40}}},
        {8,4294967295u,1874134297u,2325623861u,{{42,19},{26,42},{43,42},{19,8},{8,26},{36,8},{8,43},{42,36}}},
    };
    for (const auto &f:cases) {
        uint32_t state=f.seed;swgb::RmsMap map;std::vector<uint16_t> civs;
        for (uint16_t owner=1;owner<=f.players;++owner) civs.push_back(owner);
        check(swgb::generateRmsMap(script,{},64,64,data,state,map,&error,f.players,civs),error.c_str());
        check(state==f.finalState && map.ownedObjects.size()==f.players,"Original3..8 owner fullstage RNG/count");
        uint32_t hash=2166136261u;
        for (auto tile:map.terrain) hash=(hash^tile)*16777619u;
        check(hash==f.hash,"Original many-owner complete terrain raster");
        for(size_t i=0;i<f.players;++i) {
            const auto &o=map.ownedObjects[i];
            check(o.owner==i+1 && o.civilization==i+1 && o.x==f.centers[i][0] && o.y==f.centers[i][1] &&
                  o.pendingFoundation==(i?27:255),"Original many-owner positions and raw pending terrain byte");
        }
    }
}
