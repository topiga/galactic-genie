// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace swgb {
// Original PER train/count operands recognize line indices10..42, not arbitrary negatives.
inline bool aiUnitLineOperand(int operand) {return operand<=-10 && operand>=-42;}
//588a10 returns first enabled member;588a90 visits all and retains the last.
template<class Enabled>
int aiUnitLineSelection(const std::vector<int16_t> &members,bool last,Enabled enabled) {
    int result=-1;
    for(int id:members)if(enabled(id)) {result=id;if(!last)break;}
    return result;
}
template<class Count>
int aiUnitLineCount(const std::vector<int16_t> &members,Count count) {
    int result=0;for(int id:members)result+=count(id);return result;
}
inline bool aiUnitLineContains(const std::vector<int16_t> &members,int id) {
    return std::find(members.begin(),members.end(),id)!=members.end();
}
} // namespace swgb
