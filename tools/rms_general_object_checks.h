// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <filesystem>
#include <fstream>
// Original 4df9b0 full runner, authored cliff/forest geometry, seed1, two Empire owners.
// Private rms-general-rows-*-1-1-1-oracle-2026-11-15.json records every original object.
static void rmsGeneralObjectChecks(const swgb::dat::DatFile &data) {
    std::ifstream input(std::filesystem::path(__FILE__).parent_path()/"fixtures/cliff-full-start.rms");
    check(bool(input),"authored general-object fixture opens");
    const std::string source((std::istreambuf_iterator<char>(input)),{});
    const auto section=source.find("<OBJECTS_GENERATION>");
    check(section!=std::string::npos,"authored object section exists");
    const auto first=source.find("create_object",section);
    std::vector<std::string> rows;
    for(auto start=first;start!=std::string::npos;) {
        const auto end=source.find('}',start);
        check(end!=std::string::npos,"authored object block closes");
        rows.push_back(source.substr(start,end-start+1));
        start=source.find("create_object",end+1);
    }
    check(rows.size()==23,"authored fixture retains23 rows");
    const struct {uint32_t count,rng,records;} cases[]={
        {6203,4194478548u,780067774u}, {6206,4166862475u,3404087702u},
        {6210,885130658u,1156138209u}, {6203,502114030u,4034934607u},
        {6219,170179477u,316708715u}, {6217,2876734422u,1883431379u},
        {6206,1396042352u,2825504220u}, {6203,327591995u,467029569u},
        {6205,1003759820u,2218478092u},
        {6213,1186593183u,3238364985u},
        {6221,707435866u,2827731819u},
    };
    auto replace=[](std::string &text,const std::string &from,const std::string &to) {
        const auto at=text.find(from);check(at!=std::string::npos,"authored variation finds directive");
        text.replace(at,from.size(),to);
    };
    for(size_t variant=0;variant<11;++variant) {
        auto changed=rows;
        switch(variant) {
        case 0: std::reverse(changed.begin()+4,changed.end());break;
        case 1:
            replace(changed[5],"number_of_objects 7","number_of_objects 5");
            replace(changed[8],"number_of_objects 5","number_of_objects 9");break;
        case 2:
            changed.insert(changed.begin()+4,{"create_object 66 { number_of_objects 3 }",
                "create_object 102 { number_of_objects 4 number_of_groups 2 number_of_objects 3 set_tight_grouping min_distance_to_players 20 }"});break;
        case 3:
            replace(changed[5],"max_distance_to_other_zones 7","max_distance_to_other_zones 5");
            replace(changed[5],"min_distance_group_placement 7","min_distance_group_placement 3 temp_min_distance_group_placement 9");break;
        case 4: changed.insert(changed.begin()+4,"create_object 66 { number_of_objects 6 set_loose_grouping group_placement_radius 4 terrain_to_place_on 6 number_of_groups 3 }");break;
        case 5: changed.insert(changed.begin()+4,"create_object 413 { number_of_objects 3 set_loose_grouping group_variance 2 group_placement_radius 2 number_of_groups 5 min_distance_to_players 8 }");break;
        case 6: changed.insert(changed.begin()+4,"create_object 126 { number_of_objects 2 set_scaling_to_player_number }");break;
        case 7: changed.erase(changed.begin()+3);changed.push_back(rows[3]);break;
        case 8: changed.insert(changed.begin()+4,"create_object 66 { number_of_objects 5 max_distance_to_other_zones 7 }");break;
        case 9: changed.insert(changed.begin()+4,"create_object 66 { number_of_objects 4 set_tight_grouping number_of_groups 3 min_distance_to_players 20 max_distance_to_other_zones 7 }");break;
        case 10: changed.insert(changed.begin()+4,"create_object 413 { number_of_objects 20 max_distance_to_other_zones 15 }");break;
        }
        std::string text=source.substr(0,first);
        for(const auto &row:changed)text+=row+'\n';
        swgb::RmsScript script;std::string error;swgb::RmsMap map;uint32_t rng=1;
        check(script.load(60000,[&](int32_t,std::vector<uint8_t> &bytes) {
            bytes.assign(text.begin(),text.end());return true;
        },swgb::RmsScript::Limits{},&error),error.c_str());
        check(swgb::generateRmsMap(script,{"FIXED_POSITIONS","TINY_MAP","RANDOM_MAP","2_PLAYERS","DEFAULT_RESOURCES"},
            120,120,data,rng,map,&error,2,{1,1}),error.c_str());
        check(map.objects.empty(),"general rows have no pre-owned records");
        uint32_t ordinal=0,count=0,hash=2166136261u;
        auto record=[&](int unit,int owner,int facet,float x,float y) {
            for(uint32_t word:{ordinal,uint32_t(owner),uint32_t(unit),uint32_t(facet),uint32_t(x*2),uint32_t(y*2)})hash=(hash^word)*16777619u;
            ++count;
        };
        for(const auto &o:map.cliffs){ordinal=o.creationOrdinal;record(o.unit,0,o.facet,o.x,o.y);++ordinal;}
        for(const auto &o:map.scenery){if(o.alive)record(o.unitId,0,o.facet,o.x,o.y);++ordinal;}
        for(const auto &o:map.ownedObjects){record(o.unitId,o.owner,o.facet,o.x,o.y);++ordinal;}
        for(const auto &o:map.resourcesAfterOwned){record(o.unitId,0,o.facet,o.x,o.y);++ordinal;}
        check(count==cases[variant].count && rng==cases[variant].rng && hash==cases[variant].records,
            "general object rows match complete original object order and RNG");
        auto gridHash=[](const std::vector<uint8_t> &grid) {
            uint32_t value=2166136261u;
            for(size_t i=0;i<14400;++i)value=(value^(grid.empty()?0:grid[i]))*16777619u;
            return value;
        };
        check(gridHash(map.terrain)==742396127u && gridHash(map.elevation)==1017236165u && gridHash(map.slopes)==1017236165u,
            "general rows preserve original terrain/elevation/slopes");
    }
    std::puts("PASS: eleven general object-row variations match complete original runner");
}
