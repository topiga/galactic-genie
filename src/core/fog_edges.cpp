// SPDX-License-Identifier: GPL-3.0-or-later
#include "fog_edges.h"
#include "bytes.h"
#include <algorithm>
#include <fstream>
#include <iterator>

namespace swgb {
uint8_t FogEdges::pattern(uint8_t neighbors) {
    // Original 0x609720: cardinal edges suppress their two redundant corners.
    static const auto table=[] {
        std::array<uint8_t,256> out{};
        unsigned count=0;
        auto canonical=[](unsigned mask) {
            if(mask&0x80) mask&=~0x09u;
            if(mask&0x40) mask&=~0x0cu;
            if(mask&0x20) mask&=~0x06u;
            if(mask&0x10) mask&=~0x03u;
            return mask;
        };
        for(unsigned i=0;i<256;++i) if(canonical(i)==i) out[i]=uint8_t(count++);
        for(unsigned i=0;i<256;++i) out[i]=out[canonical(i)];
        return out;
    }();
    return table[neighbors];
}

bool FogEdges::load(const std::string &tilePath,const std::string &blackPath,std::string *error) {
    auto read=[](const std::string &path) {
        std::ifstream file(path,std::ios::binary|std::ios::ate);
        if(!file || file.tellg()<0 || file.tellg()>2*1024*1024) throw FormatError("invalid fog edge file: "+path);
        std::vector<uint8_t> bytes(size_t(file.tellg()));file.seekg(0);
        if(!bytes.empty() && !file.read(reinterpret_cast<char *>(bytes.data()),bytes.size())) throw FormatError("cannot read fog edges: "+path);
        return bytes;
    };
    try {return parse(read(tilePath),read(blackPath),error);}
    catch(const std::exception &e) {if(error) *error=e.what();return false;}
}

bool FogEdges::parse(const std::vector<uint8_t> &tile,const std::vector<uint8_t> &black,std::string *error) {
    shapes_={};heights_={};
    try {
        auto decode=[&](const std::vector<uint8_t> &bytes,bool blackFile) {
            ByteReader reader(bytes);
            std::array<uint32_t,17> slopes{};reader.array(slopes.data(),slopes.size());
            if(slopes[0]!=68) throw FormatError("invalid fog slope table");
            const int entries=blackFile?47:94;
            for(int slope=0;slope<17;++slope) {
                const size_t end=slope<16?slopes[slope+1]:bytes.size();
                if(slopes[slope]>end || end>bytes.size() || end-slopes[slope]<size_t(entries)*4) throw FormatError("invalid fog block bounds");
                reader.seek(slopes[slope]);auto offsets=reader.vec<uint32_t>(entries);
                if(offsets[0]!=slopes[slope]+entries*4) throw FormatError("invalid fog span table");
                for(int entry=0;entry<entries;++entry) {
                    const size_t limit=entry+1<entries?offsets[entry+1]:end;
                    if(offsets[entry]>limit || limit>end || limit-offsets[entry]<3 || (limit-offsets[entry])%3) throw FormatError("invalid fog span bounds");
                    reader.seek(offsets[entry]);
                    auto &shape=shapes_[slope][blackFile?entry:entry/2];
                    auto &spans=blackFile?shape.black:(entry%2?shape.fog:shape.clear);
                    while(reader.pos()<limit) {
                        FogSpan span{reader.u8(),reader.u8(),reader.u8()};
                        if(span.y==255) {
                            if(span.left!=255 || span.right!=255 || reader.pos()!=limit) throw FormatError("invalid fog terminator");
                            break;
                        }
                        if(span.left>span.right || span.right>96 || span.y>72 || reader.pos()==limit) throw FormatError("invalid fog span");
                        heights_[slope]=std::max(heights_[slope],int(span.y)+1);spans.push_back(span);
                    }
                }
            }
        };
        decode(tile,false);decode(black,true);return true;
    } catch(const std::exception &e) {shapes_={};heights_={};if(error) *error=e.what();return false;}
}
} // namespace swgb
