// SPDX-License-Identifier: GPL-3.0-or-later
#include "../src/engine/ai_attack_memory.h"
#include <cstdio>
#include <stdexcept>
static void check(bool ok,const char *message){if(!ok)throw std::runtime_error(message);}
int main() {
    using namespace swgb;
    // Independent original58f260 controls,2026-11-18. Y-varying original
    // control repeats first case; Y has no parameter in the recovered helper.
    struct Case {int w,h,x1,x2,owner,a,b,index,field;};
    const Case cases[]={
        {120,120,10,30,1,1,2,0,2},{120,120,10,30,1,2,1,0,1},
        {120,120,10,30,1,2,3,-1,0},{120,120,10,30,1,1,1,0,1},
        {120,120,10,30,1,2,1,0,1},{120,120,100,110,1,2,1,15,1},
        {120,80,40,50,1,2,1,6,1},{120,120,266,286,1,2,1,0,1},
        {0,120,10,30,1,2,1,-1,0}};
    for(const auto &c:cases) {
        std::array<AiAttackMemoryQuarter,16> quarters{};
        const auto result=recordAiAttackMemoryQuarter(quarters,c.w,c.h,c.x1,c.x2,c.owner,c.a,c.b);
        check(result==(c.index<0?AiAttackMemoryResult::Ignored:AiAttackMemoryResult::Recorded),
              "Original attack-memory counter admission");
        for(size_t i=0;i<quarters.size();++i) {
            check(quarters[i].explored==0 &&
                  quarters[i].received==uint32_t(int(i)==c.index && c.field==1) &&
                  quarters[i].issued==uint32_t(int(i)==c.index && c.field==2),
                  "Original counter index and receiver precedence");
        }
    }
    //28 actual event513 controls stopped after group gate. These do not
    // certify subsequent diplomacy effects or final memory-event emission.
    const int groups[]={-1,100,101,102,103,108,109};
    const bool accepted[7][4]={
        {true,true,true,true},{true,true,true,false},{false,false,false,false},
        {true,true,true,false},{false,false,false,false},{true,true,true,false},
        {false,false,false,false}};
    for(size_t g=0;g<7;++g)for(int sn=0;sn<2;++sn)for(int state=0;state<2;++state)
        check(aiAttackMemoryGroupAllows(groups[g],state?2:0,sn)==accepted[g][sn*2+state],
              "Original event513 military-group admission gate");
    // Actual full513 dispatch controls vary attacker AI/human and both
    // diplomacy predicates. All eight reach the same counter event; those
    // predicates affect other strategy state, not this admission result.
    {
        std::vector<uint32_t> processed;
        AiAttackMemoryNotification event{1,true,true,1,2,70,-1,0,0};
        check(admitAiAttackMemoryNotification(processed,event) && processed==std::vector<uint32_t>{1},
              "Original full513 ordinary hostile notification reaches memory");
        check(!admitAiAttackMemoryNotification(processed,event) && processed.size()==1,
              "Original full513 repeated victim is suppressed before memory");
    }
    const AiAttackMemoryNotification rejected[]={
        {1,true,true,1,0,70,-1,0,0}, // Gaia consumes victim dedup first.
        {1,true,true,1,1,70,-1,0,0},
        {1,true,true,1,2,60,-1,0,0},
        {1,true,false,1,2,70,-1,0,0},
        {1,false,true,1,2,70,-1,0,0}};
    for(size_t i=0;i<5;++i) {
        std::vector<uint32_t> processed;
        check(!admitAiAttackMemoryNotification(processed,rejected[i]) && processed.size()==(i==0?1u:0u),
              "Original full513 rejected objects preserve dedup ordering");
    }
    for(int group:{101,103,109}) {
        std::vector<uint32_t> processed;
        check(!admitAiAttackMemoryNotification(processed,{1,true,true,1,2,70,group,0,0}) &&
              processed==std::vector<uint32_t>{1},"Original group rejection still consumes victim dedup");
    }
    std::array<AiAttackMemoryQuarter,16> quarters{};
    check(recordAiAttackMemoryQuarter(quarters,120,120,120,120,1,2,1)==
          AiAttackMemoryResult::UnsupportedQuarter,"Out-of-table original corner is explicitly unsupported");
    quarters[0].received=UINT32_MAX;
    recordAiAttackMemoryQuarter(quarters,120,120,10,30,1,2,1);
    check(quarters[0].received==0,"Counter increment retains uint32 wrap");
    std::puts("PASS: original attack-memory quarter counters and bounded513 group gates");
}
