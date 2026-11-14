// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <algorithm>
#include <string>
// Authored stock-profile syntax shared by end-to-end original startup checks.
static std::string rmsStockResourceSource(bool forest,int resourceRows=6) {
  std::string source=std::string("<LAND_GENERATION> base_terrain ")+(forest?"41":"0")+
    " create_player_lands { terrain_type "+(forest?"9":"0")+" base_size 3 number_of_tiles 60 } ";
  if(forest) source+="<TERRAIN_GENERATION> create_terrain 10 { base_terrain 41 number_of_tiles 37 number_of_clumps 4 spacing_to_other_terrain_types 1 set_avoid_player_start_areas } create_terrain 6 { base_terrain 9 number_of_tiles 9 number_of_clumps 2 clumping_factor 6 } ";
  source+=R"(<OBJECTS_GENERATION>
    create_object 109 { number_of_objects 1 set_place_for_every_player min_distance_to_players 0 max_distance_to_players 0 }
    create_object 83 { number_of_objects 1 set_place_for_every_player min_distance_to_players 6 max_distance_to_players 6 }
    create_object 550 { set_place_for_every_player min_distance_to_players 7 max_distance_to_players 9 match_player_civ }
    create_object 285 { number_of_objects 5 min_distance_to_players 25 temp_min_distance_group_placement 20 })";
  const int counts[]={6,7,4,4,5,4},inner[]={10,12,18,25,14,20},outer[]={12,16,26,35,18,26};
  for(int row=0;row<std::min(resourceRows,6);++row) {
    const std::string object="create_object "+std::to_string(row>=4?102:row?66:59)+
      " { set_place_for_every_player set_gaia_object_only set_tight_grouping number_of_objects "+std::to_string(counts[row])+
      " group_placement_radius "+std::to_string(row>=4?2:3)+" min_distance_to_players "+std::to_string(inner[row])+
      " max_distance_to_players "+std::to_string(outer[row])+" min_distance_group_placement "+std::to_string(row?7:6)+
      " max_distance_to_other_zones 7 } ";
    source+=object;
  }
  for(int row=6;row<resourceRows;++row)
    source+=" create_object 594 { set_place_for_every_player set_gaia_object_only set_loose_grouping number_of_objects "+
      std::to_string(row==6?4:2)+" number_of_groups "+std::to_string(row==6?1:2)+
      " min_distance_to_players "+std::to_string(row==6?10:14)+" max_distance_to_players "+std::to_string(row==6?12:30)+" } ";
  return source;
}
