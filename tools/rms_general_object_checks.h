// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <filesystem>
#include <fstream>
#include "rms_common_border_checks.h"
// Original 4df9b0 full runner, authored cliff/forest geometry, seed1, two owners (Empire unless paired below).
// Private rms-general-rows-*-1-1-1-oracle-2026-11-15.json records every original object.
static void rmsGeneralObjectChecks(swgb::Assets &assets) {
    rmsCommonBorderChecks(assets);
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
        {6204,3734544698u,521501797u,742396127u},
        {6204,1377182600u,950845333u,742396127u},
        {6206,3096183411u,124473770u,742396127u},
        {6202,1530472592u,374431829u,742396127u},
        {6206,3610667440u,1571899923u,742396127u},
        {6210,2307401085u,3562095874u,742396127u},
        {6206,3610667440u,1571899923u,742396127u},
        {6230,2077327384u,4170535033u,742396127u},
        {6202,3566328568u,374431829u,742396127u},
        {6206,2077327384u,1095507756u,742396127u},
        {6210,561083704u,2899244114u,742396127u},
        {6206,2077327384u,1095507756u,742396127u},
        {6279,2630457888u,422691155u,742396127u},
        {6195,3061798307u,2267873829u,742396127u},
        {6202,664476888u,864327435u,742396127u},
        {6206,4112436110u,4029094520u,742396127u},
        {6204,3529783542u,2909101468u,742396127u},
        {6206,4112436110u,1534552978u,742396127u},
        {6205,1007836572u,1455093729u,742396127u},
        {6204,3529783542u,1855488030u,742396127u},
        {6204,3529783542u,1152800464u,742396127u},
        {6202,1530472592u,374431829u,742396127u},
        {6206,4238084908u,4013083760u,742396127u},
        {6316,2174457350u,2143467900u,742396127u},
        {6336,3578249443u,164514819u,980235455u},
        {6336,3578249443u,164514819u,980235455u},
        {6322,1025370822u,2327484671u,458682992u},
        {6202,664476888u,374431829u,742396127u},
        {6202,664476888u,374431829u,742396127u},
        {6330,2120920959u,1363971012u,3073990882u},
        {6067,112705475u,2840059083u,3870201515u},
        {6077,2361844694u,730438242u,299840089u},
    };
    auto replace=[](std::string &text,const std::string &from,const std::string &to) {
        const auto at=text.find(from);check(at!=std::string::npos,"authored variation finds directive");
        text.replace(at,from.size(),to);
    };
    for(size_t variant=0;variant<std::size(cases);++variant) {
        auto changed=rows;
        std::vector<uint16_t> civilizations={1,1};
        if(variant>=37 && variant<=40)civilizations={uint16_t(1+2*(variant-37)),uint16_t(2+2*(variant-37))};
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
        if((variant>=12 && variant<=16) || variant>=50) {
            const size_t selected=variant>=50?(variant==50?12:15):variant;
            if(variant<50)replace(text,"<CLIFF_GENERATION>","");
            std::string neutral;
            std::vector<std::pair<int,int>> positions;
            if(selected==12 || selected==14)positions.push_back({50,50});
            if(selected==13 || selected==14)positions.push_back({25,25});
            if(selected==15)positions.push_back({75,28});
            if(selected==16)positions.push_back({0,0});
            for(const auto &position:positions)
                neutral+="create_land { terrain_type 6 base_size 0 number_of_tiles 0 land_position "+
                    std::to_string(position.first)+" "+std::to_string(position.second)+" land_id 7 }\n";
            const std::string stage=variant>=50?"<CLIFF_GENERATION>":"<TERRAIN_GENERATION>";
            replace(text,stage,neutral+stage);
            changed.assign(rows.begin(),rows.begin()+3);
            changed.push_back("create_object 66 { number_of_objects 5 set_tight_grouping number_of_groups 3 place_on_specific_land_id 7 min_distance_to_players 6 max_distance_to_players 12 min_distance_group_placement 3 }");
            changed.push_back("create_object 413 { number_of_objects 10 }");
        }
        if(variant>=17 && variant<=19) {
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
        if(variant>=20 && variant<=22) {
            const std::string scouts="create_object 550 { set_place_for_every_player match_player_civ number_of_objects 1 min_distance_to_players 7 max_distance_to_players 9 group_placement_radius 3 }";
            if(variant==20)changed.insert(changed.begin()+3,scouts);
            else if(variant==21)changed.push_back(scouts);
            else {
                changed.clear();unsigned nova=0;
                for(const auto &row:rows) {
                    changed.push_back(row);
                    if(row.find("create_object 66 ")==0 && (++nova==1 || nova==4))changed.push_back(scouts);
                }
            }
        }
        if(variant>=23 && variant<=32) {
            const bool scout=variant>=28;const size_t quantity=(variant-23)%5;
            const int counts[]={0,2,4,1,99};
            changed.push_back("create_object "+std::string(scout?"550 { match_player_civ ":"83 { ")+"set_place_for_every_player number_of_objects "+
                std::to_string(counts[quantity])+(quantity==3?" set_scaling_to_player_number ":" ")+
                "min_distance_to_players "+(scout?"7 max_distance_to_players 9":"6 max_distance_to_players 6")+" group_placement_radius 3 }");
        }
        if(variant==33 || variant==34) {
            for(size_t i=1;i<=2;++i)replace(changed[i],"number_of_objects 1",
                "number_of_objects "+std::to_string(variant==33?0:2));
        }
        if(variant>=35 && variant<=43) {
            const int quantity=variant==41?0:variant==42?2:variant==43?99:1;
            const std::string air="create_object 1521 { set_place_for_every_player match_player_civ number_of_objects "+std::to_string(quantity)+" min_distance_to_players 3 max_distance_to_players 6 }";
            if(variant==35 || variant==37 || variant==38)changed.insert(changed.begin()+3,air);else changed.push_back(air);
        }
        if(variant>=44 && variant<=49) {
            const char *zones[]={"zone 0","set_zone_by_team","set_zone_randomly"};
            if(variant<47)replace(text,"<CLIFF_GENERATION>","");
            replace(text,"number_of_tiles 7200",std::string("number_of_tiles 7200 ")+zones[(variant-44)%3]);
        }
        for(const auto &row:changed)text+=row+'\n';
        swgb::RmsScript script;std::string error;swgb::RmsMap map;uint32_t rng=1;
        check(script.load(60000,[&](int32_t,std::vector<uint8_t> &bytes) {
            bytes.assign(text.begin(),text.end());return true;
        },swgb::RmsScript::Limits{},&error),error.c_str());
        check(swgb::generateRmsMap(script,{"FIXED_POSITIONS","TINY_MAP","RANDOM_MAP","2_PLAYERS","DEFAULT_RESOURCES"},
            120,120,data,rng,map,&error,2,civilizations),error.c_str());
        check(map.objects.empty(),"general rows have no pre-owned records");
        swgb::Game game(assets);
        if(variant>=12) {
            uint32_t importedState=1;
            check(game.initRms(script,{"FIXED_POSITIONS","TINY_MAP","RANDOM_MAP","2_PLAYERS","DEFAULT_RESOURCES"},
                120,importedState,&error,2,civilizations),error.c_str());
            check(importedState==rng && game.activeObjectCount()==cases[variant].count,"selected-land native import count/RNG");
        }
        if(variant>=17) {
            const int ids[]={4,11,21,37,40,84,98,164,165};
            const float values[4][9]={{-2,7,2,6,1,3,725,350,375},{-5,10,2,9,1,3,875,500,375},
                {0,5,2,3,2,3,625,250,375},{-1,6,2,3,3,3,675,300,375}};
            const float quantityValues[29][2][9]={
                {{1.0f,4.0f,2.0f,3.0f,1.0f,3.0f,575.0f,200.0f,375.0f},{1.0f,4.0f,2.0f,3.0f,1.0f,3.0f,575.0f,200.0f,375.0f}},
                {{-1.0f,6.0f,2.0f,5.0f,1.0f,3.0f,675.0f,300.0f,375.0f},{-1.0f,6.0f,2.0f,5.0f,1.0f,3.0f,675.0f,300.0f,375.0f}},
                {{-3.0f,8.0f,2.0f,7.0f,1.0f,3.0f,775.0f,400.0f,375.0f},{-3.0f,8.0f,2.0f,7.0f,1.0f,3.0f,775.0f,400.0f,375.0f}},
                {{-1.0f,6.0f,2.0f,5.0f,1.0f,3.0f,675.0f,300.0f,375.0f},{-1.0f,6.0f,2.0f,5.0f,1.0f,3.0f,675.0f,300.0f,375.0f}},
                {{-16.0f,21.0f,2.0f,20.0f,1.0f,3.0f,1425.0f,1050.0f,375.0f},{-10.0f,15.0f,2.0f,14.0f,1.0f,3.0f,1125.0f,750.0f,375.0f}},
                {{1.0f,4.0f,2.0f,3.0f,1.0f,3.0f,575.0f,200.0f,375.0f},{1.0f,4.0f,2.0f,3.0f,1.0f,3.0f,575.0f,200.0f,375.0f}},
                {{-1.0f,6.0f,2.0f,3.0f,3.0f,3.0f,675.0f,300.0f,375.0f},{-1.0f,6.0f,2.0f,3.0f,3.0f,3.0f,675.0f,300.0f,375.0f}},
                {{-3.0f,8.0f,2.0f,3.0f,5.0f,3.0f,775.0f,400.0f,375.0f},{-3.0f,8.0f,2.0f,3.0f,5.0f,3.0f,775.0f,400.0f,375.0f}},
                {{-1.0f,6.0f,2.0f,3.0f,3.0f,3.0f,675.0f,300.0f,375.0f},{-1.0f,6.0f,2.0f,3.0f,3.0f,3.0f,675.0f,300.0f,375.0f}},
                {{-59.0f,64.0f,2.0f,3.0f,61.0f,3.0f,3575.0f,3200.0f,375.0f},{-16.0f,21.0f,2.0f,3.0f,18.0f,3.0f,1425.0f,1050.0f,375.0f}},
                {{5.0f,0.0f,2.0f,0.0f,0.0f,3.0f,375.0f,0.0f,375.0f},{5.0f,0.0f,2.0f,0.0f,0.0f,3.0f,375.0f,0.0f,375.0f}},
                {{1.0f,4.0f,2.0f,2.0f,2.0f,3.0f,575.0f,200.0f,375.0f},{1.0f,4.0f,2.0f,2.0f,2.0f,3.0f,575.0f,200.0f,375.0f}},
                {{0,5,2,3,2,3,735,360,375},{0,5,2,3,2,3,735,360,375}},
                {{0,5,2,3,2,3,735,360,375},{0,5,2,3,2,3,735,360,375}},
                {{0.0f,5.0f,2.0f,3.0f,2.0f,3.0f,735.0f,360.0f,375.0f},{0.0f,5.0f,3.0f,3.0f,2.0f,3.0f,735.0f,360.0f,375.0f}},
                {{0.0f,5.0f,2.0f,3.0f,2.0f,3.0f,735.0f,360.0f,375.0f},{0.0f,5.0f,2.0f,3.0f,2.0f,3.0f,735.0f,360.0f,375.0f}},
                {{0.0f,5.0f,1.0f,3.0f,2.0f,3.0f,735.0f,360.0f,375.0f},{0.0f,5.0f,2.0f,3.0f,2.0f,3.0f,735.0f,360.0f,375.0f}},
                {{0.0f,5.0f,2.0f,3.0f,2.0f,3.0f,735.0f,360.0f,375.0f},{0.0f,5.0f,2.0f,3.0f,2.0f,3.0f,735.0f,360.0f,375.0f}},
                {{1.0f,4.0f,2.0f,3.0f,1.0f,3.0f,575.0f,200.0f,375.0f},{1.0f,4.0f,2.0f,3.0f,1.0f,3.0f,575.0f,200.0f,375.0f}},
                {{-1.0f,6.0f,2.0f,3.0f,3.0f,3.0f,895.0f,520.0f,375.0f},{-1.0f,6.0f,2.0f,3.0f,3.0f,3.0f,895.0f,520.0f,375.0f}},
                {{-62.0f,67.0f,2.0f,3.0f,64.0f,3.0f,10655.0f,10280.0f,375.0f},{-50.0f,55.0f,2.0f,3.0f,52.0f,3.0f,8735.0f,8360.0f,375.0f}},
                {{1.0f,4.0f,2.0f,3.0f,1.0f,3.0f,575.0f,200.0f,375.0f},{1.0f,4.0f,2.0f,3.0f,1.0f,3.0f,575.0f,200.0f,375.0f}},
                {{1.0f,4.0f,2.0f,3.0f,1.0f,3.0f,575.0f,200.0f,375.0f},{1.0f,4.0f,2.0f,3.0f,1.0f,3.0f,575.0f,200.0f,375.0f}},
                {{1.0f,4.0f,2.0f,3.0f,1.0f,3.0f,575.0f,200.0f,375.0f},{1.0f,4.0f,2.0f,3.0f,1.0f,3.0f,575.0f,200.0f,375.0f}},
                {{1.0f,4.0f,2.0f,3.0f,1.0f,3.0f,575.0f,200.0f,375.0f},{1.0f,4.0f,2.0f,3.0f,1.0f,3.0f,575.0f,200.0f,375.0f}},
                {{1.0f,4.0f,2.0f,3.0f,1.0f,3.0f,575.0f,200.0f,375.0f},{1.0f,4.0f,2.0f,3.0f,1.0f,3.0f,575.0f,200.0f,375.0f}},
                {{1.0f,4.0f,2.0f,3.0f,1.0f,3.0f,575.0f,200.0f,375.0f},{1.0f,4.0f,2.0f,3.0f,1.0f,3.0f,575.0f,200.0f,375.0f}},
                {{1.0f,4.0f,2.0f,3.0f,1.0f,3.0f,575.0f,200.0f,375.0f},{1.0f,4.0f,2.0f,3.0f,1.0f,3.0f,575.0f,200.0f,375.0f}},
                {{1.0f,4.0f,2.0f,3.0f,1.0f,3.0f,575.0f,200.0f,375.0f},{1.0f,4.0f,2.0f,3.0f,1.0f,3.0f,575.0f,200.0f,375.0f}},
            };
            const int extra=variant>=20?2+(variant==22):int(variant==19);
            const float *expectedCounters=values[extra];
            for(int owner=1;owner<=2;++owner) {
                if(variant>=23)expectedCounters=quantityValues[variant-23][owner-1];
                check(game.populationUsedForTesting(owner)==expectedCounters[1] && game.populationCapacityForTesting(owner)==(civilizations[owner-1]==5?200:5),
                    "additional owned units preserve original population usage/capacity");
                for(size_t i=0;i<9;++i) {
                    const float actual=ids[i]==4?game.buildingPopulationCapacityForTesting(owner)-game.populationUsedForTesting(owner):
                        ids[i]==84?game.playerAttributeForTesting(owner,84):game.resource(owner,ids[i]);
                    check(actual==expectedCounters[i],("additional owned resource "+std::to_string(ids[i])+" owner "+std::to_string(owner)+
                        " got "+std::to_string(actual)+" expected "+std::to_string(expectedCounters[i])).c_str());
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
        {20,6011u,1,0,255,0.20000000298023224f,2500.0f},
        {20,6012u,2,0,27,0.20000000298023224f,2500.0f},
        {20,6013u,1,6,255,1.0f,40.0f},
        {20,6014u,1,5,255,1.0f,40.0f},
        {20,6015u,1,7,255,1.0f,40.0f},
        {20,6016u,2,7,255,1.5916622877120972f,40.0f},
        {20,6017u,2,5,255,1.0f,40.0f},
        {20,6018u,2,7,255,2.6373791694641113f,40.0f},
        {20,6019u,1,0,255,0.20000000298023224f,60.0f},
        {20,6020u,2,0,255,0.20000000298023224f,60.0f},
        {20,6021u,1,0,255,0.20000000298023224f,60.0f},
        {20,6022u,2,0,255,0.20000000298023224f,60.0f},
        {21,6011u,1,0,255,0.20000000298023224f,2500.0f},
        {21,6012u,2,0,27,0.20000000298023224f,2500.0f},
        {21,6013u,1,6,255,1.0f,40.0f},
        {21,6014u,1,5,255,1.0f,40.0f},
        {21,6015u,1,7,255,1.0f,40.0f},
        {21,6016u,2,7,255,1.5916622877120972f,40.0f},
        {21,6017u,2,5,255,1.0f,40.0f},
        {21,6018u,2,7,255,2.6373791694641113f,40.0f},
        {21,6019u,1,0,255,0.20000000298023224f,60.0f},
        {21,6020u,2,0,255,0.20000000298023224f,60.0f},
        {21,6236u,1,0,255,0.20000000298023224f,60.0f},
        {21,6237u,2,0,255,0.20000000298023224f,60.0f},
        {22,6011u,1,0,255,0.20000000298023224f,2500.0f},
        {22,6012u,2,0,27,0.20000000298023224f,2500.0f},
        {22,6013u,1,6,255,1.0f,40.0f},
        {22,6014u,1,5,255,1.0f,40.0f},
        {22,6015u,1,7,255,1.0f,40.0f},
        {22,6016u,2,7,255,1.5916622877120972f,40.0f},
        {22,6017u,2,5,255,1.0f,40.0f},
        {22,6018u,2,7,255,2.6373791694641113f,40.0f},
        {22,6019u,1,0,255,0.20000000298023224f,60.0f},
        {22,6020u,2,0,255,0.20000000298023224f,60.0f},
        {22,6046u,1,0,255,0.20000000298023224f,60.0f},
        {22,6047u,2,0,255,0.20000000298023224f,60.0f},
        {22,6108u,1,0,255,0.20000000298023224f,60.0f},
        {22,6109u,2,0,255,0.20000000298023224f,60.0f},
        {23,6011u,1,0,255,0.20000000298023224f,2500.0f},
        {23,6012u,2,0,27,0.20000000298023224f,2500.0f},
        {23,6019u,1,0,255,0.20000000298023224f,60.0f},
        {23,6020u,2,0,255,0.20000000298023224f,60.0f},
        {24,6236u,1,5,255,1.0f,40.0f},
        {24,6237u,1,4,255,1.0f,40.0f},
        {24,6238u,2,5,255,1.0f,40.0f},
        {24,6239u,2,7,255,3.5840632915496826f,40.0f},
        {25,6236u,1,5,255,1.0f,40.0f},
        {25,6239u,1,3,255,1.0f,40.0f},
        {25,6240u,2,3,255,1.0f,40.0f},
        {25,6243u,2,6,255,1.0f,40.0f},
        {26,6236u,1,5,255,1.0f,40.0f},
        {26,6237u,1,4,255,1.0f,40.0f},
        {26,6238u,2,5,255,1.0f,40.0f},
        {26,6239u,2,7,255,3.5840632915496826f,40.0f},
        {27,6236u,1,5,255,1.0f,40.0f},
        {27,6252u,1,0,255,1.0f,40.0f},
        {27,6253u,2,2,255,1.0f,40.0f},
        {27,6263u,2,4,255,1.0f,40.0f},
        {28,6011u,1,0,255,0.20000000298023224f,2500.0f},
        {28,6012u,2,0,27,0.20000000298023224f,2500.0f},
        {28,6019u,1,0,255,0.20000000298023224f,60.0f},
        {28,6020u,2,0,255,0.20000000298023224f,60.0f},
        {29,6236u,1,0,255,0.20000000298023224f,60.0f},
        {29,6237u,1,0,255,0.20000000298023224f,60.0f},
        {29,6238u,2,0,255,0.20000000298023224f,60.0f},
        {29,6239u,2,0,255,0.20000000298023224f,60.0f},
        {30,6236u,1,0,255,0.20000000298023224f,60.0f},
        {30,6239u,1,0,255,0.20000000298023224f,60.0f},
        {30,6240u,2,0,255,0.20000000298023224f,60.0f},
        {30,6243u,2,0,255,0.20000000298023224f,60.0f},
        {31,6236u,1,0,255,0.20000000298023224f,60.0f},
        {31,6237u,1,0,255,0.20000000298023224f,60.0f},
        {31,6238u,2,0,255,0.20000000298023224f,60.0f},
        {31,6239u,2,0,255,0.20000000298023224f,60.0f},
        {32,6236u,1,0,255,0.20000000298023224f,60.0f},
        {32,6295u,1,0,255,0.20000000298023224f,60.0f},
        {32,6296u,2,0,255,0.20000000298023224f,60.0f},
        {32,6312u,2,0,255,0.20000000298023224f,60.0f},
        {33,6011u,1,0,255,0.20000000298023224f,2500.0f},
        {33,6012u,2,0,27,0.20000000298023224f,2500.0f},
        {34,6011u,1,0,255,0.20000000298023224f,2500.0f},
        {34,6012u,2,0,27,0.20000000298023224f,2500.0f},
        {34,6013u,1,6,255,1.0f,40.0f},
        {34,6014u,1,5,255,1.0f,40.0f},
        {34,6015u,2,2,255,1.0f,40.0f},
        {34,6016u,2,7,255,1.5916622877120972f,40.0f},
        {34,6017u,1,0,255,0.20000000298023224f,60.0f},
        {34,6018u,1,0,255,0.20000000298023224f,60.0f},
        {34,6019u,2,0,255,0.20000000298023224f,60.0f},
        {34,6020u,2,0,255,0.20000000298023224f,60.0f},
        {35,6011u,1,0,255,0.20000000298023224f,2500.0f},
        {35,6012u,2,0,27,0.20000000298023224f,2500.0f},
        {35,6013u,1,6,255,1.0f,40.0f},
        {35,6014u,1,5,255,1.0f,40.0f},
        {35,6015u,1,7,255,1.0f,40.0f},
        {35,6016u,2,7,255,1.5916622877120972f,40.0f},
        {35,6017u,2,5,255,1.0f,40.0f},
        {35,6018u,2,7,255,2.6373791694641113f,40.0f},
        {35,6019u,1,0,255,0.20000000298023224f,60.0f},
        {35,6020u,2,0,255,0.20000000298023224f,60.0f},
        {35,6021u,1,0,255,0.8500000238418579f,150.0f},
        {35,6022u,2,3,255,0.8500000238418579f,150.0f},
        {36,6011u,1,0,255,0.20000000298023224f,2500.0f},
        {36,6012u,2,0,27,0.20000000298023224f,2500.0f},
        {36,6013u,1,6,255,1.0f,40.0f},
        {36,6014u,1,5,255,1.0f,40.0f},
        {36,6015u,1,7,255,1.0f,40.0f},
        {36,6016u,2,7,255,1.5916622877120972f,40.0f},
        {36,6017u,2,5,255,1.0f,40.0f},
        {36,6018u,2,7,255,2.6373791694641113f,40.0f},
        {36,6019u,1,0,255,0.20000000298023224f,60.0f},
        {36,6020u,2,0,255,0.20000000298023224f,60.0f},
        {36,6236u,1,4,255,0.8500000238418579f,150.0f},
        {36,6237u,2,3,255,0.8500000238418579f,150.0f},
        {37,6011u,1,0,255,0.20000000298023224f,2500.0f},
        {37,6012u,2,0,27,0.20000000298023224f,2500.0f},
        {37,6013u,1,6,255,1.0f,40.0f},
        {37,6014u,1,5,255,1.0f,40.0f},
        {37,6015u,1,7,255,1.0f,40.0f},
        {37,6016u,2,7,255,1.5916622877120972f,40.0f},
        {37,6017u,2,5,255,1.0f,40.0f},
        {37,6018u,2,7,255,2.6373791694641113f,40.0f},
        {37,6019u,1,0,255,0.20000000298023224f,60.0f},
        {37,6020u,2,2,255,0.20000000298023224f,60.0f},
        {37,6021u,1,0,255,0.8500000238418579f,150.0f},
        {37,6022u,2,5,255,0.5f,150.0f},
        {38,6011u,1,0,255,0.20000000298023224f,2500.0f},
        {38,6012u,2,0,27,0.20000000298023224f,2500.0f},
        {38,6013u,1,6,255,1.0f,40.0f},
        {38,6014u,1,5,255,1.0f,40.0f},
        {38,6015u,1,7,255,1.0f,40.0f},
        {38,6016u,2,7,255,1.5916622877120972f,40.0f},
        {38,6017u,2,5,255,1.0f,40.0f},
        {38,6018u,2,7,255,2.6373791694641113f,40.0f},
        {38,6019u,1,0,255,2.7770378589630127f,60.0f},
        {38,6020u,2,0,255,0.0f,60.0f},
        {38,6021u,1,0,255,0.8500000238418579f,150.0f},
        {38,6022u,2,1,255,0.8500000238418579f,150.0f},
        {39,6011u,1,0,255,0.20000000298023224f,2500.0f},
        {39,6012u,2,0,27,0.20000000298023224f,2500.0f},
        {39,6013u,1,6,255,1.0f,40.0f},
        {39,6014u,1,5,255,1.0f,40.0f},
        {39,6015u,1,7,255,1.0f,40.0f},
        {39,6016u,2,7,255,1.5916622877120972f,40.0f},
        {39,6017u,2,5,255,1.0f,40.0f},
        {39,6018u,2,7,255,2.6373791694641113f,40.0f},
        {39,6019u,1,0,255,2.7770378589630127f,60.0f},
        {39,6020u,2,0,255,0.2920987606048584f,60.0f},
        {39,6236u,1,4,255,0.8500000238418579f,150.0f},
        {39,6237u,2,3,255,0.8500000238418579f,150.0f},
        {40,6011u,1,0,255,0.20000000298023224f,2500.0f},
        {40,6012u,2,0,27,0.20000000298023224f,2500.0f},
        {40,6013u,1,6,255,1.0f,40.0f},
        {40,6014u,1,5,255,1.0f,40.0f},
        {40,6015u,1,7,255,1.0f,40.0f},
        {40,6016u,2,7,255,1.5916622877120972f,40.0f},
        {40,6017u,2,5,255,1.0f,40.0f},
        {40,6018u,2,7,255,2.6373791694641113f,40.0f},
        {40,6019u,1,0,255,0.20000000298023224f,60.0f},
        {40,6020u,2,0,255,0.2920987606048584f,60.0f},
        {40,6236u,1,4,255,0.8500000238418579f,150.0f},
        {40,6237u,2,3,255,0.8500000238418579f,150.0f},
        {41,6011u,1,0,255,0.20000000298023224f,2500.0f},
        {41,6012u,2,0,27,0.20000000298023224f,2500.0f},
        {41,6013u,1,6,255,1.0f,40.0f},
        {41,6014u,1,5,255,1.0f,40.0f},
        {41,6015u,1,7,255,1.0f,40.0f},
        {41,6016u,2,7,255,1.5916622877120972f,40.0f},
        {41,6017u,2,5,255,1.0f,40.0f},
        {41,6018u,2,7,255,2.6373791694641113f,40.0f},
        {41,6019u,1,0,255,0.20000000298023224f,60.0f},
        {41,6020u,2,0,255,0.20000000298023224f,60.0f},
        {42,6011u,1,0,255,0.20000000298023224f,2500.0f},
        {42,6012u,2,0,27,0.20000000298023224f,2500.0f},
        {42,6013u,1,6,255,1.0f,40.0f},
        {42,6014u,1,5,255,1.0f,40.0f},
        {42,6015u,1,7,255,1.0f,40.0f},
        {42,6016u,2,7,255,1.5916622877120972f,40.0f},
        {42,6017u,2,5,255,1.0f,40.0f},
        {42,6018u,2,7,255,2.6373791694641113f,40.0f},
        {42,6019u,1,0,255,0.20000000298023224f,60.0f},
        {42,6020u,2,0,255,0.20000000298023224f,60.0f},
        {42,6236u,1,4,255,0.8500000238418579f,150.0f},
        {42,6237u,1,0,255,0.8500000238418579f,150.0f},
        {42,6238u,2,1,255,0.8500000238418579f,150.0f},
        {42,6239u,2,3,255,0.8500000238418579f,150.0f},
        {43,6011u,1,0,255,0.20000000298023224f,2500.0f},
        {43,6012u,2,0,27,0.20000000298023224f,2500.0f},
        {43,6013u,1,6,255,1.0f,40.0f},
        {43,6014u,1,5,255,1.0f,40.0f},
        {43,6015u,1,7,255,1.0f,40.0f},
        {43,6016u,2,7,255,1.5916622877120972f,40.0f},
        {43,6017u,2,5,255,1.0f,40.0f},
        {43,6018u,2,7,255,2.6373791694641113f,40.0f},
        {43,6019u,1,0,255,0.20000000298023224f,60.0f},
        {43,6020u,2,0,255,0.20000000298023224f,60.0f},
        {43,6236u,1,4,255,0.8500000238418579f,150.0f},
        {43,6298u,1,4,255,0.8500000238418579f,150.0f},
        {43,6299u,2,4,255,0.8500000238418579f,150.0f},
        {43,6349u,2,2,255,0.8500000238418579f,150.0f},
        {44,6111u,1,0,255,0.20000000298023224f,2500.0f},
        {44,6112u,2,0,27,0.20000000298023224f,2500.0f},
        {44,6113u,1,5,255,1.0f,40.0f},
        {44,6114u,1,1,255,1.0f,40.0f},
        {44,6115u,1,6,255,1.0f,40.0f},
        {44,6116u,2,6,255,1.0f,40.0f},
        {44,6117u,2,2,255,1.0f,40.0f},
        {44,6118u,2,1,255,1.0f,40.0f},
        {44,6119u,1,0,255,0.20000000298023224f,60.0f},
        {44,6120u,2,0,255,0.20000000298023224f,60.0f},
        {45,6111u,1,0,255,0.20000000298023224f,2500.0f},
        {45,6112u,2,0,27,0.20000000298023224f,2500.0f},
        {45,6113u,1,5,255,1.0f,40.0f},
        {45,6114u,1,1,255,1.0f,40.0f},
        {45,6115u,1,6,255,1.0f,40.0f},
        {45,6116u,2,6,255,1.0f,40.0f},
        {45,6117u,2,2,255,1.0f,40.0f},
        {45,6118u,2,1,255,1.0f,40.0f},
        {45,6119u,1,0,255,0.20000000298023224f,60.0f},
        {45,6120u,2,0,255,0.20000000298023224f,60.0f},
        {46,6096u,1,0,255,0.20000000298023224f,2500.0f},
        {46,6097u,2,0,27,0.20000000298023224f,2500.0f},
        {46,6098u,1,7,255,1.0f,40.0f},
        {46,6099u,1,6,255,1.0f,40.0f},
        {46,6100u,1,1,255,1.0f,40.0f},
        {46,6101u,2,7,255,2.9559617042541504f,40.0f},
        {46,6102u,2,1,255,1.0f,40.0f},
        {46,6103u,2,7,255,1.0f,40.0f},
        {46,6104u,1,0,255,0.20000000298023224f,60.0f},
        {46,6105u,2,0,255,0.20000000298023224f,60.0f},
        {47,6011u,1,0,255,0.20000000298023224f,2500.0f},
        {47,6012u,2,0,27,0.20000000298023224f,2500.0f},
        {47,6013u,1,6,255,1.0f,40.0f},
        {47,6014u,1,5,255,1.0f,40.0f},
        {47,6015u,1,7,255,1.0f,40.0f},
        {47,6016u,2,7,255,1.5916622877120972f,40.0f},
        {47,6017u,2,5,255,1.0f,40.0f},
        {47,6018u,2,7,255,2.6373791694641113f,40.0f},
        {47,6019u,1,0,255,0.20000000298023224f,60.0f},
        {47,6020u,2,0,255,0.20000000298023224f,60.0f},
        {48,6011u,1,0,255,0.20000000298023224f,2500.0f},
        {48,6012u,2,0,27,0.20000000298023224f,2500.0f},
        {48,6013u,1,6,255,1.0f,40.0f},
        {48,6014u,1,5,255,1.0f,40.0f},
        {48,6015u,1,7,255,1.0f,40.0f},
        {48,6016u,2,7,255,1.5916622877120972f,40.0f},
        {48,6017u,2,5,255,1.0f,40.0f},
        {48,6018u,2,7,255,2.6373791694641113f,40.0f},
        {48,6019u,1,0,255,0.20000000298023224f,60.0f},
        {48,6020u,2,0,255,0.20000000298023224f,60.0f},
        {49,6147u,1,0,255,0.20000000298023224f,2500.0f},
        {49,6148u,2,0,27,0.20000000298023224f,2500.0f},
        {49,6149u,1,5,255,1.0f,40.0f},
        {49,6150u,1,0,255,1.0f,40.0f},
        {49,6151u,1,6,255,1.0f,40.0f},
        {49,6152u,2,2,255,1.0f,40.0f},
        {49,6153u,2,5,255,1.0f,40.0f},
        {49,6154u,2,7,255,1.0f,40.0f},
        {49,6155u,1,0,255,0.20000000298023224f,60.0f},
        {49,6156u,2,0,255,0.20000000298023224f,60.0f},
        {50,6071u,1,0,255,0.20000000298023224f,2500.0f},
        {50,6072u,2,0,27,0.20000000298023224f,2500.0f},
        {50,6073u,1,7,255,1.0f,40.0f},
        {50,6074u,1,2,255,1.0f,40.0f},
        {50,6075u,1,7,255,1.5872981548309326f,40.0f},
        {50,6076u,2,1,255,1.0f,40.0f},
        {50,6077u,2,1,255,1.0f,40.0f},
        {50,6078u,2,7,255,1.2029175758361816f,40.0f},
        {50,6079u,1,0,255,0.20000000298023224f,60.0f},
        {50,6080u,2,0,255,0.20000000298023224f,60.0f},
        {51,6063u,1,0,255,0.20000000298023224f,2500.0f},
        {51,6064u,2,0,27,0.20000000298023224f,2500.0f},
        {51,6065u,1,0,255,1.0f,40.0f},
        {51,6066u,1,0,255,1.0f,40.0f},
        {51,6067u,1,3,255,1.0f,40.0f},
        {51,6068u,2,3,255,1.0f,40.0f},
        {51,6069u,2,0,255,1.0f,40.0f},
        {51,6070u,2,5,255,1.0f,40.0f},
        {51,6071u,1,0,255,0.20000000298023224f,60.0f},
        {51,6072u,2,0,255,0.20000000298023224f,60.0f},
        };
        for(const auto &pose:ownedPoses)if(pose.variant==variant) {
            const int unit=game.objectUnitId(pose.id);
            const auto *graphic=data.graphic(data.civs[civilizations[pose.owner-1]].units[unit].standingGraphic[0]);
            const auto animation=game.objectAnimationForTesting(pose.id);const bool animated=graphic->sequenceType&1;
            // The original RMS harness excludes ordinary civilization bonuses; native import applies them.
            const float expectedHp=civilizations[pose.owner-1]==8 && (unit==83 || unit==293)?55:pose.hp;
            check(game.objectPlayer(pose.id)==pose.owner && game.objectHitPoints(pose.id)==expectedHp &&
                game.pendingFoundationForTesting(pose.id)==pose.pending &&
                animation[0]==(animated?pose.frame*graphic->frameDuration:0) &&
                std::abs(animation[1]-(animated?pose.hold-graphic->frameDuration:0))<.00001f,
                ("owned pose variant "+std::to_string(variant)+" id "+std::to_string(pose.id)+
                 " hp "+std::to_string(game.objectHitPoints(pose.id))+" expected "+std::to_string(expectedHp)+
                 " animation "+std::to_string(animation[0])+"/"+std::to_string(animation[1])+" expected frame/hold "+
                 std::to_string(pose.frame)+"/"+std::to_string(pose.hold)).c_str());
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
        for(const auto &o:map.resourcesAfterOwned){
            record(o.unitId,o.owner,o.facet,o.x,o.y);
            const bool air=o.owner && data.civs[civilizations[o.owner-1]].units[o.unitId].cls==59;
            if(variant>=35)check(o.heightAboveGround==(air?1.f:0.f) &&
                game.objectHeightForTesting(ordinal+1)==o.heightAboveGround,"original transport height survives native import");
            if(variant>=35 && air) {
                const auto ground=game.objectPosition(6011),air=game.objectPosition(ordinal+1);
                float gx,gy,ax,ay;
                check(game.objectScreenPosition(6011,960,544,gx,gy) && game.objectScreenPosition(ordinal+1,960,544,ax,ay),
                    "transport and ground reference project");
                const float expectedY=((air[0]+air[1]-ground[0]-ground[1])*swgb::Game::kTileHalfH-
                    data.terrainBlock.tileHalfHeight)*game.zoom();
                check(std::abs(ay-gy-expectedY)<.001f,"transport projects one half-tile above ground");
            }
            ++ordinal;
        }
        check(count==cases[variant].count && rng==cases[variant].rng && hash==cases[variant].records,
            "general object rows match complete original object order and RNG");
        auto gridHash=[](const std::vector<uint8_t> &grid) {
            uint32_t value=2166136261u;
            for(size_t i=0;i<14400;++i)value=(value^(grid.empty()?0:grid[i]))*16777619u;
            return value;
        };
        check(gridHash(map.terrain)==cases[variant].terrain && gridHash(map.elevation)==1017236165u && gridHash(map.slopes)==1017236165u,
            "general rows preserve original terrain/elevation/slopes");
        if(variant==44) {
            const auto retained=game.activeObjectCount();uint32_t unchanged=1;
            check(!game.initRms(script,{"FIXED_POSITIONS","TINY_MAP","RANDOM_MAP","2_PLAYERS","DEFAULT_RESOURCES"},
                120,unchanged,&error,2,{1,2}) && unchanged==1 && game.activeObjectCount()==retained,
                "unverified explicit-zone civilization profile preserves the running world and RNG");
        }
        if(variant==22) {
            const auto retained=game.activeObjectCount();
            for(const char *row:{"create_object 1521 { set_place_for_every_player match_player_civ number_of_objects 2 group_variance 1 min_distance_to_players 3 max_distance_to_players 6 }",
                "create_object 83 { set_place_for_every_player number_of_objects 2 group_variance 1 min_distance_to_players 6 max_distance_to_players 6 }",
                "create_object 550 { set_place_for_every_player match_player_civ number_of_objects 2 group_variance 1 min_distance_to_players 7 max_distance_to_players 9 }"}) {
                const std::string unsupported=text+row;swgb::RmsScript invalid;uint32_t unchanged=1;
                check(invalid.load(60000,[&](int32_t,std::vector<uint8_t> &bytes){bytes.assign(unsupported.begin(),unsupported.end());return true;},
                    swgb::RmsScript::Limits{},&error),error.c_str());
                check(!game.initRms(invalid,{"FIXED_POSITIONS","TINY_MAP","RANDOM_MAP","2_PLAYERS","DEFAULT_RESOURCES"},120,unchanged,&error,2,civilizations) &&
                    unchanged==1 && game.activeObjectCount()==retained && game.populationUsedForTesting(1)==6,
                    "unsupported owned-row variance preserve the running world and caller RNG");
            }
        }
    }
    std::printf("PASS: %zu general object-row variations match complete original runner\n",std::size(cases));
}
