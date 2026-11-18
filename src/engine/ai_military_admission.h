// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "../core/genie_random.h"
#include <algorithm>
#include <cstdint>
#include <limits>
#include <optional>
#include <vector>
namespace swgb {
struct AiMilitaryAdmissionGroup {
    int32_t id=-1;
    int type=100,state=1,memberCount=0;
    uint32_t flags=32;
};
struct AiMilitaryAdmissionSettings {
    int soldiers=0,exploreMinimum=1,exploreMaximumGroups=0;
    int attackMinimum=4,attackMaximum=10,attackMaximumGroups=0,attackRandomness=1;
};
struct AiMilitaryAdmissionOperation {
    bool create=false;
    int32_t id=-1;
    int type=0,desired=0;
};
struct AiMilitaryAdmissionPlan {
    std::vector<AiMilitaryAdmissionOperation> operations;
    int32_t nextId=0;
    uint32_t randomState=0;
};
// Ordinary100/101 reconciliation from5aaa20. Special112 pool admission is a
// separate caller stage; this helper neither invents that pool nor changes its quota.
// Deleting a group releases membership only: original5acee0 sends no Stop command.
inline std::optional<AiMilitaryAdmissionPlan> planAiMilitaryAdmission(
        std::vector<AiMilitaryAdmissionGroup> groups,const AiMilitaryAdmissionSettings &settings,
        int32_t nextId,uint32_t randomState) {
    if(settings.soldiers<0 || settings.exploreMinimum<=0 || settings.exploreMaximumGroups<0 ||
       settings.attackMinimum<=0 || settings.attackMaximum<0 || settings.attackMaximumGroups<0 ||
       settings.attackRandomness<0 || settings.attackRandomness==std::numeric_limits<int>::max() || nextId<0)
        return std::nullopt;
    AiMilitaryAdmissionPlan plan;plan.nextId=nextId;plan.randomState=randomState;
    const auto erase=[&](size_t index) {
        plan.operations.push_back({false,groups[index].id,groups[index].type,0});
        groups.erase(groups.begin()+index);
    };
    for(size_t i=0;i<groups.size();) {
        if(groups[i].memberCount<0)return std::nullopt;
        if(!groups[i].memberCount)erase(i);else ++i;
    }
    const auto count=[&](int type) {
        return int(std::count_if(groups.begin(),groups.end(),[&](const auto &g){return g.type==type;}));
    };
    const int wantedExplore=int(std::min<int64_t>(settings.exploreMaximumGroups,
        (int64_t(settings.soldiers)+settings.exploreMinimum-1)/settings.exploreMinimum));
    const int64_t remaining=int64_t(settings.soldiers)-int64_t(wantedExplore)*settings.exploreMinimum;
    const int wantedAttack=remaining>0?int(std::min<int64_t>(settings.attackMaximumGroups,
        (remaining+settings.attackMinimum-1)/settings.attackMinimum)):0;
    const auto create=[&](int type,int desired)->bool {
        if(plan.nextId==std::numeric_limits<int32_t>::max())return false;
        const int32_t id=plan.nextId++;
        plan.operations.push_back({true,id,type,desired});
        groups.push_back({id,type,1,0,32});
        return true;
    };
    for(int n=count(101);n<wantedExplore;++n)
        if(!create(101,settings.exploreMinimum))return std::nullopt;
    while(count(101)>wantedExplore) {
        auto it=std::find_if(groups.begin(),groups.end(),[](const auto &g){return g.type==101 && g.state==0;});
        if(it==groups.end())it=std::find_if(groups.begin(),groups.end(),[](const auto &g){return g.type==101 && g.state==1;});
        if(it==groups.end())break;
        erase(size_t(it-groups.begin()));
    }
    for(int n=count(100);n<wantedAttack;++n) {
        if(plan.nextId==std::numeric_limits<int32_t>::max())return std::nullopt;
        const int extra=int(genieRandomDraw(plan.randomState)%uint32_t(settings.attackRandomness+1));
        const int desired=int(std::min<int64_t>(settings.attackMaximum,int64_t(settings.attackMinimum)+extra));
        if(!create(100,desired))return std::nullopt;
    }
    while(count(100)>wantedAttack) {
        const auto it=std::find_if(groups.begin(),groups.end(),[](const auto &g){return g.type==100;});
        if(it==groups.end() || (it->flags&8u))break;
        erase(size_t(it-groups.begin()));
    }
    return plan;
}
} // namespace swgb
