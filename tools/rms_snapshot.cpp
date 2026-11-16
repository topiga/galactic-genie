// SPDX-License-Identifier: GPL-3.0-or-later
// Tiny120 fixed two-player RMS snapshot for original-engine comparisons.
#include "../src/engine/rms_map.h"
#include "../src/core/genie_dat.h"
#include "../src/core/drs.h"
#include <iostream>
#include <fstream>
int main(int argc,char **argv) {
    if(argc!=9 || std::string(argv[8])!="tiny"){std::cerr<<"usage: swgb-rms-snapshot DAT SEED TERRAIN CIV1 CIV2 DRS RESOURCE tiny\n";return 2;}
    try {
    swgb::dat::DatFile data;std::string error;
    if(!data.load(argv[1],&error)){std::cerr<<error;return 1;}
    swgb::ResourceSet resources;
    if(!resources.add(argv[6],&error)){std::cerr<<error;return 1;}
    swgb::RmsScript script;swgb::RmsMap map;uint32_t rng=uint32_t(std::stoul(argv[2]));
    if(!script.load(std::stoi(argv[7]),resources,&error) ||
        !swgb::generateRmsMap(script,argc==9?std::unordered_set<std::string>{"FIXED_POSITIONS","TINY_MAP","DEFAULT_RESOURCES"}:std::unordered_set<std::string>{"FIXED_POSITIONS"},120,120,data,rng,map,&error,2,{uint16_t(std::stoi(argv[4])),uint16_t(std::stoi(argv[5]))})){std::cerr<<error;return 1;}
    if(!map.objects.empty()){std::cerr<<"unexpected pre-owned object rows";return 1;}
    std::cout.precision(17);std::cout<<"rng "<<rng<<'\n';uint32_t next=0;
    for(const auto &o:map.cliffs){std::cout<<o.creationOrdinal<<" 0 "<<o.unit<<' '<<o.facet<<' '<<o.x<<' '<<o.y<<'\n';next=o.creationOrdinal+1;}
    for(const auto &o:map.scenery) {
        if(o.alive)std::cout<<next<<" 0 "<<o.unitId<<' '<<int(o.facet)<<' '<<o.x<<' '<<o.y<<'\n';
        ++next;
    }
    for(const auto &o:map.ownedObjects)std::cout<<next++<<' '<<o.owner<<' '<<o.unitId<<' '<<int(o.facet)<<' '<<o.x<<' '<<o.y<<' '<<int(o.pendingFoundation)<<' '<<int(o.animationFrame)<<' '<<o.initialIdleDelay<<' '<<data.graphic(data.civs[o.civilization].units[o.unitId].standingGraphic[0])->frameDuration<<' '<<int(data.graphic(data.civs[o.civilization].units[o.unitId].standingGraphic[0])->sequenceType)<<'\n';
    for(const auto &o:map.resourcesAfterOwned) {
        const auto *g=data.graphic(data.civs[o.owner?uint16_t(std::stoi(argv[3+o.owner])):0].units[o.unitId].standingGraphic[0]);
        std::cout<<next++<<' '<<o.owner<<' '<<o.unitId<<' '<<int(o.facet)<<' '<<o.x<<' '<<o.y<<' ';if(o.owner)std::cout<<int(o.pendingFoundation)<<' ';std::cout<<int(o.animationFrame)<<' '<<o.initialIdleDelay<<' '<<g->frameDuration<<' '<<int(g->sequenceType)<<'\n';
    }
    // RmsMap uses empty height/slope arrays for flat maps; snapshots contain every tile.
    if(map.elevation.empty())map.elevation.assign(map.terrain.size(),0);
    if(map.slopes.empty())map.slopes.assign(map.terrain.size(),0);
    std::ofstream height(std::string(argv[3])+".heights",std::ios::binary);height.write(reinterpret_cast<const char*>(map.elevation.data()),map.elevation.size());
    std::ofstream slope(std::string(argv[3])+".slopes",std::ios::binary);slope.write(reinterpret_cast<const char*>(map.slopes.data()),map.slopes.size());
    std::ofstream raster(argv[3],std::ios::binary);raster.write(reinterpret_cast<const char*>(map.terrain.data()),map.terrain.size());
    height.flush();slope.flush();raster.flush();
    if(!height || !slope || !raster || !std::cout){std::cerr<<"snapshot write failed";return 1;}
    } catch(const std::exception &error){std::cerr<<error.what();return 2;}
}
