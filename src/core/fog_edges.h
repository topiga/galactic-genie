// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace swgb {
struct FogSpan { uint8_t y,left,right; };
struct FogShape {
    std::vector<FogSpan> clear,fog,black;
};
class FogEdges {
public:
    bool load(const std::string &tilePath,const std::string &blackPath,std::string *error);
    bool parse(const std::vector<uint8_t> &tile,const std::vector<uint8_t> &black,std::string *error);
    static uint8_t pattern(uint8_t neighbors);
    const FogShape &shape(int slope,int pattern) const {return shapes_.at(slope).at(pattern);}
    int height(int slope) const {return heights_.at(slope);}
private:
    std::array<std::array<FogShape,47>,17> shapes_{};
    std::array<int,17> heights_{};
};
} // namespace swgb
