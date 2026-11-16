// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <filesystem>
#include <fstream>
// Original 4df9b0 full runner, authored cliff/forest geometry, seed1, two Empire owners.
// Private rms-general-rows-*-1-1-1-oracle-2026-11-15.json records every original object.
static void rmsGeneralObjectChecks(swgb::Assets &assets) {
    const auto &data=assets.dat();
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
    const struct {uint32_t count,rng,records,terrain=742396127u;} cases[]={
        {6203,4194478548u,780067774u}, {6206,4166862475u,3404087702u},
        {6210,885130658u,1156138209u}, {6203,502114030u,4034934607u},
        {6219,170179477u,316708715u}, {6217,2876734422u,1883431379u},
        {6206,1396042352u,2825504220u}, {6203,327591995u,467029569u},
        {6205,1003759820u,2218478092u},
        {6213,1186593183u,3238364985u},
        {6221,707435866u,2827731819u},
        {6202,664476888u,374431829u},
        {6144,472243236u,535885288u,2643218578u},
        {6144,101811930u,3017554391u,2643218578u},
        {6157,2212104137u,1995093907u,726499856u},
        {6145,778134838u,1629751366u,980235455u},
        {6147,1759471658u,3491954245u,398166423u},
        {6209,1555457656u,1303879948u,742396127u},
        {6208,1935763599u,3019999758u,742396127u},
        {6215,3226277433u,3720278662u,742396127u},
    };
    auto replace=[](std::string &text,const std::string &from,const std::string &to) {
        const auto at=text.find(from);check(at!=std::string::npos,"authored variation finds directive");
        text.replace(at,from.size(),to);
    };
    for(size_t variant=0;variant<20;++variant) {
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
        case 11: changed.insert(changed.begin()+4,"create_object 66 { number_of_objects 7 place_on_specific_land_id 7 }");break;
        }
        std::string text=source.substr(0,first);
        if(variant>=12 && variant<=16) {
            replace(text,"<CLIFF_GENERATION>","");
            std::string neutral;
            std::vector<std::pair<int,int>> positions;
            if(variant==12 || variant==14)positions.push_back({50,50});
            if(variant==13 || variant==14)positions.push_back({25,25});
            if(variant==15)positions.push_back({75,28});
            if(variant==16)positions.push_back({0,0});
            for(const auto &position:positions)
                neutral+="create_land { terrain_type 6 base_size 0 number_of_tiles 0 land_position "+
                    std::to_string(position.first)+" "+std::to_string(position.second)+" land_id 7 }\n";
            replace(text,"<TERRAIN_GENERATION>",neutral+"<TERRAIN_GENERATION>");
            changed.assign(rows.begin(),rows.begin()+3);
            changed.push_back("create_object 66 { number_of_objects 5 set_tight_grouping number_of_groups 3 place_on_specific_land_id 7 min_distance_to_players 6 max_distance_to_players 12 min_distance_group_placement 3 }");
            changed.push_back("create_object 413 { number_of_objects 10 }");
        }
        if(variant>=17) {
            const std::string workers="create_object 83 { set_place_for_every_player number_of_objects 1 min_distance_to_players 6 max_distance_to_players 6 group_placement_radius 3 }";
            if(variant==17)changed.insert(changed.begin()+3,workers);
            else if(variant==18)changed.push_back(workers);
            else {
                changed.clear();unsigned nova=0;
                for(const auto &row:rows) {
                    changed.push_back(row);
                    if(row.find("create_object 66 ")==0 && (++nova==1 || nova==4))changed.push_back(workers);
                }
            }
        }
        for(const auto &row:changed)text+=row+'\n';
        swgb::RmsScript script;std::string error;swgb::RmsMap map;uint32_t rng=1;
        check(script.load(60000,[&](int32_t,std::vector<uint8_t> &bytes) {
            bytes.assign(text.begin(),text.end());return true;
        },swgb::RmsScript::Limits{},&error),error.c_str());
        check(swgb::generateRmsMap(script,{"FIXED_POSITIONS","TINY_MAP","RANDOM_MAP","2_PLAYERS","DEFAULT_RESOURCES"},
            120,120,data,rng,map,&error,2,{1,1}),error.c_str());
        check(map.objects.empty(),"general rows have no pre-owned records");
        swgb::Game game(assets);
        if(variant>=12) {
            uint32_t importedState=1;
            check(game.initRms(script,{"FIXED_POSITIONS","TINY_MAP","RANDOM_MAP","2_PLAYERS","DEFAULT_RESOURCES"},
                120,importedState,&error,2,{1,1}),error.c_str());
            check(importedState==rng && game.activeObjectCount()==cases[variant].count,"selected-land native import count/RNG");
        }
        if(variant>=17) {
            const int ids[]={4,11,21,37,40,84,98,164,165};
            const float values[2][9]={{-2,7,2,6,1,3,725,350,375},{-5,10,2,9,1,3,875,500,375}};
            const int extra=variant==19;
            for(int owner=1;owner<=2;++owner) {
                check(game.populationUsedForTesting(owner)==(extra?10:7) && game.populationCapacityForTesting(owner)==5,
                    "additional workers preserve original population usage/capacity");
                for(size_t i=0;i<9;++i) {
                    const float actual=ids[i]==4?game.populationCapacityForTesting(owner)-game.populationUsedForTesting(owner):
                        ids[i]==84?game.playerAttributeForTesting(owner,84):game.resource(owner,ids[i]);
                    check(actual==values[extra][i],("additional worker resource "+std::to_string(ids[i])+" owner "+std::to_string(owner)+
                        " got "+std::to_string(actual)+" expected "+std::to_string(values[extra][i])).c_str());
                }
            }
        }
        const struct {size_t variant;uint32_t id;int owner,frame,pending;float hold,hp;} ownedPoses[]={
        {17,6011u,1,0,255,0.20000000298023224f,2500.0f},
        {17,6012u,2,0,27,0.20000000298023224f,2500.0f},
        {17,6013u,1,6,255,1.0f,40.0f},
        {17,6014u,1,5,255,1.0f,40.0f},
        {17,6015u,1,7,255,1.0f,40.0f},
        {17,6016u,2,7,255,1.5916622877120972f,40.0f},
        {17,6017u,2,5,255,1.0f,40.0f},
        {17,6018u,2,7,255,2.6373791694641113f,40.0f},
        {17,6019u,1,0,255,0.20000000298023224f,60.0f},
        {17,6020u,2,0,255,0.20000000298023224f,60.0f},
        {17,6021u,1,7,255,3.294015407562256f,40.0f},
        {17,6022u,1,5,255,1.0f,40.0f},
        {17,6023u,1,5,255,1.0f,40.0f},
        {17,6024u,2,1,255,1.0f,40.0f},
        {17,6025u,2,7,255,1.0f,40.0f},
        {17,6026u,2,4,255,1.0f,40.0f},
        {18,6011u,1,0,255,0.20000000298023224f,2500.0f},
        {18,6012u,2,0,27,0.20000000298023224f,2500.0f},
        {18,6013u,1,6,255,1.0f,40.0f},
        {18,6014u,1,5,255,1.0f,40.0f},
        {18,6015u,1,7,255,1.0f,40.0f},
        {18,6016u,2,7,255,1.5916622877120972f,40.0f},
        {18,6017u,2,5,255,1.0f,40.0f},
        {18,6018u,2,7,255,2.6373791694641113f,40.0f},
        {18,6019u,1,0,255,0.20000000298023224f,60.0f},
        {18,6020u,2,0,255,0.20000000298023224f,60.0f},
        {18,6236u,1,5,255,1.0f,40.0f},
        {18,6237u,1,4,255,1.0f,40.0f},
        {18,6238u,1,2,255,1.0f,40.0f},
        {18,6239u,2,1,255,1.0f,40.0f},
        {18,6240u,2,3,255,1.0f,40.0f},
        {18,6241u,2,7,255,1.0f,40.0f},
        {19,6011u,1,0,255,0.20000000298023224f,2500.0f},
        {19,6012u,2,0,27,0.20000000298023224f,2500.0f},
        {19,6013u,1,6,255,1.0f,40.0f},
        {19,6014u,1,5,255,1.0f,40.0f},
        {19,6015u,1,7,255,1.0f,40.0f},
        {19,6016u,2,7,255,1.5916622877120972f,40.0f},
        {19,6017u,2,5,255,1.0f,40.0f},
        {19,6018u,2,7,255,2.6373791694641113f,40.0f},
        {19,6019u,1,0,255,0.20000000298023224f,60.0f},
        {19,6020u,2,0,255,0.20000000298023224f,60.0f},
        {19,6046u,1,4,255,1.0f,40.0f},
        {19,6047u,1,4,255,1.0f,40.0f},
        {19,6048u,1,7,255,1.3966184854507446f,40.0f},
        {19,6049u,2,7,255,1.0f,40.0f},
        {19,6050u,2,3,255,1.0f,40.0f},
        {19,6051u,2,5,255,1.0f,40.0f},
        {19,6112u,1,1,255,1.0f,40.0f},
        {19,6113u,1,1,255,1.0f,40.0f},
        {19,6114u,1,6,255,1.0f,40.0f},
        {19,6115u,2,4,255,1.0f,40.0f},
        {19,6116u,2,4,255,1.0f,40.0f},
        {19,6117u,2,6,255,1.0f,40.0f},
        };
        for(const auto &pose:ownedPoses)if(pose.variant==variant) {
            const int unit=game.objectUnitId(pose.id);
            const auto *graphic=data.graphic(data.civs[1].units[unit].standingGraphic[0]);
            const auto animation=game.objectAnimationForTesting(pose.id);const bool animated=graphic->sequenceType&1;
            check(game.objectPlayer(pose.id)==pose.owner && game.objectHitPoints(pose.id)==pose.hp &&
                game.pendingFoundationForTesting(pose.id)==pose.pending &&
                animation[0]==(animated?pose.frame*graphic->frameDuration:0) &&
                std::abs(animation[1]-(animated?pose.hold-graphic->frameDuration:0))<.00001f,
                "interleaved owned startup preserves original ownership, health and animation");
        }
        uint32_t ordinal=0,count=0,hash=2166136261u;
        auto record=[&](int unit,int owner,int facet,float x,float y) {
            for(uint32_t word:{ordinal,uint32_t(owner),uint32_t(unit),uint32_t(facet),uint32_t(x*2),uint32_t(y*2)})hash=(hash^word)*16777619u;
            if(variant>=12) {
                const uint32_t id=ordinal+1;const auto position=game.objectPosition(id);
                check(game.objectUnitId(id)==unit && game.objectPlayer(id)==owner && position[0]==y && position[1]==120-x,
                    "selected-land native IDs, ownership and positions");
            }
            ++count;
        };
        for(const auto &o:map.cliffs){ordinal=o.creationOrdinal;record(o.unit,0,o.facet,o.x,o.y);++ordinal;}
        for(const auto &o:map.scenery){if(o.alive)record(o.unitId,0,o.facet,o.x,o.y);++ordinal;}
        for(const auto &o:map.ownedObjects){record(o.unitId,o.owner,o.facet,o.x,o.y);++ordinal;}
        for(const auto &o:map.resourcesAfterOwned){record(o.unitId,o.owner,o.facet,o.x,o.y);++ordinal;}
        check(count==cases[variant].count && rng==cases[variant].rng && hash==cases[variant].records,
            "general object rows match complete original object order and RNG");
        auto gridHash=[](const std::vector<uint8_t> &grid) {
            uint32_t value=2166136261u;
            for(size_t i=0;i<14400;++i)value=(value^(grid.empty()?0:grid[i]))*16777619u;
            return value;
        };
        check(gridHash(map.terrain)==cases[variant].terrain && gridHash(map.elevation)==1017236165u && gridHash(map.slopes)==1017236165u,
            "general rows preserve original terrain/elevation/slopes");
    }
    std::puts("PASS: twenty general object-row variations match complete original runner");
}
