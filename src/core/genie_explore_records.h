// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <array>
#include <cstdint>
#include <optional>

namespace swgb {
// Original58f910: ordered information records, independent of live target objects.
// Record supplies id,owner,position[3] bytes in original X/Y/Z order.
template<class Records,class Knowledge,class Zone,class Distance,class Path,class Mark>
std::optional<std::array<int,2>> genieExploreRememberedDestination(
        int actorId,int owner,int width,int height,const Records &records,
        Knowledge knowledge,Zone sameZone,Distance distance,Path path,Mark mark) {
    if(actorId==0)return std::nullopt;
    const typename Records::value_type *best=nullptr;int bestDistance=0;
    for(const auto &record:records) {
        if(record.id==-1 || record.owner==0 || record.owner==owner)continue;
        const int x=record.position[0],y=record.position[1];
        if(x<width && y<height && knowledge(x,y)!=255)continue;
        if(!sameZone(x,y))continue;
        const int value=int(distance(x,y,record.position[2]));
        if(!best || value<bestDistance) {best=&record;bestDistance=value;}
    }
    if(!best)return std::nullopt;
    const int x=best->position[0],y=best->position[1];
    const bool accepted=path(x,y);mark(x,y,accepted?0:1);
    if(!accepted)return std::nullopt;
    return std::array<int,2>{x,y};
}
} // namespace swgb
