// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <array>
#include <cstdint>
#include <cstddef>
#include <unordered_set>
#include <vector>
namespace swgb {
// Original57deb0 reads four sticky fields, not a current-visibility query.
struct AiResourceKnowledge {
    std::array<bool,4> found{}; // food, carbon, ore, nova
    std::unordered_set<uint32_t> objects;
    std::vector<uint8_t> observed;
    void admit(int cls,bool hasCenter,float edgeDistance,int mineralLimit,int foodLimit) {
        int type=cls==27?0:cls==30?2:cls==29?3:-1;
        if(type>=0 && (!hasCenter || double(edgeDistance)<double(type==0?foodLimit:mineralLimit)))
            found[size_t(type)]=true;
    }
    void forestTile(int terrain,bool hasTree) {
        if(!hasTree)return;
        switch(terrain) {
        case 49:case 50:case 10:case 19:case 18:case 13:case 20:case 21:case 17:
            found[1]=true;break;
        }
    }
};
} // namespace swgb
