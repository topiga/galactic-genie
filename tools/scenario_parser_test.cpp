// SPDX-License-Identifier: GPL-3.0-or-later
// Usage: scenario_parser_test <original five-unit editor SC1> <older SCX> [more SC1 files]
// Original game files stay outside the repository.
#include "../src/core/scenario.h"
#include "../src/core/genie_dat.h"
#include <zlib.h>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
static void check(bool ok,const char *message){if(!ok)throw std::runtime_error(message);}
static std::vector<uint8_t> read(const char *path){std::ifstream f(path,std::ios::binary);check(bool(f),"Open scenario");return {(std::istreambuf_iterator<char>(f)),{}};}
static uint32_t word(const std::vector<uint8_t>&b,size_t p){check(p+4<=b.size(),"Read fixture word");uint32_t v;std::memcpy(&v,b.data()+p,4);return v;}
static void put(std::vector<uint8_t>&b,size_t p,uint32_t v){check(p+4<=b.size(),"Write fixture word");std::memcpy(b.data()+p,&v,4);}
static std::vector<uint8_t> pack(const std::vector<uint8_t>&file,const std::vector<uint8_t>&body){
 const size_t offset=8+word(file,4);std::vector<uint8_t> out(file.begin(),file.begin()+offset),compressed(compressBound(body.size()));z_stream z{};
 check(deflateInit2(&z,Z_BEST_SPEED,Z_DEFLATED,-MAX_WBITS,8,Z_DEFAULT_STRATEGY)==Z_OK,"Initialize fixture compression");z.next_in=const_cast<Bytef*>(body.data());z.avail_in=body.size();z.next_out=compressed.data();z.avail_out=compressed.size();const int result=deflate(&z,Z_FINISH);const size_t size=z.total_out;deflateEnd(&z);check(result==Z_STREAM_END,"Compress fixture");out.insert(out.end(),compressed.begin(),compressed.begin()+size);return out;
}
int main(int argc,char**argv){if(argc<3)return 2;try{
 const auto editor=read(argv[1]);swgb::Scenario s;std::string error;check(s.load(editor,&error),error.c_str());
 check(std::fabs(s.playerDataVersion-1.30f)<.0001f&&s.map.width==144&&s.map.height==144,"Original editor version and map dimensions");
 check(s.units.size()==5&&s.nextUnitId==5,"All five editor units retained");
 const float xs[]={70.5f,69.5f,68.5f,71.5f,70.5f},ys[]={71.5f,72.5f,73.5f,73.5f,74.5f};
 for(size_t i=0;i<5;++i){const auto&u=s.units[i];check(u.spawnId==i&&u.unitId==329&&u.player==(i<3?1:2)&&u.x==xs[i]&&u.y==ys[i]&&u.state==2,"Original owner, ID0, prototype and position retained");}
 for(size_t p=0;p<2;++p){check(s.players[p].civilization==1&&s.players[p].active,"Both Empire players loaded");for(float r:s.players[p].resources)check(r==0,"Original zero resources");}
 const size_t offset=8+word(editor,4);std::vector<uint8_t> body;check(swgb::dat::inflateRaw({editor.begin()+offset,editor.end()},body,&error),error.c_str());
 const uint8_t marker[]={0x9d,0xff,0xff,0xff};auto it=std::search(body.begin(),body.end(),std::begin(marker),std::end(marker));check(it!=body.end(),"Find editor resource separator");const size_t resourceOffset=size_t(it-body.begin())+4;
 check(word(body,resourceOffset+16*6*4)==0xffffff9d,"Exactly six resource fields precede victory separator");
 auto varied=body;for(size_t p=0;p<16;++p)for(size_t r=0;r<6;++r)put(varied,resourceOffset+4*(p*6+r),uint32_t(p*100+r+1));
 check(s.load(pack(editor,varied),&error),error.c_str());for(size_t p=0;p<16;++p)for(size_t r=0;r<6;++r)check(s.players[p].resources[r]==(p<8&&r<5?0.f:float(p*100+r+1)),"Six-field alignment and later playable-player resource overrides");
 auto corrupt=body;put(corrupt,resourceOffset+16*6*4,0);check(!s.load(pack(editor,corrupt),&error)&&s.units.empty()&&s.map.tiles.empty(),"Bad victory separator is rejected and partial state cleared");
 auto truncated=editor;truncated.resize(16);check(!s.load(truncated,&error),"Truncated file rejected");
 check(s.load(read(argv[2]),&error),error.c_str());check(std::fabs(s.playerDataVersion-1.22f)<.0001f&&!s.map.tiles.empty()&&!s.units.empty(),"Older packaged 1.22 SCX still loads");
 for(int i=3;i<argc;++i)check(s.load(read(argv[i]),&error),error.c_str());
 std::cout<<"PASS: original SC1 units/resources, malformed input, older SCX and "<<argc-3<<" additional scenarios\n";
 }catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}}
