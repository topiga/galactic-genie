// SPDX-License-Identifier: GPL-3.0-or-later
#include "engine/rms_object_rows.h"
#include <array>
#include <cassert>
#include <iostream>
#include <sstream>
using namespace swgb;
static std::vector<RmsToken> tokens(const std::string &text) {
    std::vector<RmsToken> result;
    std::istringstream input(text);
    for(std::string word;input>>word;) result.push_back({word,54103,1});
    return result;
}
static std::array<int32_t,15> words(const RmsObjectRow &r) {
    return {r.unitId,r.terrain,int32_t(r.mode)|(int32_t(r.scaling)<<8),r.perGroup,r.variance,
        r.groups,r.radius,r.owner,r.landId,r.minDistance,r.maxDistance,r.spacing,
        r.temporarySpacing,r.zoneDistance,r.matchPlayerCiv};
}
static RmsObjectRow parse(const std::string &text) {
    auto input=tokens(text);size_t index=0;RmsObjectRow row;std::string error;
    assert(parseRmsObjectRow(input,index,row,&error) && error.empty() && index==input.size());
    return row;
}
#include "rms_object_dispatch_checks.h"
int main() {
    checkRmsObjectDispatch();
    // Original handler oracle rms-object-parser-oracle-2026-11-13.json.
    assert((words(parse("109 { }"))==std::array<int32_t,15>{109,-1,0,1,0,1,3,-1,-1,-1,-1,0,0,0,0}));
    assert((words(parse("109 { set_place_for_every_player group_placement_radius 18 min_distance_to_players 0 max_distance_to_players 0 }"))==
        std::array<int32_t,15>{109,-1,0,1,0,1,18,-1,1,0,0,0,0,0,0}));
    assert((words(parse("66 { number_of_objects 7 group_placement_radius 3 set_tight_grouping set_gaia_object_only set_place_for_every_player min_distance_to_players 12 max_distance_to_players 16 min_distance_group_placement 7 max_distance_to_other_zones 7 }"))==
        std::array<int32_t,15>{66,-1,2,7,0,1,3,0,1,12,16,7,0,7,0}));
    assert(words(parse("348 { number_of_objects 7 number_of_groups 3 }"))==
           words(parse("348 { number_of_groups 3 number_of_objects 7 }")));
    auto row=parse("348 { number_of_objects 7 number_of_groups 3 number_of_groups 5 set_tight_grouping set_loose_grouping }");
    assert(row.mode==1 && row.perGroup==7 && row.groups==5);
    row=parse("348 { number_of_objects 7 set_tight_grouping number_of_objects 9 set_loose_grouping }");
    assert(row.mode==1 && row.perGroup==9 && row.groups==1);
    row=parse("285 { number_of_objects 5 min_distance_to_players 25 temp_min_distance_group_placement 20 }");
    assert((words(row)==std::array<int32_t,15>{285,-1,0,1,0,5,3,-1,-2,25,-1,0,20,0,0}));
    row=parse("550 { terrain_to_place_on 22 group_variance 2 set_scaling_to_map_size set_scaling_to_player_number match_player_civ set_gaia_object_only place_on_specific_land_id 25 max_distance_to_players 9 }");
    assert(row.terrain==22 && row.variance==2 && row.scaling==2 && row.matchPlayerCiv==1 &&
           row.owner==0 && row.landId==35 && row.minDistance==-1 && row.maxDistance==9);
    row=parse("550 { min_distance_to_players 7 place_on_specific_land_id 25 set_place_for_every_player max_distance_to_players -1 min_distance_to_players +0 }");
    assert(row.landId==1 && row.minDistance==0 && row.maxDistance==-1);
    row=parse("550 { max_distance_to_players -1 }");
    assert(row.landId==-2);
    row=parse("550 { place_on_specific_land_id 2147483637 }");
    assert(row.landId==2147483647);
    row=parse("350 { number_of_objects 30 set_scaling_to_player_number set_scaling_to_map_size }");
    RmsObjectRow scaled;std::string error;
    {
        // Original55019's unresolved ANIMAL_SAARLAC leaves its block on the Womprat row.
        auto previous=parse("1475 { number_of_objects 6 number_of_groups 1 group_variance 2 set_gaia_object_only min_distance_to_players 10 set_place_for_every_player }");
        auto input=tokens("{ number_of_objects 1 set_gaia_object_only min_distance_to_players 10 }");size_t index=0;
        assert(parseRmsObjectRow(input,index,previous,&error) && index==input.size());
        assert((words(previous)==std::array<int32_t,15>{1475,-1,1,1,2,1,3,0,1,10,-1,0,0,0,0}));
        input=tokens("{ number_of_objects 5 group_variance -1 }");index=0;const auto before=words(previous);
        assert(!parseRmsObjectRow(input,index,previous,&error) && index==0 && words(previous)==before);
        input=tokens("{ }");index=0;RmsObjectRow missing;
        assert(!parseRmsObjectRow(input,index,missing,&error) && index==0 && missing.unitId==-1);
    }
    assert(finalizeRmsObjectRow(row,120,120,2,scaled,&error));
    assert(scaled.groups==43 && scaled.perGroup==1 && scaled.mode==0 && scaled.scaling==1);
    row=parse("350 { number_of_objects 30 set_scaling_to_player_number }");
    assert(finalizeRmsObjectRow(row,120,120,2,scaled,&error) && scaled.groups==60);
    assert(finalizeRmsObjectRow(row,120,120,0,scaled,&error) && scaled.groups==1);
    row=parse("350 { number_of_objects 0 }");
    assert(finalizeRmsObjectRow(row,120,120,2,scaled,&error) && scaled.groups==0);
    row.scaling=1;
    assert(finalizeRmsObjectRow(row,1,1,2,scaled,&error) && scaled.groups==1);
    row=parse("350 { number_of_objects 7 set_loose_grouping set_scaling_to_map_size }");
    assert(finalizeRmsObjectRow(row,200,200,2,scaled,&error) && scaled.groups==4 && scaled.perGroup==7);
    for(const auto &bad: {"-1 { }","32768 { }","109", "109 {", "109 { set_scale_by_groups }",
        "109 { number_of_objects -1 }", "109 { number_of_groups 2147483648 }",
        "109 { group_variance -1 }", "109 { group_placement_radius -1 }",
        "109 { min_distance_to_players -2 }", "109 { max_distance_to_players -2 }",
        "109 { min_distance_group_placement -1 }", "109 { temp_min_distance_group_placement -1 }",
        "109 { max_distance_to_other_zones -1 }", "109 { terrain_to_place_on 256 }",
        "109 { terrain_to_place_on -2 }", "109 { terrain_to_place_on +-1 }",
        "109 { place_on_specific_land_id 2147483638 }", "109 { place_on_specific_land_id -1 }",
        "109 { number_of_objects 2x }", "109 { number_of_objects }"}) {
        auto input=tokens(bad);size_t index=0;RmsObjectRow unchanged;unchanged.groups=77;
        assert(!parseRmsObjectRow(input,index,unchanged,&error) && !error.empty());
        assert(index==0 && unchanged.groups==77 && unchanged.unitId==-1);
    }
    auto input=tokens("ignored 109 { } trailing");size_t index=1;
    assert(parseRmsObjectRow(input,index,row,&error) && index==4 && row.unitId==109);
    index=99;assert(!parseRmsObjectRow(input,index,row,&error) && index==99);
    row=parse("350 { number_of_objects 2147483647 set_scaling_to_map_size }");
    scaled.groups=77;
    assert(!finalizeRmsObjectRow(row,120,120,2,scaled,&error) && scaled.groups==77);
    row.groups=0;
    assert(!finalizeRmsObjectRow(row,65536,65536,2,scaled,&error) && scaled.groups==77);
    row.scaling=2;row.groups=2147483647;
    assert(!finalizeRmsObjectRow(row,120,120,2,scaled,&error) && scaled.groups==77);
    row.groups=1;
    assert(!finalizeRmsObjectRow(row,0,120,2,scaled,&error));
    assert(!finalizeRmsObjectRow(row,120,120,32767,scaled,&error));
    std::cout<<"RMS object rows passed\n";
}
