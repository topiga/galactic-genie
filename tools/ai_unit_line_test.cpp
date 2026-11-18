// SPDX-License-Identifier: GPL-3.0-or-later
#include "../src/engine/ai_unit_line.h"
#include <array>
#include <cstdio>
#include <cstdlib>
static void check(bool value,const char *message) {if(!value) {std::fprintf(stderr,"FAIL %s\n",message);std::exit(1);}}
int main() {
    const std::vector<int16_t> members{10,11,12};
    // Original full588a10/588a90 and577380/57b7d0 controls, all8 enabled combinations.
    const std::array<int,8> first{-1,10,11,10,12,10,11,10},last{-1,10,11,11,12,12,12,12};
    for(int mask=0;mask<8;++mask) {
        const auto enabled=[&](int id){return (mask&(1<<(id-10)))!=0;};
        check(swgb::aiUnitLineSelection(members,false,enabled)==first[size_t(mask)],"first queued member");
        check(swgb::aiUnitLineSelection(members,true,enabled)==last[size_t(mask)],"last affordability member");
    }
    // Six original57e520/57e690 controls sum every member, including earlier upgrades.
    for(const std::array<int,3> values: {std::array<int,3>{0,0,0},{0,0,0},{1,2,3},{4,5,6},{0,5,0},{0,7,0}})
        check(swgb::aiUnitLineCount(members,[&](int id){return values[size_t(id-10)];})==values[0]+values[1]+values[2],"all members counted");
    check(!swgb::aiUnitLineOperand(-9) && swgb::aiUnitLineOperand(-10) && swgb::aiUnitLineOperand(-42) && !swgb::aiUnitLineOperand(-43),"original operand boundary");
    check(swgb::aiUnitLineContains(members,11) && !swgb::aiUnitLineContains(members,13),"queued line membership");
    std::puts("PASS original unit-line selection and counting (14 oracle controls)");
}
