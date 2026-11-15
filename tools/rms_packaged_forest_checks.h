// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Full original Forest54204, seed16160, Tiny/FIXED/DEFAULT, two Empire owners.
// Original executable replay includes all23 object rows; grids also match Windows.
static void rmsPackagedForestChecks(swgb::Assets &assets,const std::string &dataDir) {
 swgb::ResourceSet resources;swgb::RmsScript script;std::string error;
 check(resources.add(dataDir+"/gamedata_x1.drs",&error) && script.load(54204,resources,&error),error.c_str());
 const std::unordered_set<std::string> defines{"TINY_MAP","FIXED_POSITIONS","DEFAULT_RESOURCES"};
 uint32_t rng=16160;swgb::RmsMap map;
 check(swgb::generateRmsMap(script,defines,120,120,assets.dat(),rng,map,&error,2,{1,1}),error.c_str());
 check(rng==3740942338u && map.scenery.size()==7554 && map.ownedObjects.size()==10 && map.resourcesAfterOwned.size()==238,"full original Forest count/RNG");
 auto bytesHash=[](const auto &bytes){uint32_t hash=2166136261u;for(auto v:bytes)hash=(hash^v)*16777619u;return hash;};
 check(bytesHash(map.terrain)==3348257725u && bytesHash(map.elevation)==908132044u,"original complete Forest terrain/elevation grids");

 uint32_t hash=2166136261u;
 auto pose=[&](int unit,int owner,float x,float y,uint8_t facet){for(auto v:{uint32_t(unit),uint32_t(owner),uint32_t(x*2),uint32_t(y*2),uint32_t(facet)})hash=(hash^v)*16777619u;};
 for(const auto &o:map.scenery){check(o.alive,"original Forest retains automatic trees");pose(o.unitId,0,o.x,o.y,o.facet);}
 check(hash==2138192714u,"original automatic tree creation order/pose");
 hash=2166136261u;for(const auto &o:map.ownedObjects)pose(o.unitId,o.owner,o.x,o.y,o.facet);
 check(hash==3085727744u,"original owned starting creation order/pose");
 hash=2166136261u;for(const auto &o:map.resourcesAfterOwned)pose(o.unitId,0,o.x,o.y,o.facet);
 check(hash==640727536u,"all238 original Gaia object poses/order");

 swgb::Game game(assets);rng=16160;
 check(game.initRms(script,defines,120,rng,&error,2,{1,1}),error.c_str());
 check(rng==3740942338u && game.activeObjectCount()==7802,"full packaged Forest native import");
 uint32_t id=1;
 auto imported=[&](int unit,int owner,float x,float y){const auto p=game.objectPosition(id);check(game.objectUnitId(id)==unit && game.objectPlayer(id)==owner && p[0]==x && p[1]==y,"native Forest IDs/types/owners/positions");++id;};
 for(const auto&o:map.scenery)imported(o.unitId,0,o.x,o.y);
 for(const auto&o:map.ownedObjects)imported(o.unitId,o.owner,o.x,o.y);
 for(const auto&o:map.resourcesAfterOwned)imported(o.unitId,0,o.x,o.y);
 const struct{size_t index;uint8_t frame;float delay;} animations[]={{77,4,0.f},{78,0,0.f},{79,2,0.f},{80,1,0.f},{81,0,0.f},{82,7,0.f},{83,6,0.f},{84,2,0.f},{85,0,0.f},{86,2,0.f},{87,3,0.f},{88,7,0.f},{89,2,0.f},{90,5,0.f},{91,2,0.f},{92,2,0.f},{93,7,0.32649499177932739f},{94,2,0.f},{95,7,0.18707606196403503f},{96,1,0.f},{117,0,0.f},{118,0,0.f},{119,0,0.f},{120,0,0.f},{121,0,0.f},{122,0,0.f},{123,0,0.f},{124,0,0.f},{125,0,0.f},{126,7,0.36436846852302551f},{127,7,0.f},{128,5,0.f},{129,9,0.f},{130,0,0.f},{131,0,0.f},{132,11,0.1168193444609642f},{133,11,0.f},{234,5,0.f},{235,0,0.f},{236,4,0.f},{237,6,0.f}};
 for(const auto&e:animations){const auto&o=map.resourcesAfterOwned[e.index];const auto*g=assets.dat().graphic(assets.dat().civs[0].units[o.unitId].standingGraphic[0]);const auto a=game.objectAnimationForTesting(uint32_t(7565+e.index));check(o.animationFrame==e.frame && std::abs(o.initialIdleDelay-e.delay)<.000001f && a[0]==e.frame*g->frameDuration && std::abs(a[1]-e.delay)<.000001f,"original complete Forest animal/fish animation import");}
 // Original fish gate rejects diagonal-only shore contact; cardinal contact is required.
 std::vector<uint8_t> ground(14400,6),eligible(14400,0);ground[60*120+60]=1;ground[59*120+59]=2;eligible[60*120+60]=1;
 std::vector<swgb::RmsTimberPlacement> placed;rng=1;
 check(swgb::generateRmsGroupedResourcePlacements(120,120,ground,assets.dat(),69,{0,4,1,0,3},rng,placed,&error,{},16000000,{},swgb::RmsResourceDistance::Near,&eligible) && placed.empty(),"fish require cardinal shore contact");
 ground[60*120+59]=2;rng=1;
 check(swgb::generateRmsGroupedResourcePlacements(120,120,ground,assets.dat(),69,{0,4,1,0,3},rng,placed,&error,{},16000000,{},swgb::RmsResourceDistance::Near,&eligible) && placed.size()==1 && placed[0].x==60.5f && placed[0].y==60.5f,"fish accept cardinal shore contact");
 ground[0]=22;rng=1;const auto before=eligible;
 check(!swgb::generateRmsGroupedResourcePlacements(120,120,ground,assets.dat(),1585,{0,7,1,0,3},rng,placed,&error,{},16000000,{},swgb::RmsResourceDistance::Near,&eligible) && rng==1 && placed.empty() && eligible==before,"unsupported wave quantity rejected atomically");
 // Controlled original replays vary forage/Nerf quantities, retaining all later rows.
 const struct{int nearHerd,farHerd,predator;uint32_t count,poses,rng,frames;int globalPredatorSize=1,treeUnit=413,forestTerrain=21;uint32_t sceneryPoses=2138192714u,terrainHash=3348257725u;bool chainedHunt=false,omitNerfs=false;unsigned explicitTrees=110;bool omitForage=false;unsigned longChain=0,lateHunt=0;bool terrain9=false,terrain22=false;} variations[]={
  {4,2,126,220,1662422u,278519141u,513271022u},
  {2,1,126,210,3750728331u,4161792159u,758251980u},
  {8,4,126,235,989150139u,2125502712u,2483236625u},
  {4,2,812,220,1670957952u,278519141u,513271022u},
  {4,2,1197,220,3239039312u,278519141u,513271022u},
  {4,2,1197,224,1665827321u,44224755u,513271022u,2},
  {4,2,126,220,3933298748u,278519141u,513271022u,1,348},
  {4,2,126,220,2518584576u,278519141u,513271022u,1,349},
  {4,2,126,220,3933298748u,278519141u,513271022u,1,348,18,780123404u,1911528789u},
  {4,2,126,224,636226797u,44224755u,513271022u,2,348,18,780123404u,1911528789u},
  {4,2,126,220,2518584576u,278519141u,513271022u,1,349,20,4094354918u,3646470573u},
  {4,2,126,226,921507206u,2914264210u,513271022u,1,348,18,780123404u,1911528789u,true},
  {0,0,126,211,1050928889u,2709380116u,2166136261u,1,349,20,4094354918u,3646470573u,true,true},
  {4,2,126,213,3985350200u,3794078108u,3773911184u,1,348,18,780123404u,1911528789u,false,false,110,true},
  {4,2,126,238,2497652140u,1393824384u,513271022u,1,348,18,780123404u,1911528789u,true,false,110,false,1},
  {0,0,126,219,4220003808u,2530534559u,2166136261u,1,349,20,4094354918u,3646470573u,true,true,110,false,2},
  {4,2,126,242,4120192947u,1364577121u,1289455766u,1,348,18,780123404u,1911528789u,true,false,110,false,1,1},
  {0,0,126,220,3799455950u,734846818u,2166136261u,1,349,20,4094354918u,3646470573u,true,true,110,false,2,2},
  {0,0,126,219,3181366234u,3194867101u,2166136261u,1,349,20,4094354918u,3646470573u,true,true,110,false,2,3},
  {4,2,126,220,3933298748u,278519141u,513271022u,1,348,18,780123404u,3296036497u,false,false,110,false,0,0,true},
  // Terrain22 oracle executes original objects on native pre-object input; it does not prove terrain generation.
  {4,2,126,228,358297336u,3038787869u,513271022u,1,348,18,780123404u,2976665734u,false,false,110,false,0,0,true,true}};
 for(const auto &v:variations) {
  std::vector<swgb::RmsToken> tokens;rng=16160;
  check(script.preprocess(defines,tokens,&error,&rng) && rng==1993207714u,"Forest preprocessing RNG");
  if(v.forestTerrain!=21)for(size_t i=0;i+1<tokens.size();++i) {
   const auto &name=tokens[i].text;
   if(name!="create_terrain" && name!="base_terrain" && name!="replace_terrain" && name!="terrain_cost" && name!="terrain_size")continue;
   for(size_t j=i+1;j<=i+(name=="replace_terrain"?2:1);++j) {
    if(v.terrain22 && tokens[j].text=="1")tokens[j].text="22";
    else if(v.terrain22 && tokens[j].text=="22")tokens[j].text="1";
    else if(v.terrain9 && tokens[j].text=="5")tokens[j].text="9";
    else if(v.terrain9 && tokens[j].text=="9")tokens[j].text="5";
    else if(tokens[j].text=="21")tokens[j].text=std::to_string(v.forestTerrain);
    else if(tokens[j].text==std::to_string(v.forestTerrain))tokens[j].text="21";
   }
  }
  std::string source,unit;unsigned changes=0,herdRow=0,predatorRow=0,predatorChanges=0,treeRows=0;
  for(size_t i=0;i<tokens.size();++i) {
   if(tokens[i].text=="create_object" && i+1<tokens.size()) {
    unit=tokens[i+1].text;
    if(unit=="1365" && v.lateHunt) {
     while(i<tokens.size() && tokens[i].text!="}")++i;
     if(v.lateHunt==1)source+="create_object 594 { number_of_objects 2 set_loose_grouping set_gaia_object_only set_place_for_every_player min_distance_to_players 10 max_distance_to_players 12 } ";
     else if(v.lateHunt==2)source+="create_object 822 { number_of_objects 1 set_gaia_object_only set_place_for_every_player min_distance_to_players 16 max_distance_to_players 22 } ";
     continue;
    }
    if(unit=="413" && treeRows==2 && v.lateHunt)source+="create_object "+std::to_string(v.lateHunt==1?1249:822)+" { number_of_objects 3 group_variance 1 set_loose_grouping set_gaia_object_only set_place_for_every_player min_distance_to_players 35 min_distance_group_placement 5 } ";
    if((unit=="594" && v.omitNerfs) || (unit=="59" && v.omitForage)){while(i<tokens.size() && tokens[i].text!="}")++i;continue;}
    if(unit=="126" && predatorRow==0 && v.chainedHunt)source+="create_object 1365 { number_of_objects 4 group_variance 1 set_loose_grouping set_gaia_object_only set_place_for_every_player min_distance_to_players 14 max_distance_to_players 30 } ";
    if(unit=="126" && predatorRow==0 && v.longChain) {
     source+=v.longChain==1?
      "create_object 822 { number_of_objects 4 set_loose_grouping set_gaia_object_only set_place_for_every_player min_distance_to_players 16 max_distance_to_players 22 } create_object 1249 { number_of_objects 2 set_gaia_object_only set_place_for_every_player min_distance_to_players 16 max_distance_to_players 22 } ":
      "create_object 1249 { number_of_objects 2 set_loose_grouping set_gaia_object_only set_place_for_every_player min_distance_to_players 16 max_distance_to_players 22 } create_object 822 { number_of_objects 1 set_gaia_object_only set_place_for_every_player min_distance_to_players 16 max_distance_to_players 22 } ";
    }
    if(unit=="413"){tokens[i+1].text=std::to_string(v.treeUnit);++treeRows;}
    if(unit=="126"){tokens[i+1].text=std::to_string(v.predator);++predatorRow;}
   }
   if((unit=="59" || unit=="594") && tokens[i].text=="number_of_objects" && i+1<tokens.size()) {
    const int expected=unit=="59"?12:herdRow==0?4:2;
    check(tokens[i+1].text==std::to_string(expected),"original Forest forage/herd quantity");
    tokens[i+1].text=std::to_string(unit=="59"?3:herdRow++==0?v.nearHerd:v.farHerd);++changes;
   }
   if(unit=="126" && predatorRow==2 && tokens[i].text=="number_of_objects" && i+1<tokens.size()) {
    check(tokens[i+1].text=="1","original global predator quantity");
    tokens[i+1].text=std::to_string(v.globalPredatorSize);++predatorChanges;
   }
   source+=tokens[i].text+" ";
  }
  check(changes==((v.omitNerfs?0u:2u)+(v.omitForage?0u:1u)) && herdRow==(v.omitNerfs?0u:2u) && predatorChanges==1 && treeRows==3,"controlled forage and two herd rows");
  swgb::RmsScript varied;
  check(varied.load(0,[&](int32_t resource,std::vector<uint8_t>&bytes){if(resource)return false;bytes.assign(source.begin(),source.end());return true;},{},&error),error.c_str());
  check(swgb::generateRmsMap(varied,defines,120,120,assets.dat(),rng,map,&error,2,{1,1}),error.c_str());
  check(rng==v.rng && map.scenery.size()==7554 && map.ownedObjects.size()==10 && map.resourcesAfterOwned.size()==v.count,"controlled resource full-stage count/RNG");
  check(bytesHash(map.terrain)==v.terrainHash && bytesHash(map.elevation)==908132044u,"controlled resource original grids");
  hash=2166136261u;for(const auto&o:map.scenery)pose(o.unitId,0,o.x,o.y,o.facet);
  check(hash==v.sceneryPoses,"controlled original automatic tree poses/order");
  hash=2166136261u;uint32_t frames=2166136261u,herds=0;
  for(const auto&o:map.resourcesAfterOwned) {
   pose(o.unitId,0,o.x,o.y,o.facet);
   if(o.unitId==594){frames=(frames^o.animationFrame)*16777619u;++herds;check(o.initialIdleDelay==0,"original Nerf extra idle delay");}
  }
  check(hash==v.poses,"controlled resource all original Gaia poses/order");
  check(frames==v.frames && herds==uint32_t(v.nearHerd*2+v.farHerd*4+(v.lateHunt==1?4:0)),"controlled original Nerf animation frames/count");
  if(v.treeUnit!=413) {
   rng=1993207714u;
   check(game.initRms(varied,defines,120,rng,&error,2,{1,1}) && rng==v.rng,"explicit tree species native import");
   if(v.forestTerrain!=21)for(size_t i=0;i<map.scenery.size();++i) {
    const auto &o=map.scenery[i];const auto pos=game.objectPosition(uint32_t(i+1));
    check(o.unitId==v.treeUnit && game.objectUnitId(uint32_t(i+1))==v.treeUnit && pos[0]==o.x && pos[1]==o.y && game.objectResourceAmount(uint32_t(i+1))==100,"native automatic forest tree positions/carbon");
   }
   if(v.terrain22) {
    const uint8_t frames[]={1,21,46,47,34,22,1,1};size_t waves=0;
    for(size_t i=0;i<map.resourcesAfterOwned.size();++i) {
     const auto &o=map.resourcesAfterOwned[i];if(o.unitId!=1585)continue;
     check(waves<8 && o.animationFrame==frames[waves] && o.initialIdleDelay==0,"original wave startup frames");
     const auto a=game.objectAnimationForTesting(uint32_t(map.scenery.size()+map.ownedObjects.size()+i+1));
     check(a[0]==frames[waves]*.13f && a[1]==0,"native original wave animation import");++waves;
    }
    check(waves==8,"original full Forest wave count");
   }
   unsigned trees=0;
   for(size_t i=0;i<map.resourcesAfterOwned.size();++i) {
    const auto &o=map.resourcesAfterOwned[i];if(o.unitId!=v.treeUnit)continue;++trees;
    const uint32_t id=uint32_t(map.scenery.size()+map.ownedObjects.size()+i+1);const auto pos=game.objectPosition(id);
    check(game.objectUnitId(id)==v.treeUnit && pos[0]==o.x && pos[1]==o.y && game.objectResourceAmount(id)==100,"native explicit tree positions and carbon amount");
   }
   check(trees==v.explicitTrees,"original near/far/global explicit tree counts");
  }
  if(v.predator!=126) {
   rng=1993207714u;
   check(game.initRms(varied,defines,120,rng,&error,2,{1,1}) && rng==v.rng,"controlled predator native import");
   struct PredatorAnimation{size_t index;uint8_t frame;float delay;};
   const std::vector<PredatorAnimation> singleAnimations={{75,7,0.0239997208f},{76,7,0.686550438f},{77,7,0.903152466f},{78,7,0.289547384f},{107,2,0.f},{108,7,0.0298775807f},{109,7,0.0541031733f},{110,7,0.0439405218f}};
   const std::vector<PredatorAnimation> pairAnimations={{75,7,0.0239997208f},{76,7,0.686550438f},{77,7,0.903152466f},{78,7,0.289547384f},{107,2,0.f},{108,7,0.522244871f},{109,7,0.551139772f},{110,7,0.f},{111,7,0.282955378f},{112,6,0.f},{113,7,0.569377661f},{114,7,0.471651345f}};
   const auto &expected=v.globalPredatorSize==2?pairAnimations:singleAnimations;
   for(const auto &e:expected) {
    const auto&o=map.resourcesAfterOwned[e.index];
    check(o.unitId==v.predator && o.animationFrame==e.frame && std::abs(o.initialIdleDelay-e.delay)<.000001f,"original predator startup animation");
    const uint32_t id=uint32_t(map.scenery.size()+map.ownedObjects.size()+e.index+1);
    const auto animation=game.objectAnimationForTesting(id);
    check(game.objectUnitId(id)==v.predator && std::abs(animation[0]-e.frame*.1f)<.000001f && std::abs(animation[1]-e.delay)<.000001f,"native predator identity and animation");
   }
  }

 }
 // Live Windows seed2 raster captured after restart; mobile startup poses remain unverified.
 rng=2;
 check(swgb::generateRmsMap(script,defines,120,120,assets.dat(),rng,map,&error,2,{1,1}),error.c_str());
 check(bytesHash(map.terrain)==4035986315u && bytesHash(map.elevation)==3707759835u && bytesHash(map.slopes)==3939991058u,"live Windows seed2 complete terrain/elevation/slopes");
 hash=2166136261u;unsigned staticCount=0;
 for(const auto &o:map.resourcesAfterOwned) {
  if(o.unitId!=285 && o.unitId!=59 && o.unitId!=66 && o.unitId!=102 && o.unitId!=348 && o.unitId!=349 && o.unitId!=413 && o.unitId!=1585)continue;
  for(auto v:{uint32_t(o.unitId),0u,uint32_t(o.x*2),uint32_t(o.y*2)})hash=(hash^v)*16777619u;
  ++staticCount;
 }
 check(staticCount==202 && hash==582877992u,"live Windows seed2 static resource positions/order");
 std::puts("PASS: controlled forage, herds, predators and tree variants through all Forest rows");
 std::puts("PASS: full packaged Forest23 rows,7802 objects, original grids/RNG and native import");
}
