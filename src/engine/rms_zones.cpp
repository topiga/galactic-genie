// SPDX-License-Identifier: GPL-3.0-or-later
#include "rms_zones.h"
#include <algorithm>
#include <cmath>

namespace swgb {
bool generateRmsZones(uint32_t size,const std::vector<uint8_t> &terrain,
    const std::vector<float> &passability,RmsZones &out,std::string *err) {
    out={};
    if (err) err->clear();
    auto fail=[&](const char *reason) { if(err) *err=reason; return false; };
    if (!size || size>1024 || terrain.size()!=size_t(size)*size)
        return fail("RMS zones require a square map of size1..1024");
    for (float value:passability) if (!std::isfinite(value))
        return fail("RMS zones require finite passability values");
    for (uint8_t tile:terrain) if (tile>=passability.size())
        return fail("RMS zone terrain is outside the passability table");
    RmsZones result;
    result.tiles.assign(terrain.size(),255);
    std::vector<uint32_t> pending;
    uint32_t components=0;
    for (uint32_t seed=0;seed<terrain.size();++seed) {
        if (result.tiles[seed]!=255) continue;
        const uint8_t zone=uint8_t(components++%255);
        const bool positive=passability[terrain[seed]]>0;
        result.tiles[seed]=zone;
        pending.push_back(seed);
        while (!pending.empty()) {
            const uint32_t tile=pending.back();pending.pop_back();
            const uint32_t x=tile%size,y=tile/size;
            auto visit=[&](uint32_t next) {
                if (result.tiles[next]==255 && (passability[terrain[next]]>0)==positive) {
                    result.tiles[next]=zone;pending.push_back(next);
                }
            };
            if (x) visit(tile-1);
            if (x+1<size) visit(tile+1);
            if (y) visit(tile-size);
            if (y+1<size) visit(tile+size);
        }
    }
    result.count=std::min(components,255u);
    out=std::move(result);
    return true;
}

bool generateRmsZoneAdjacency(uint32_t size,const RmsZones &zones,
    RmsZoneAdjacency &out,std::string *err) {
    out={};
    if (err) err->clear();
    auto fail=[&](const char *reason) { if(err) *err=reason; return false; };
    if (!size || size>1024 || zones.tiles.size()!=size_t(size)*size ||
        !zones.count || zones.count>255)
        return fail("RMS zone adjacency requires a square raster and1..255 zones");
    for (uint8_t label:zones.tiles) if (label>=zones.count)
        return fail("RMS zone label is outside the component count");
    const uint32_t count=zones.count;
    out.contact.assign(count*count,0);
    out.commonNeighbor.assign(count*count,255);
    for (uint32_t y=0;y<size;++y) for (uint32_t x=0;x<size;++x) {
        const uint32_t row=zones.tiles[y*size+x]*count;
        for (uint32_t yy=y?y-1:0;yy<std::min(y+2,size);++yy)
            for (uint32_t xx=x?x-1:0;xx<std::min(x+2,size);++xx)
                out.contact[row+zones.tiles[yy*size+xx]]=1;
    }
    for (uint32_t a=0;a<count;++a) for (uint32_t b=0;b<count;++b) {
        if (out.contact[a*count+b]) continue;
        for (uint32_t k=0;k<count;++k) {
            if (out.contact[a*count+k] && out.contact[b*count+k]) {
                out.commonNeighbor[a*count+b]=uint8_t(k);
                break;
            }
        }
    }
    return true;
}

bool RmsZoneCache::lookup(uint32_t size,const std::vector<uint8_t> &terrain,
    const std::vector<float> &passability,uint32_t &index,std::string *err) {
    index=UINT32_MAX;
    if (err) err->clear();
    auto fail=[&](const char *reason) { if(err) *err=reason; return false; };
    if (!size || size>1024 || terrain.size()!=size_t(size)*size || (size_ && size_!=size))
        return fail("RMS zone cache requires a consistent square map; clear before resizing");
    std::vector<bool> positive;
    for (float value:passability) {
        if (!std::isfinite(value)) return fail("RMS zones require finite passability values");
        positive.push_back(value>0);
    }
    for (uint8_t tile:terrain) if (tile>=passability.size())
        return fail("RMS zone terrain is outside the passability table");
    for (uint32_t i=0;i<entries_.size();++i) if (entries_[i].positive==positive) {
        index=i;
        return true;
    }
    Entry result;
    result.positive=std::move(positive);
    if (!generateRmsZones(size,terrain,passability,result.zones,err) ||
        !generateRmsZoneAdjacency(size,result.zones,result.adjacency,err)) return false;
    index=uint32_t(entries_.size());
    entries_.push_back(std::move(result));
    size_=size;
    return true;
}
const RmsZoneCache::Entry *RmsZoneCache::entry(uint32_t index) const {
    return index<entries_.size()?&entries_[index]:nullptr;
}
void RmsZoneCache::clear() {
    entries_.clear();
    size_=0;
}
}
