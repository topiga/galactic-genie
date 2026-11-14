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
 // Controlled original replay changes only forage quantity, retaining all later rows.
 std::vector<swgb::RmsToken> tokens;rng=16160;
 check(script.preprocess(defines,tokens,&error,&rng) && rng==1993207714u,"Forest preprocessing RNG");
 std::string source;bool forage=false;unsigned changes=0;
 for(size_t i=0;i<tokens.size();++i) {
  if(tokens[i].text=="create_object" && i+1<tokens.size())forage=tokens[i+1].text=="59";
  if(forage && tokens[i].text=="number_of_objects" && i+1<tokens.size()) {
   check(tokens[i+1].text=="12","original Forest forage quantity");tokens[i+1].text="3";++changes;
  }
  source+=tokens[i].text+" ";
 }
 check(changes==1,"one controlled forage row");
 swgb::RmsScript smallForage;
 check(smallForage.load(0,[&](int32_t resource,std::vector<uint8_t>&bytes){if(resource)return false;bytes.assign(source.begin(),source.end());return true;},{},&error),error.c_str());
 check(swgb::generateRmsMap(smallForage,defines,120,120,assets.dat(),rng,map,&error,2,{1,1}),error.c_str());
 check(rng==278519141u && map.scenery.size()==7554 && map.ownedObjects.size()==10 && map.resourcesAfterOwned.size()==220,"controlled forage3 full-stage count/RNG");
 check(bytesHash(map.terrain)==3348257725u && bytesHash(map.elevation)==908132044u,"controlled forage3 original grids");
 hash=2166136261u;for(const auto&o:map.resourcesAfterOwned)pose(o.unitId,0,o.x,o.y,o.facet);
 check(hash==1662422u,"controlled forage3 all220 original Gaia poses/order");
 std::puts("PASS: controlled forage3 and all subsequent Forest rows match original");
 std::puts("PASS: full packaged Forest23 rows,7802 objects, original grids/RNG and native import");
}
