// SPDX-License-Identifier: GPL-3.0-or-later
#include "engine/rms_scenery_appearance.h"
#include <cassert>
#include <cstring>
#include <iostream>
int main() {
    struct Fixture { uint32_t first,last,headingBits; uint8_t facet; };
    const Fixture fixtures[]={
        {0u,2730u,0u,11},
        {2731u,5461u,1057360530u,0},
        {5462u,8191u,1065749138u,0},
        {8192u,10922u,1070141403u,1},
        {10923u,13652u,1074137746u,2},
        {13653u,16383u,1076333878u,3},
        {16384u,19114u,1078530011u,4},
        {19115u,21844u,1080726143u,5},
        {21845u,24575u,1082526354u,6},
        {24576u,27305u,1083624420u,7},
        {27306u,30036u,1084722486u,8},
        {30037u,32766u,1085820552u,9},
        {32767u,32767u,1086918619u,10},
    };
    // Invert the original LCG three times so the third draw visits the entire domain.
    constexpr uint32_t inverse=3115528533u;
    for (const auto &fixture:fixtures) for(uint32_t draw=fixture.first;draw<=fixture.last;++draw) {
        const uint32_t expectedState=draw<<16;
        uint32_t rng=expectedState;
        for(int i=0;i<3;++i) rng=(rng-2531011u)*inverse;
        const auto appearance=swgb::drawRmsCarbonAppearance(rng);
        uint32_t bits;std::memcpy(&bits,&appearance.headingRadians,sizeof(bits));
        assert(rng==expectedState && bits==fixture.headingBits && appearance.facet==fixture.facet);
    }
    const Fixture tenAngles[]={
        {0u,3276u,0u,9},
        {3277u,6553u,1059117436u,0},
        {6554u,9830u,1067506044u,1},
        {9831u,13106u,1072776762u,2},
        {13107u,16383u,1075894652u,3},
        {16384u,19660u,1078530011u,4},
        {19661u,22936u,1081165370u,5},
        {22937u,26213u,1082965580u,6},
        {26214u,29490u,1084283260u,7},
        {29491u,32766u,1085600939u,8},
        {32767u,32767u,1086918619u,9},
    };
    for (const auto &fixture:tenAngles) for(uint32_t draw=fixture.first;draw<=fixture.last;++draw) {
        const uint32_t expectedState=draw<<16;
        uint32_t rng=expectedState;
        for(int i=0;i<3;++i) rng=(rng-2531011u)*inverse;
        const auto appearance=swgb::drawRmsCarbonAppearance(rng,10);
        uint32_t bits;std::memcpy(&bits,&appearance.headingRadians,sizeof(bits));
        assert(rng==expectedState && bits==fixture.headingBits && appearance.facet==fixture.facet);
    }
    { uint32_t rng=1u;const auto a=swgb::drawRmsCarbonAppearance(rng);assert(rng==415139642u && a.facet==0); }
    { uint32_t rng=12345u;const auto a=swgb::drawRmsCarbonAppearance(rng);assert(rng==3837989202u && a.facet==7); }
    { uint32_t rng=4294967295u;const auto a=swgb::drawRmsCarbonAppearance(rng);assert(rng==2368614256u && a.facet==0); }
    { uint32_t rng=948628385u;const auto a=swgb::drawRmsCarbonAppearance(rng);assert(rng==932654426u && a.facet==3); }
    std::cout << "Original carbon appearance full-domain and constructor fixtures passed\n";
}
