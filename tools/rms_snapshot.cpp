// SPDX-License-Identifier: GPL-3.0-or-later
// RMS snapshot with explicit raw dimensions; size preset remains TINY_MAP.
#include "../src/engine/rms_map.h"
#include "../src/core/genie_dat.h"
#include "../src/core/drs.h"
#include <iostream>
#include <fstream>
#include <charconv>
#include <sstream>
#include <stdexcept>
int main(int argc,char **argv) {
    if(argc!=9 && argc!=10){std::cerr<<"usage: swgb-rms-snapshot DAT SEED TERRAIN CIV1 CIV2 DRS RESOURCE tiny|tiny-random\n"
        "   or: swgb-rms-snapshot DAT SEED TERRAIN CIVS DRS RESOURCE WIDTH HEIGHT tiny|tiny-random\n";return 2;}
    try {
    const auto number=[](const std::string &text,uint32_t maximum) {
        uint32_t value=0;const auto result=std::from_chars(text.data(),text.data()+text.size(),value);
        if(result.ec!=std::errc{} || result.ptr!=text.data()+text.size() || value>maximum)
            throw std::runtime_error("invalid unsigned snapshot argument: "+text);
        return value;
    };
    const bool legacy=argc==9;
    uint32_t width=legacy?120:number(argv[7],1024),height=legacy?120:number(argv[8],1024);
    uint32_t rng=number(argv[2],UINT32_MAX);
    std::vector<uint16_t> civs;
    std::string csv=legacy?std::string(argv[4])+","+argv[5]:argv[4];
    if(csv.empty() || csv.back()==',')throw std::runtime_error("empty civilization");
    std::istringstream fields(csv);std::string field;
    while(std::getline(fields,field,',')) {
        const auto civ=number(field,8);
        if(!civ || civs.size()==8)throw std::runtime_error("requires 1..8 civilizations in 1..8");
        civs.push_back(uint16_t(civ));
    }
    const std::string mode=argv[legacy?8:9];
    if(!width || !height || (mode!="tiny" && mode!="tiny-random"))throw std::runtime_error("invalid dimensions or placement mode");
    const auto resource=number(argv[legacy?7:6],INT32_MAX);
    swgb::dat::DatFile data;std::string error;
    if(!data.load(argv[1],&error)){std::cerr<<error;return 1;}
    swgb::ResourceSet resources;
    if(!resources.add(argv[legacy?6:5],&error)){std::cerr<<error;return 1;}
    swgb::RmsScript script;swgb::RmsMap map;
    if(!script.load(int32_t(resource),resources,&error) ||
        !swgb::generateRmsMap(script,mode=="tiny"?std::unordered_set<std::string>{"FIXED_POSITIONS","TINY_MAP","DEFAULT_RESOURCES"}:std::unordered_set<std::string>{"TINY_MAP","DEFAULT_RESOURCES"},width,height,data,rng,map,&error,uint32_t(civs.size()),civs)){std::cerr<<error;return 1;}
    if(!map.objects.empty()){std::cerr<<"unexpected pre-owned object rows";return 1;}
    std::cout.precision(17);std::cout<<"rng "<<rng<<'\n';uint32_t next=0;
    for(const auto &o:map.cliffs){std::cout<<o.creationOrdinal<<" 0 "<<o.unit<<' '<<o.facet<<' '<<o.x<<' '<<o.y<<'\n';next=o.creationOrdinal+1;}
    for(const auto &o:map.scenery) {
        if(o.alive)std::cout<<next<<" 0 "<<o.unitId<<' '<<int(o.facet)<<' '<<o.x<<' '<<o.y<<'\n';
        ++next;
    }
    for(const auto &o:map.ownedObjects)std::cout<<next++<<' '<<o.owner<<' '<<o.unitId<<' '<<int(o.facet)<<' '<<o.x<<' '<<o.y<<' '<<int(o.pendingFoundation)<<' '<<int(o.animationFrame)<<' '<<o.initialIdleDelay<<' '<<data.graphic(data.civs[o.civilization].units[o.unitId].standingGraphic[0])->frameDuration<<' '<<int(data.graphic(data.civs[o.civilization].units[o.unitId].standingGraphic[0])->sequenceType)<<'\n';
    for(const auto &o:map.resourcesAfterOwned) {
        const auto *g=data.graphic(data.civs[o.owner?civs.at(o.owner-1):0].units[o.unitId].standingGraphic[0]);
        std::cout<<next++<<' '<<o.owner<<' '<<o.unitId<<' '<<int(o.facet)<<' '<<o.x<<' '<<o.y<<' ';if(o.owner)std::cout<<int(o.pendingFoundation)<<' ';std::cout<<int(o.animationFrame)<<' '<<o.initialIdleDelay<<' '<<g->frameDuration<<' '<<int(g->sequenceType)<<'\n';
    }
    // RmsMap uses empty height/slope arrays for flat maps; snapshots contain every tile.
    if(map.elevation.empty())map.elevation.assign(map.terrain.size(),0);
    if(map.slopes.empty())map.slopes.assign(map.terrain.size(),0);
    std::ofstream heightFile(std::string(argv[3])+".heights",std::ios::binary);heightFile.write(reinterpret_cast<const char*>(map.elevation.data()),map.elevation.size());
    std::ofstream slope(std::string(argv[3])+".slopes",std::ios::binary);slope.write(reinterpret_cast<const char*>(map.slopes.data()),map.slopes.size());
    std::ofstream raster(argv[3],std::ios::binary);raster.write(reinterpret_cast<const char*>(map.terrain.data()),map.terrain.size());
    heightFile.flush();slope.flush();raster.flush();
    if(!heightFile || !slope || !raster || !std::cout){std::cerr<<"snapshot write failed";return 1;}
    } catch(const std::exception &error){std::cerr<<error.what();return 2;}
}
